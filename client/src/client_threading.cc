#include "webcface/client.h"
#include "webcface/log.h"
#include "webcface/internal/client_internal.h"
#include "webcface/internal/client_ws.h"
#include "webcface/internal/unlock.h"
#include "webcface/internal/logger.h"
#include <string>
#include <chrono>
#include <thread>
#include <spdlog/sinks/stdout_color_sinks.h>

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
    logger_buf = std::make_unique<LoggerBuf>(this);
    logger_os = std::make_unique<std::ostream>(logger_buf.get());
    logger_buf_w = std::make_unique<LoggerBufW>(this);
    logger_os_w = std::make_unique<std::wostream>(logger_buf_w.get());
    log_store.setRecv(name, std::make_shared<std::vector<LogLineData>>());
}

Client::~Client() {
    data->close();
    data->join();
}
void internal::ClientData::join() {
    if (ws_thread.joinable()) {
        ws_thread.join();
    }
    if (recv_thread.joinable()) {
        recv_thread.join();
    }
}

void internal::ClientData::start() {
    std::lock_guard lock(this->ws_m);
    this->do_ws_init = true;
    this->ws_cond.notify_all();
    if (!ws_thread.joinable()) {
        ws_thread = std::thread(internal::wsThreadMain, shared_from_this());
    }
    if (!recv_thread.joinable() && this->auto_recv.load()) {
        recv_thread = std::thread(internal::recvThreadMain, shared_from_this());
    }
}
void Client::close() { data->close(); }
void internal::ClientData::close() {
    std::lock_guard lock(this->ws_m);
    this->closing.store(true);
    this->ws_cond.notify_all();
}
bool Client::connected() const {
    std::lock_guard lock(data->ws_m);
    return data->connected;
}
void internal::wsThreadMain(const std::shared_ptr<ClientData> &data) {
    if (data->port <= 0) {
        return;
    }
    std::optional<std::chrono::steady_clock::time_point> last_connected,
        last_recv;
    while (true) {
        std::unique_lock lock(data->ws_m);
        if (!data->connected) {
            if (last_connected) {
                // すでに接続したことがあるなら最低10ms間隔を空ける
                data->ws_cond.wait_until(
                    lock, *last_connected + std::chrono::milliseconds(10),
                    [&] { return data->closing.load(); });
            }
            data->ws_cond.wait(lock, [&] {
                return data->closing.load() || data->do_ws_init ||
                       data->auto_reconnect.load();
            });
            if (data->closing.load()) {
                return;
            }
            { // do_ws_initがtrueまたはauto_reconnectがtrueなので再接続を行う
                ScopedUnlock un(lock);
                internal::WebSocket::init(data);
            }
            data->do_ws_init = false;
            data->connected = data->current_curl_connected;
            last_connected = std::chrono::steady_clock::now();
            last_recv = std::nullopt;

        } else {
            if (last_recv) {
                // syncデータがなければ、100us間隔を空ける
                data->ws_cond.wait_until(
                    lock, *last_recv + std::chrono::microseconds(100), [&] {
                        return data->closing.load() || data->do_ws_recv ||
                               !data->sync_queue.empty();
                    });
            }

            {
                // syncの前にrecv
                // この間にdo_ws_recvがtrueになることもあるだろうが、無問題
                // (完了したらどのみちfalseにする)
                ScopedUnlock un(lock);
                while (true) { // データがなくなるまで受信
                    bool has_recv = internal::WebSocket::recv(
                        data, [data](std::string msg) {
                            std::unique_lock lock(data->ws_m);
                            data->recv_queue.push(std::move(msg));
                            data->ws_cond.notify_all();
                        });
                    if (!has_recv) {
                        break;
                    }
                }
            }
            data->do_ws_recv = false;
            data->connected = data->current_curl_connected;
            if (!data->connected) {
                data->self_member_id = std::nullopt;
                data->sync_init_end = false;
            }
            data->ws_cond.notify_all();
            last_recv = std::chrono::steady_clock::now();

            while (data->connected && !data->sync_queue.empty()) {
                // sync
                std::string msg = std::move(data->sync_queue.front());
                data->sync_queue.pop();
                {
                    ScopedUnlock un(lock);
                    internal::WebSocket::send(data, msg);
                }
            }

            if (data->closing.load()) {
                {
                    ScopedUnlock un(lock);
                    internal::WebSocket::close(data);
                }
                data->connected = data->current_curl_connected;
                data->ws_cond.notify_all();
                return;
            }
        }
        data->ws_cond.notify_all();
    }
}
void Client::recvImpl(std::optional<std::chrono::microseconds> timeout) {
    data->recvImpl(timeout);
}
void internal::ClientData::recvImpl(
    std::optional<std::chrono::microseconds> timeout) {
    auto start_t = std::chrono::steady_clock::now();
    {
        std::unique_lock lock(this->ws_m);
        this->do_ws_recv = true;
        // 接続できてるなら、recvが完了するまで待つ
        // 1回のrecvはすぐ終わるのでtimeoutいらない
        this->ws_cond.wait(lock, [&] {
            return this->closing.load() ||
                   !(this->connected && this->do_ws_recv);
        });
    }
    std::unique_lock lock(this->ws_m);
    if (timeout) {
        // recv_queueにデータが入るまで待つ
        // 遅くともtimeout経過したら抜ける
        this->ws_cond.wait_until(lock, start_t + *timeout, [&] {
            return this->closing.load() || !this->recv_queue.empty() ||
                   (!this->connected && !this->auto_reconnect.load());
        });
        if (this->recv_queue.empty()) {
            // timeoutし、recv準備完了でない場合return
            return;
        }
    } else {
        // recv_queueにデータが入るまで無制限に待つ
        this->ws_cond.wait(lock, [&] {
            return this->closing.load() || !this->recv_queue.empty() ||
                   (!this->connected && !this->auto_reconnect.load());
        });
    }
    if (this->closing.load()) {
        return;
    }

    while (!this->recv_queue.empty()) {
        std::string msg = std::move(this->recv_queue.front());
        this->recv_queue.pop();
        {
            ScopedUnlock un(lock);
            this->onRecv(msg);
        }
    }
    // なにか受信できたらreturn
    // または (!this->connected && !this->auto_reconnect.load()) の場合もreturn
}
void internal::recvThreadMain(const std::shared_ptr<ClientData> &data) {
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
        data->recvImpl(std::nullopt);
    }
}

