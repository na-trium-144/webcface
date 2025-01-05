#include "webcface/common/internal/message/pack.h"
#include "webcface/client.h"
#include "webcface/internal/client_internal.h"
#include "webcface/internal/client_ws.h"
#include "webcface/common/internal/unlock.h"
#include <string>
#include <chrono>
#include <thread>
#include <spdlog/sinks/stdout_color_sinks.h>
#include "./c_wcf/c_wcf_internal.h"

WEBCFACE_NS_BEGIN

extern "C" wcfClient *wcfInit(const char *name, const char *host, int port) {
    auto wcli =
        new std::shared_ptr<internal::ClientData>(new internal::ClientData(
            strOrEmpty(name), SharedString::encode(host ? host : "127.0.0.1"),
            port));
    wcli_list.push_back(wcli);
    return wcli;
}
extern "C" wcfClient *wcfInitW(const wchar_t *name, const wchar_t *host,
                               int port) {
    auto wcli =
        new std::shared_ptr<internal::ClientData>(new internal::ClientData(
            strOrEmpty(name), SharedString::encode(host ? host : L"127.0.0.1"),
            port));
    wcli_list.push_back(wcli);
    return wcli;
}
extern "C" wcfClient *wcfInitDefault(const char *name) {
    return wcfInit(name, "127.0.0.1", WEBCFACE_DEFAULT_PORT);
}
extern "C" wcfClient *wcfInitDefaultW(const wchar_t *name) {
    return wcfInitW(name, L"127.0.0.1", WEBCFACE_DEFAULT_PORT);
}
extern "C" int wcfIsValid(wcfClient *wcli) {
    WCF_GET_WCLI(0);
    return 1;
}
extern "C" wcfStatus wcfDestroy(void *ptr) {
    {
        auto f_ptr = static_cast<const wcfMultiVal *>(ptr);
        auto f_it = func_val_list.find(f_ptr);
        if (f_it != func_val_list.end()) {
            func_val_list.erase(f_it);
            delete f_ptr;
            return WCF_OK;
        }
    }
    {
        auto fw_ptr = static_cast<const wcfMultiValW *>(ptr);
        auto fw_it = func_val_list_w.find(fw_ptr);
        if (fw_it != func_val_list_w.end()) {
            func_val_list_w.erase(fw_it);
            delete fw_ptr;
            return WCF_OK;
        }
    }
    {
        auto v_ptr = static_cast<const wcfViewComponent *>(ptr);
        auto v_it = view_list.find(v_ptr);
        if (v_it != view_list.end()) {
            view_list.erase(v_it);
            delete[] v_ptr;
            return WCF_OK;
        }
    }
    {
        auto vw_ptr = static_cast<const wcfViewComponentW *>(ptr);
        auto vw_it = view_list_w.find(vw_ptr);
        if (vw_it != view_list_w.end()) {
            view_list_w.erase(vw_it);
            delete[] vw_ptr;
            return WCF_OK;
        }
    }
    {
        auto res = static_cast<Promise *>(ptr);
        auto res_it =
            std::find(func_result_list.begin(), func_result_list.end(), res);
        if (res_it != func_result_list.end()) {
            func_result_list.erase(res_it);
            delete res;
            return WCF_OK;
        }
    }
    return WCF_BAD_HANDLE;
}

internal::ClientData::ClientData(const SharedString &name,
                                 const SharedString &host, int port)
    : std::enable_shared_from_this<ClientData>(), self_member_name(name),
      host(host), port(port), current_curl_path(), current_ws_buf(),
      value_store(name), text_store(name), func_store(name), view_store(name),
      image_store(name), robot_model_store(name), canvas3d_store(name),
      canvas2d_store(name), log_store(name), sync_time_store(name) {
    static auto stderr_sink =
        std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
    logger_internal = std::make_shared<spdlog::logger>(
        "webcface_internal(" + name.decode() + ")", stderr_sink);
    if (std::getenv("WEBCFACE_TRACE") != nullptr) {
        logger_internal->set_level(spdlog::level::trace);
    } else if (std::getenv("WEBCFACE_VERBOSE") != nullptr) {
        logger_internal->set_level(spdlog::level::debug);
    } else {
        logger_internal->set_level(spdlog::level::off);
    }
}

