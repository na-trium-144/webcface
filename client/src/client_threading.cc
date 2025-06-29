#include "webcface/common/internal/message/pack.h"
#include "webcface/client.h"
#include "webcface/internal/client_internal.h"
#include "webcface/internal/client_ws.h"
#include "webcface/common/internal/unlock.h"

#ifdef WEBCFACE_COMPILER_IS_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wabi"
#endif
#include <spdlog/sinks/stdout_color_sinks.h>
#ifdef WEBCFACE_COMPILER_IS_GCC
#pragma GCC diagnostic pop
#endif
#include <string>
#include <chrono>
#include <thread>

WEBCFACE_NS_BEGIN

Client::Client(const SharedString &name, const SharedString &host, int port)
    : Client(name, std::make_shared<internal::ClientData>(name, host, port)) {}

Client::Client(const SharedString &name,
               const std::shared_ptr<internal::ClientData> &data)
    : Member(data, name), data(data) {}

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
    internal::WebSocket::globalInit();
}
internal::ClientData::~ClientData() {
    join();
    internal::WebSocket::globalDeinit();
}

Client::~Client() {
    this->sanity.check();
    data->close();
    data->join();
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
    auto lock_ws = this->ws_data.lock();
    lock_ws->do_ws_init = true;
    lock_ws.cond().notify_all();
    if (!ws_thread.joinable()) {
        ws_thread = std::thread(internal::wsThreadMain, shared_from_this());
    }
    // if (!sync_thread.joinable() && this->auto_sync.load()) {
    //     sync_thread = std::thread(internal::syncThreadMain,
    //     shared_from_this());
    // }
}
const Client &Client::close() const {
    this->sanity.check();
    data->close();
    return *this;
}
void internal::ClientData::close() {
    auto lock_ws = this->ws_data.lock();
    this->closing.store(true);
    lock_ws.cond().notify_all();
}
bool Client::connected() const {
    this->sanity.check();
    return data->ws_data.lock()->connected;
}
void internal::wsThreadMain(const std::shared_ptr<ClientData> &data) {
    if (data->port <= 0) {
        return;
    }
    std::optional<std::chrono::steady_clock::time_point> last_connected,
        last_recv;
    while (true) {
        auto lock_ws = data->ws_data.lock();
        assert(lock_ws->connected == data->current_curl_connected);
        if (!lock_ws->connected) {
            if (last_connected) {
                // すでに接続したことがあるなら最低10ms間隔を空ける
                lock_ws.cond().wait_until(
                    lock_ws, *last_connected + std::chrono::milliseconds(10),
                    [&] { return data->closing.load(); });
            }
            // auto_reconnectなら続行、
            // そうでなければstart()が呼ばれるまで待機、
            // またはclosingならreturn
            lock_ws.cond().wait(lock_ws, [&] {
                return data->closing.load() || lock_ws->do_ws_init ||
                       data->auto_reconnect.load();
            });
            if (data->closing.load()) {
                return;
            }
            {
                // do_ws_initがtrueまたはauto_reconnectがtrueなので再接続を行う
                ScopedUnlock un(lock_ws);
                internal::WebSocket::init(data);
            }
            lock_ws->do_ws_init = false;
            lock_ws->connected = data->current_curl_connected;
            if (lock_ws->connected) {
                lock_ws->did_disconnect = false;
            }
            last_connected = std::chrono::steady_clock::now();
            last_recv = std::nullopt;
            // ここのnotify_allはdo_ws_initのリセットとconnectedの更新を通知
            lock_ws.cond().notify_all();

            if (lock_ws->connected) {
                // 接続が成功したらsyncDataFirstを送信
                // ws_condに通知するものはとくにない
                ScopedUnlock un(lock_ws);
                ClientData::SyncDataFirst sync_first;
                {
                    // sync_firstがnulloptでない(sync()時にsyncDataFirstを実行済み)の場合はそれを使い、
                    // nulloptの場合はここでsyncDataFirstを呼ぶ
                    auto lock_s = data->sync_data.lock();
                    if (!lock_s->sync_first) {
                        lock_s->sync_first = lock_s->syncDataFirst(data.get());
                    }
                    sync_first = std::move(*lock_s->sync_first);
                    // data->sync_firstはnulloptでない値が入ったままにしておく
                    // (std::moveで空にはなるがnulloptにはならない)
                    // すでに接続できていて普通に送ればいいだけのときに間違ってsync_firstに入れるのを防ぐ
                }
                auto msg_s = data->packSyncDataFirst(sync_first);
                data->logger_internal->trace("-> packed: {}",
                                             message::messageTrace(msg_s));
                internal::WebSocket::send(data, msg_s);
            }
            lock_ws->connected = data->current_curl_connected;
            lock_ws.cond().notify_all();
        } else {
            lock_ws->do_ws_init = false;
            if (last_recv) {
                // syncデータがキューにあれば続行、
                // そうでなければ100us間隔を空けるかrecvがよばれるまで待機、
                // またはclosingなら続行(recvとsendを1回済ませてからreturn)
                lock_ws->recv_ready = true;
                lock_ws.cond().wait_until(
                    lock_ws, *last_recv + std::chrono::microseconds(100), [&] {
                        return data->closing.load() || lock_ws->do_ws_recv ||
                               !lock_ws->sync_queue.empty();
                    });
                lock_ws->do_ws_recv = true;
            }

            {
                // sendの前にrecvを行う
                ScopedUnlock un(lock_ws);
#ifdef WEBCFACE_COMPILER_IS_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wabi"
#endif
                while (
                    // curl側に溜まっているデータがなくなるまで受信処理
                    internal::WebSocket::recv(data, [data](std::string &&msg) {
                        auto lock_ws = data->ws_data.lock();
                        data->logger_internal->trace(
                            "unpacking: {}", message::messageTrace(msg));
                        lock_ws->recv_queue.push(
                            message::unpack(msg, data->logger_internal));
                        lock_ws.cond().notify_all();
                    })) {
                }
#ifdef WEBCFACE_COMPILER_IS_GCC
#pragma GCC diagnostic pop
#endif
            }
            lock_ws->do_ws_recv = false;
            lock_ws->recv_ready = false;
            last_recv = std::chrono::steady_clock::now();
            // ここのnotify_allはrecvの終了を通知
            lock_ws.cond().notify_all();

            // connectedがfalseになった場合、sync_init_endやsync_firstなどの変数もリセットしないといけないが、
            // send() が終わった後のconnectedのチェック後にまとめてやるので
            // ここでは現在のconnected状態をnotify_all時に反映しないでおく
            lock_ws->connected = data->current_curl_connected;

            std::queue<std::variant<std::string, ClientData::SyncDataSnapshot>>
                sync_queue_data;
            lock_ws->sync_queue.swap(sync_queue_data);
            while (!sync_queue_data.empty() && lock_ws->connected) {
                // queueのデータを送信
                {
                    ScopedUnlock un(lock_ws);
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
                    data->logger_internal->trace("-> packed: {}",
                                                 message::messageTrace(msg_s));
                    internal::WebSocket::send(data, msg_s);
                }
                lock_ws->connected = data->current_curl_connected;
            }

            if (!lock_ws->connected) {
                // recv中またはsend中に切断を検知した場合
                data->self_member_id = std::nullopt;
                lock_ws->sync_init_end = false;
                lock_ws->did_disconnect = true;
                ScopedUnlock un(lock_ws);
                {
                    auto lock_s = data->sync_data.lock();
                    lock_s->sync_first = std::nullopt;
                }
            }

            if (data->closing.load()) {
                {
                    ScopedUnlock un(lock_ws);
                    internal::WebSocket::close(data);
                }
                lock_ws->connected = data->current_curl_connected;
                lock_ws.cond().notify_all();
                return;
            }

            // connectedの状態を通知
            lock_ws.cond().notify_all();
        }
    }
}
const Client &
Client::syncImpl(std::optional<std::chrono::microseconds> timeout) const {
    start();
    data->syncImpl(true, true, timeout);
    return *this;
}
void internal::ClientData::syncImpl(
    bool sync, bool forever, std::optional<std::chrono::microseconds> timeout) {
    auto start_t = std::chrono::steady_clock::now();
    {
        auto lock_ws = this->ws_data.lock();
        if (lock_ws->recv_ready) {
            lock_ws->do_ws_recv = true;
            // 接続できてるなら、recvが完了するまで待つ
            // 1回のrecvはすぐ終わるのでtimeoutいらない
            // condition_variableは遅い
            // lock_ws.cond().wait(lock, [&] {
            //     return this->closing.load() || !this->do_ws_recv;
            // });
            if (timeout && *timeout <= std::chrono::microseconds(0)) {
                ScopedUnlock un(lock_ws);
                while (true) {
                    if (this->closing.load() ||
                        !this->ws_data.lock()->do_ws_recv) {
                        break;
                    }
                }
            }
        } else {
            lock_ws->do_ws_recv = true;
            // recvさせるけど、待たない
        }
    }
    if (sync) {
        bool connected2;
        {
            auto lock_ws = this->ws_data.lock();
            connected2 = lock_ws->connected;
        }
        {
            auto lock_s = this->sync_data.lock();
            if (!connected2 && !lock_s->sync_first) {
                lock_s->sync_first = lock_s->syncDataFirst(this);
            } else {
                auto sync_now_data = lock_s->syncData(this, false);
                lock_s.unlock();
                this->messagePushAlways(std::move(sync_now_data));
            }
        }
    }
    do {
        auto lock_ws = this->ws_data.lock();
        if (timeout) {
            if (*timeout > std::chrono::microseconds(0)) {
                // recv_queueにデータが入るまで待つ
                // 遅くともtimeout経過したら抜ける
                lock_ws.cond().wait_until(lock_ws, start_t + *timeout, [&] {
                    return this->closing.load() ||
                           (!lock_ws->recv_queue.empty() &&
                            !lock_ws->do_ws_recv) ||
                           lock_ws->did_disconnect ||
                           (!lock_ws->connected &&
                            !this->auto_reconnect.load());
                });
            }
            if (lock_ws->recv_queue.empty() && !lock_ws->did_disconnect) {
                // timeoutし、recv準備完了でない場合return
                break;
            }
        } else {
            // recv_queueにデータが入るまで無制限に待つ
            lock_ws.cond().wait(lock_ws, [&] {
                return this->closing.load() ||
                       (!lock_ws->recv_queue.empty() && !lock_ws->do_ws_recv) ||
                       lock_ws->did_disconnect ||
                       (!lock_ws->connected && !this->auto_reconnect.load());
            });
        }
        if (lock_ws->did_disconnect) {
            {
                ScopedUnlock un(lock_ws);
                auto cl = this->member_disconnected_event.lock()
                              .get()[self_member_name];
                if (cl && *cl) {
                    cl->operator()(Field{shared_from_this(), self_member_name});
                }
                StrMap1<bool> entry = this->member_entry.lock().get();
                for (const auto &it : entry) {
                    const auto &name = it.first;
                    this->member_entry.lock().get()[name] = false;
                    cl = this->member_disconnected_event.lock().get()[name];
                    if (cl && *cl) {
                        cl->operator()(Field{shared_from_this(), name});
                    }
                }
            }
            lock_ws->did_disconnect = false;
        }
        if (this->closing.load() ||
            (!lock_ws->connected && !this->auto_reconnect.load())) {
            // close時と接続されてないときreturn
            break;
        }

        while (!lock_ws->recv_queue.empty()) {
            auto msg = std::move(lock_ws->recv_queue.front());
            lock_ws->recv_queue.pop();
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
            lock_ws.cond().wait(
                lock, [&] { return data->closing.load() || data->connected; });
            if (data->closing.load()) {
                return;
            }
        }
        data->syncImpl(true, std::nullopt);
    }
}
*/

