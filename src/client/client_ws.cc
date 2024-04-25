#include <webcface/client.h>
#include "../message/unix_path.h"
#include "client_internal.h"
#include <curl/curl.h>
#include <string>
#include <chrono>
#include <thread>
#include <cstdint>
#include <cstdlib>
#include <array>

WEBCFACE_NS_BEGIN

void Internal::messageThreadMain(std::shared_ptr<Internal::ClientData> data,
                                 std::string host, int port) {
    if (host.empty()) {
        host = "127.0.0.1";
    }
    while (!data->closing.load() && port > 0) {
        // try TCP, unixSocketPathWSLInterop and unixSocketPath
        // use latter if multiple connections were available
        std::array<CURL *, 3> handles;
        handles.fill(nullptr);
        std::array<std::optional<CURLcode>, 3> curl_result;
        curl_result.fill(std::nullopt);
        std::array<std::string, 3> paths;
        for (std::size_t attempt = 0;
             attempt < handles.size() && !data->closing.load(); attempt++) {
            CURL *handle = handles[attempt] = curl_easy_init();
            curl_result[attempt] = std::nullopt;
            if (std::getenv("WEBCFACE_TRACE") != nullptr) {
                curl_easy_setopt(handle, CURLOPT_VERBOSE, 1L);
            }
            switch (attempt) {
            case 0:
                paths[attempt] = host + ':' + std::to_string(port);
                curl_easy_setopt(handle, CURLOPT_URL,
                                 ("ws://" + host + "/").c_str());
                break;
            case 1:
                if (host != "127.0.0.1") {
                    continue;
                }
                paths[attempt] = Message::Path::unixSocketPath(port).string();
                curl_easy_setopt(handle, CURLOPT_UNIX_SOCKET_PATH,
                                 paths[attempt].c_str());
                curl_easy_setopt(handle, CURLOPT_URL,
                                 ("ws://" + host + "/").c_str());
                break;
            default:
                if (host != "127.0.0.1") {
                    continue;
                }
                if (Message::Path::detectWSL1()) {
                    paths[attempt] =
                        Message::Path::unixSocketPathWSLInterop(port).string();
                    curl_easy_setopt(handle, CURLOPT_UNIX_SOCKET_PATH,
                                     paths[attempt].c_str());
                    curl_easy_setopt(handle, CURLOPT_URL,
                                     ("ws://" + host + "/").c_str());
                    break;
                }
                if (Message::Path::detectWSL2() &&
                    !(curl_result[0] && curl_result[0] == CURLE_OK)) {
                    std::string win_host = Message::Path::wsl2Host();
                    if (!win_host.empty()) {
                        paths[attempt] = win_host + ':' + std::to_string(port);
                        curl_easy_setopt(handle, CURLOPT_URL,
                                         ("ws://" + win_host + "/").c_str());
                        break;
                    }
                }
                continue;
            }
            data->logger_internal->trace("trying {}...", paths[attempt]);
            curl_easy_setopt(handle, CURLOPT_PORT, static_cast<long>(port));
            curl_easy_setopt(handle, CURLOPT_CONNECT_ONLY, 2L);
            curl_result[attempt] = curl_easy_perform(handle);
            if (*curl_result[attempt] != CURLE_OK) {
                data->logger_internal->trace(
                    "connection failed {}",
                    static_cast<int>(*curl_result[attempt]));
            }
        }
        CURL *handle = nullptr;
        std::string path;
        for (int attempt = static_cast<int>(handles.size() - 1); attempt >= 0;
             attempt--) {
            if (curl_result[attempt] && *curl_result[attempt] == CURLE_OK) {
                handle = handles[attempt];
                path = std::move(paths[attempt]);
                break;
            }
        }
        if (handle != nullptr && !data->closing.load()) {
            {
                std::lock_guard lock(data->connect_state_m);
                data->connected.store(true);
            }
            data->connect_state_cond.notify_all();
            data->logger_internal->debug("connected to {}", path);
            std::string buf_s;
            do {
                bool closed = false;
                CURLcode ret;
                // 受信ループ
                while (true) {
                    std::size_t rlen = 0;
                    const curl_ws_frame *meta = nullptr;
                    char buffer[1024];
                    do {
                        ret = curl_ws_recv(handle, buffer, sizeof(buffer),
                                           &rlen, &meta);
                        if (meta && meta->flags & CURLWS_CLOSE) {
                            data->logger_internal->debug("connection closed");
                            closed = true;
                            break;
                        } else if (meta && static_cast<std::size_t>(
                                               meta->offset) > buf_s.size()) {
                            buf_s.append(
                                static_cast<std::size_t>(meta->offset) -
                                    buf_s.size(),
                                '\0');
                            buf_s.append(buffer, rlen);
                        } else if (meta && static_cast<std::size_t>(
                                               meta->offset) < buf_s.size()) {
                            buf_s.replace(
                                static_cast<std::size_t>(meta->offset), rlen,
                                buffer, rlen);
                        } else {
                            buf_s.append(buffer, rlen);
                        }
                    } while (meta && meta->bytesleft > 0);
                    if (buf_s.empty()) {
                        break;
                    }
                    if (ret == CURLE_OK && meta && meta->bytesleft == 0) {
                        data->logger_internal->trace("message received len={}",
                                                     buf_s.size());
                        data->recv_queue.push(buf_s);
                        std::size_t sent;
                        curl_ws_send(handle, nullptr, 0, &sent, 0, CURLWS_PONG);
                        buf_s.clear();
                    }
                }
                if (ret != CURLE_AGAIN && ret != CURLE_OK) {
                    data->logger_internal->debug("connection closed {}",
                                                 static_cast<int>(ret));
                    closed = true;
                }
                if (closed) {
                    break;
                }
                // 最低一回はqueueが空になるまで送信する。
                while (auto msg = data->message_queue->pop()) {
                    data->logger_internal->trace("sending message");
                    std::size_t sent;
                    curl_ws_send(handle, msg->c_str(), msg->size(), &sent, 0,
                                 CURLWS_BINARY);
                }
                std::this_thread::yield();
            } while (!data->closing.load());
            {
                std::lock_guard lock(data->connect_state_m);
                data->connected.store(false);
            }
            data->connect_state_cond.notify_all();
            while (data->message_queue->pop())
                ;
            data->syncDataFirst(); // 次の接続時の最初のメッセージ
        }
        for (auto &handle : handles) {
            if (handle) {
                curl_easy_cleanup(handle);
            }
        }
        if (!data->closing.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}

WEBCFACE_NS_END