void internal::ClientData::join() {
    if (ws_thread.joinable()) {
        ws_thread.join();
    }
    // if (sync_thread.joinable()) {
    //     sync_thread.join();
    // }
}

void internal::ClientData::start() {
    ScopedWsLock lock_ws(this);
    lock_ws.getData().do_ws_init = true;
    this->ws_cond.notify_all();
    if (!ws_thread.joinable()) {
        ws_thread = std::thread(internal::wsThreadMain, shared_from_this());
    }
    // if (!sync_thread.joinable() && this->auto_sync.load()) {
    //     sync_thread = std::thread(internal::syncThreadMain,
    //     shared_from_this());
    // }
}

wcfStatus wcfClose(wcfClient *wcli) {
    WCF_GET_WCLI(WCF_BAD_WCLI);
    wcli_->close();
    wcli_->join();
    wcli_list.erase(std::find(wcli_list.begin(), wcli_list.end(), wcli));
    delete &wcli_;
    return WCF_OK;
}
void internal::ClientData::close() {
    ScopedWsLock lock_ws(this);
    this->closing.store(true);
    this->ws_cond.notify_all();
}
extern "C" int wcfIsConnected(wcfClient *wcli) {
    WCF_GET_WCLI(0);
    internal::ClientData::ScopedWsLock lock_ws(wcli_);
    return lock_ws.getData().connected;
}
void internal::wsThreadMain(const std::shared_ptr<ClientData> &data) {
    if (data->port <= 0) {
        return;
    }
    std::optional<std::chrono::steady_clock::time_point> last_connected,
        last_recv;
    while (true) {
        ClientData::ScopedWsLock lock_ws(data);
        if (!lock_ws.getData().connected) {
            if (last_connected) {
                // すでに接続したことがあるなら最低10ms間隔を空ける
                data->ws_cond.wait_until(
                    lock_ws, *last_connected + std::chrono::milliseconds(10),
                    [&] { return data->closing.load(); });
            }
            data->ws_cond.wait(lock_ws, [&] {
                return data->closing.load() || lock_ws.getData().do_ws_init ||
                       data->auto_reconnect.load();
            });
            if (data->closing.load()) {
                return;
            }
            { // do_ws_initがtrueまたはauto_reconnectがtrueなので再接続を行う
                ScopedUnlock un(lock_ws);
                internal::WebSocket::init(data);
            }
            lock_ws.getData().do_ws_init = false;
            lock_ws.getData().connected = data->current_curl_connected;
            last_connected = std::chrono::steady_clock::now();
            last_recv = std::nullopt;
            data->ws_cond.notify_all();
            if (lock_ws.getData().connected) {
                ScopedUnlock un(lock_ws);
                ClientData::SyncDataFirst sync_first;
                {
                    // sync_firstがnulloptでない(sync()時にsyncDataFirstを実行済み)の場合はそれを使い、
                    // nulloptの場合はここでsyncDataFirstを呼ぶ
                    ClientData::ScopedSyncLock lock_s(data);
                    if (!lock_s.getData().sync_first) {
                        lock_s.getData().sync_first =
                            lock_s.getData().syncDataFirst(data.get());
                    }
                    sync_first = std::move(*lock_s.getData().sync_first);
                    // data->sync_firstはnulloptでない値が入ったままにしておく
                    // (std::moveで空にはなるがnulloptにはならない)
                }
                internal::WebSocket::send(data,
                                          data->packSyncDataFirst(sync_first));
            }
        } else {
            lock_ws.getData().do_ws_init = false;
            if (last_recv) {
                // syncデータがなければ、100us間隔を空ける
                lock_ws.getData().recv_ready = true;
                data->ws_cond.wait_until(
                    lock_ws, *last_recv + std::chrono::microseconds(100), [&] {
                        return data->closing.load() ||
                               lock_ws.getData().do_ws_recv ||
                               !lock_ws.getData().sync_queue.empty();
                    });
                lock_ws.getData().do_ws_recv = true;
            }

            {
                // syncの前にrecv
                ScopedUnlock un(lock_ws);
                while (true) { // データがなくなるまで受信
                    bool has_recv = internal::WebSocket::recv(
                        data, [data](std::string &&msg) {
                            ClientData::ScopedWsLock lock_ws(data);
                            lock_ws.getData().recv_queue.push(
                                message::unpack(msg, data->logger_internal));
                            data->ws_cond.notify_all();
                        });
                    if (!has_recv) {
                        break;
                    }
                }
            }
            lock_ws.getData().do_ws_recv = false;
            lock_ws.getData().recv_ready = false;
            lock_ws.getData().connected = data->current_curl_connected;
            if (!lock_ws.getData().connected) {
                data->self_member_id = std::nullopt;
                lock_ws.getData().sync_init_end = false;
                ScopedUnlock un(lock_ws);
                {
                    ClientData::ScopedSyncLock lock_s(data);
                    lock_s.getData().sync_first = std::nullopt;
                }
            }
            data->ws_cond.notify_all();
            last_recv = std::chrono::steady_clock::now();

            std::queue<std::variant<std::string, ClientData::SyncDataSnapshot>>
                sync_queue_data;
            lock_ws.getData().sync_queue.swap(sync_queue_data);
            if (lock_ws.getData().connected) {
                ScopedUnlock un(lock_ws);
                while (!sync_queue_data.empty()) {
                    // sync
                    auto msg = std::move(sync_queue_data.front());
                    std::string msg_s;
                    sync_queue_data.pop();
                    switch (msg.index()) {
                    case 0:
                        msg_s = std::move(std::get<0>(msg));
                        break;
                    case 1:
                    default: {
                        std::stringstream buf;
                        int len = 0;
                        msg_s = data->packSyncData(buf, len, std::get<1>(msg));
                        break;
                    }
                    }
                    internal::WebSocket::send(data, msg_s);
                }
            }

            if (data->closing.load()) {
                {
                    ScopedUnlock un(lock_ws);
                    internal::WebSocket::close(data);
                }
                lock_ws.getData().connected = data->current_curl_connected;
                data->ws_cond.notify_all();
                return;
            }
            data->ws_cond.notify_all();
        }
    }
}