void Client::start() { data->start(); }
void Client::waitConnection() {
    data->start();
    bool first_loop = true;
    while (!data->closing.load()) {
        std::unique_lock lock(data->ws_m);
        if (!data->connected) {
            // 初回またはautoReconnectが有効なら接続完了まで待機
            if (first_loop || data->auto_reconnect.load()) {
                data->do_ws_init = true;
                data->ws_cond.notify_all();
                data->ws_cond.wait(lock, [this] {
                    return data->closing.load() || data->connected ||
                           !data->do_ws_init;
                });
            } else {
                return;
            }
        } else {
            if (data->sync_init_end) {
                return;
            } else {
                // autoRecvならsyncInit完了まで待機
                // そうでなければrecvを呼ぶ
                if (data->auto_recv.load()) {
                    data->ws_cond.wait(lock, [this] {
                        return data->closing.load() || !data->connected ||
                               data->sync_init_end;
                    });
                } else {
                    ScopedUnlock un(lock);
                    data->recvImpl(std::nullopt);
                }
            }
        }
        first_loop = false;
    }
}
void Client::autoRecv(bool enabled) {
    if (enabled /* && interval.count() > 0 */) {
        data->auto_recv.store(true);
    } else {
        data->auto_recv.store(false);
    }
}

void Client::autoReconnect(bool enabled) {
    data->auto_reconnect.store(enabled);
}
bool Client::autoReconnect() const { return data->auto_reconnect.load(); }


WEBCFACE_NS_END