const Client &Client::start() const {
    this->sanity.check();
    data->start();
    return *this;
}
const Client &Client::waitConnection() const {
    this->sanity.check();
    data->start();
    bool first_loop = true;
    while (!data->closing.load()) {
        auto lock_ws = data->ws_data.lock();
        if (!lock_ws->connected) {
            // 初回またはautoReconnectが有効なら接続完了まで待機
            if (first_loop || data->auto_reconnect.load()) {
                lock_ws->do_ws_init = true;
                lock_ws.cond().notify_all();
                lock_ws.cond().wait(lock_ws, [this, &lock_ws] {
                    return data->closing.load() || lock_ws->connected ||
                           !lock_ws->do_ws_init;
                });
            } else {
                return *this;
            }
        } else {
            if (lock_ws->sync_init_end) {
                return *this;
            } else {
                // autoRecvならsyncInit完了まで待機
                // そうでなければrecvを呼ぶ
                // if (data->auto_sync.load()) {
                //     lock_ws.cond().wait(lock, [this] {
                //         return data->closing.load() || !data->connected ||
                //                data->sync_init_end;
                //     });
                // } else {
                ScopedUnlock un(lock_ws);
                data->syncImpl(false, false, std::nullopt);
                // }
            }
        }
        first_loop = false;
    }
    return *this;
}
// void Client::autoSync(bool enabled) {
//     if (enabled /* && interval.count() > 0 */) {
//         data->auto_sync.store(true);
//     } else {
//         data->auto_sync.store(false);
//     }
// }

const Client &Client::autoReconnect(bool enabled) const {
    this->sanity.check();
    data->auto_reconnect.store(enabled);
    return *this;
}
bool Client::autoReconnect() const {
    this->sanity.check();
    return data->auto_reconnect.load();
}


WEBCFACE_NS_END