extern "C" wcfStatus wcfSync(wcfClient *wcli) {
    WCF_GET_WCLI(WCF_BAD_WCLI);
    wcfStart(wcli);
    wcli_->syncImpl(true, true, std::chrono::microseconds(0));
    return WCF_OK;
}
extern "C" wcfStatus wcfLoopSyncFor(wcfClient *wcli, long long timeout) {
    WCF_GET_WCLI(WCF_BAD_WCLI);
    wcfStart(wcli);
    wcli_->syncImpl(true, true, std::chrono::microseconds(timeout));
    return WCF_OK;
}
extern "C" wcfStatus wcfLoopSync(wcfClient *wcli) {
    WCF_GET_WCLI(WCF_BAD_WCLI);
    wcfStart(wcli);
    wcli_->syncImpl(true, true, std::nullopt);
    return WCF_OK;
}

void internal::ClientData::syncImpl(
    bool sync, bool forever, std::optional<std::chrono::microseconds> timeout) {
    auto start_t = std::chrono::steady_clock::now();
    {
        ScopedWsLock lock_ws(this);
        if (lock_ws.getData().recv_ready) {
            lock_ws.getData().do_ws_recv = true;
            // 接続できてるなら、recvが完了するまで待つ
            // 1回のrecvはすぐ終わるのでtimeoutいらない
            // condition_variableは遅い
            // this->ws_cond.wait(lock, [&] {
            //     return this->closing.load() || !this->do_ws_recv;
            // });
            if (timeout && *timeout <= std::chrono::microseconds(0)) {
                ScopedUnlock un(lock_ws);
                while (true) {
                    ScopedWsLock lock_ws2(this);
                    if (this->closing.load() ||
                        !lock_ws2.getData().do_ws_recv) {
                        break;
                    }
                }
            }
        } else {
            lock_ws.getData().do_ws_recv = true;
            // recvさせるけど、待たない
        }
    }
    if (sync) {
        bool connected2;
        {
            ScopedWsLock lock_ws(this);
            connected2 = lock_ws.getData().connected;
        }
        {
            ScopedSyncLock lock_s(this);
            if (!connected2 && !lock_s.getData().sync_first) {
                lock_s.getData().sync_first =
                    lock_s.getData().syncDataFirst(this);
            } else {
                auto sync_now_data = lock_s.getData().syncData(this, false);
                lock_s.unlock();
                this->messagePushAlways(std::move(sync_now_data));
            }
        }
    }
    do {
        ScopedWsLock lock_ws(this);
        if (timeout) {
            if (*timeout > std::chrono::microseconds(0)) {
                // recv_queueにデータが入るまで待つ
                // 遅くともtimeout経過したら抜ける
                this->ws_cond.wait_until(lock_ws, start_t + *timeout, [&] {
                    return this->closing.load() ||
                           (!lock_ws.getData().recv_queue.empty() &&
                            !lock_ws.getData().do_ws_recv) ||
                           (!lock_ws.getData().connected &&
                            !this->auto_reconnect.load());
                });
            }
            if (lock_ws.getData().recv_queue.empty()) {
                // timeoutし、recv準備完了でない場合return
                return;
            }
        } else {
            // recv_queueにデータが入るまで無制限に待つ
            this->ws_cond.wait(lock_ws, [&] {
                return this->closing.load() ||
                       (!lock_ws.getData().recv_queue.empty() &&
                        !lock_ws.getData().do_ws_recv) ||
                       (!lock_ws.getData().connected &&
                        !this->auto_reconnect.load());
            });
        }
        if (this->closing.load() ||
            (!lock_ws.getData().connected && !this->auto_reconnect.load())) {
            // close時と接続されてないときreturn
            return;
        }

        while (!lock_ws.getData().recv_queue.empty()) {
            auto msg = std::move(lock_ws.getData().recv_queue.front());
            lock_ws.getData().recv_queue.pop();
            {
                ScopedUnlock un(lock_ws);
                this->onRecv(msg);
            }
        }
    } while (forever);
}
/*
void internal::syncThreadMain(const std::shared_ptr<ClientData> &data) {
    if (data->port <= 0) {
        return;
    }
    while (true) {
        {
            std::unique_lock lock(data->ws_m);
            data->ws_cond.wait(
                lock, [&] { return data->closing.load() || data->connected; });
            if (data->closing.load()) {
                return;
            }
        }
        data->syncImpl(true, std::nullopt);
    }
}
*/

