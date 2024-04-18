#include <webcface/client.h>
#include "../message/unix_path.h"
#include "client_internal.h"
#include <curl/curl.h>
#include <string>
#include <chrono>
#include <thread>
#include <cstdint>
#include <cstdlib>

WEBCFACE_NS_BEGIN

void Internal::messageThreadMain(std::shared_ptr<Internal::ClientData> data,
                                 std::string host, int port) {
    while (!data->closing.load() && port > 0) {
        for (int attempt = 0; attempt < 2 && !data->closing.load(); attempt++) {
            CURL *handle = curl_easy_init();
            if (std::getenv("WEBCFACE_TRACE") != nullptr) {
                curl_easy_setopt(handle, CURLOPT_VERBOSE, 1L);
            }
            if (attempt == 0) {
                curl_easy_setopt(
                    handle, CURLOPT_UNIX_SOCKET_PATH,
                    Message::unixSocketPath(port).string().c_str());
            }
            // todo: WSLInterop
            curl_easy_setopt(handle, CURLOPT_URL,
                             ("ws://" + host + "/").c_str());
            curl_easy_setopt(handle, CURLOPT_PORT, static_cast<long>(port));
            curl_easy_setopt(handle, CURLOPT_CONNECT_ONLY, 2L);
            CURLcode ret = curl_easy_perform(handle);
            if (ret != CURLE_OK) {
                data->logger_internal->trace("connection failed {}",
                                             static_cast<int>(ret));
            } else {
                {
                    std::lock_guard lock(data->connect_state_m);
                    data->connected.store(true);
                }
                data->connect_state_cond.notify_all();
                data->logger_internal->debug("connected");
                std::string buf_s;
                do {
                    bool closed = false;
                    // 受信ループ
                    while (true) {
                        std::size_t rlen = 0;
                        const curl_ws_frame *meta = nullptr;
                        char buffer[1024];
                        do {
                            ret = curl_ws_recv(handle, buffer, sizeof(buffer),
                                               &rlen, &meta);
                            if (meta && meta->flags & CURLWS_CLOSE) {
                                data->logger_internal->debug(
                                    "connection closed");
                                closed = true;
                                break;
                            } else if (meta &&
                                       static_cast<std::size_t>(meta->offset) >
                                           buf_s.size()) {
                                buf_s.append(
                                    static_cast<std::size_t>(meta->offset) -
                                        buf_s.size(),
                                    '\0');
                                buf_s.append(buffer, rlen);
                            } else if (meta &&
                                       static_cast<std::size_t>(meta->offset) <
                                           buf_s.size()) {
                                buf_s.replace(
                                    static_cast<std::size_t>(meta->offset),
                                    rlen, buffer, rlen);
                            } else {
                                buf_s.append(buffer, rlen);
                            }
                        } while (meta && meta->bytesleft > 0);
                        if (buf_s.empty()) {
                            break;
                        }
                        if (ret == CURLE_OK && meta && meta->bytesleft == 0) {
                            data->logger_internal->trace(
                                "message received len={}", buf_s.size());
                            data->recv_queue.push(buf_s);
                            std::size_t sent;
                            curl_ws_send(handle, nullptr, 0, &sent, 0,
                                         CURLWS_PONG);
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
                        curl_ws_send(handle, msg->c_str(), msg->size(), &sent,
                                     0, CURLWS_BINARY);
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
            curl_easy_cleanup(handle);
        }
        if (!data->closing.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}

WEBCFACE_NS_END