extern "C" wcfStatus wcfStart(wcfClient *wcli) {
    WCF_GET_WCLI(WCF_BAD_WCLI);
    wcli_->start();
    return WCF_OK;
}
extern "C" wcfStatus wcfWaitConnection(wcfClient *wcli) {
    WCF_GET_WCLI(WCF_BAD_WCLI);
    wcli_->start();
    bool first_loop = true;
    while (!wcli_->closing.load()) {
        internal::ClientData::ScopedWsLock lock_ws(wcli_);
        if (!lock_ws.getData().connected) {
            // 初回またはautoReconnectが有効なら接続完了まで待機
            if (first_loop || wcli_->auto_reconnect.load()) {
                lock_ws.getData().do_ws_init = true;
                wcli_->ws_cond.notify_all();
                wcli_->ws_cond.wait(lock_ws, [wcli_, &lock_ws] {
                    return wcli_->closing.load() ||
                           lock_ws.getData().connected ||
                           !lock_ws.getData().do_ws_init;
                });
            } else {
                return WCF_OK;
            }
        } else {
            if (lock_ws.getData().sync_init_end) {
                return WCF_OK;
            } else {
                // autoRecvならsyncInit完了まで待機
                // そうでなければrecvを呼ぶ
                // if (wcli_->auto_sync.load()) {
                //     wcli_->ws_cond.wait(lock, [this] {
                //         return wcli_->closing.load() || !wcli_->connected ||
                //                wcli_->sync_init_end;
                //     });
                // } else {
                ScopedUnlock un(lock_ws);
                wcli_->syncImpl(false, false, std::nullopt);
                // }
            }
        }
        first_loop = false;
    }
    return WCF_OK;
}

extern "C" wcfStatus wcfAutoReconnect(wcfClient *wcli, int enabled) {
    WCF_GET_WCLI(WCF_BAD_WCLI);
    wcli_->auto_reconnect.store(enabled);
    return WCF_OK;
}
extern "C" int wcfAutoReconnectEnabled(wcfClient *wcli) {
    WCF_GET_WCLI(0);
    return wcli_->auto_reconnect.load();
}


WEBCFACE_NS_END
