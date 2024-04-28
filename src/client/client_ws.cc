#include "client_ws.h"
#include "client_internal.h"
#include <webcface/client.h>
#include "../message/unix_path.h"
#include <curl/curl.h>
#include <string>
#include <chrono>
#include <thread>
#include <cstdint>
#include <cstdlib>

WEBCFACE_NS_BEGIN
namespace Internal {
namespace WebSocket {

void init(std::shared_ptr<Internal::ClientData> data) {
    if (data->host.empty()) {
        data->host = "127.0.0.1";
    }
    // try TCP, unixSocketPathWSLInterop and unixSocketPath
    // use latter if multiple connections were available
    data->curl_handles.fill(nullptr);
    std::array<std::optional<CURLcode>, 3> curl_result;
    curl_result.fill(std::nullopt);
    std::array<std::string, 3> paths;
    for (std::size_t attempt = 0;
         attempt < data->curl_handles.size() && !data->closing.load();
         attempt++) {
        CURL *handle = data->curl_handles[attempt] = curl_easy_init();
        curl_result[attempt] = std::nullopt;
        if (std::getenv("WEBCFACE_TRACE") != nullptr) {
            curl_easy_setopt(handle, CURLOPT_VERBOSE, 1L);
        }
        switch (attempt) {
        case 0:
            paths[attempt] = data->host + ':' + std::to_string(data->port);
            curl_easy_setopt(handle, CURLOPT_URL,
                             ("ws://" + data->host + "/").c_str());
            break;
        case 1:
            if (data->host != "127.0.0.1") {
                continue;
            }
            if (Message::Path::detectWSL1()) {
                paths[attempt] =
                    Message::Path::unixSocketPathWSLInterop(data->port)
                        .string();
                curl_easy_setopt(handle, CURLOPT_UNIX_SOCKET_PATH,
                                 paths[attempt].c_str());
                curl_easy_setopt(handle, CURLOPT_URL,
                                 ("ws://" + data->host + "/").c_str());
                break;
            }
            if (Message::Path::detectWSL2() &&
                !(curl_result[0] && curl_result[0] == CURLE_OK)) {
                std::string win_host = Message::Path::wsl2Host();
                if (!win_host.empty()) {
                    paths[attempt] =
                        win_host + ':' + std::to_string(data->port);
                    curl_easy_setopt(handle, CURLOPT_URL,
                                     ("ws://" + win_host + "/").c_str());
                    break;
                }
            }
            continue;
        case 2:
            if (data->host != "127.0.0.1") {
                continue;
            }
            paths[attempt] = Message::Path::unixSocketPath(data->port).string();
            curl_easy_setopt(handle, CURLOPT_UNIX_SOCKET_PATH,
                             paths[attempt].c_str());
            curl_easy_setopt(handle, CURLOPT_URL,
                             ("ws://" + data->host + "/").c_str());
            break;
        }
        data->logger_internal->trace("trying {}...", paths[attempt]);
        curl_easy_setopt(handle, CURLOPT_PORT, static_cast<long>(data->port));
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
    for (int attempt = static_cast<int>(data->curl_handles.size() - 1);
         attempt >= 0; attempt--) {
        if (curl_result[attempt] && *curl_result[attempt] == CURLE_OK) {
            handle = static_cast<CURL *>(data->curl_handles[attempt]);
            path = std::move(paths[attempt]);
            break;
        }
    }
    data->current_curl_handle = handle;
    if (handle) {
        send(data, data->syncDataFirst());
        {
            std::lock_guard lock(data->connect_state_m);
            data->connected.store(true);
            data->connect_state_cond.notify_all();
        }
        data->logger_internal->debug("connected to {}", path);
        data->current_curl_closed = false;
    }
}
void close(std::shared_ptr<Internal::ClientData> data) {
    {
        std::lock_guard lock(data->connect_state_m);
        data->connected.store(false);
        data->connect_state_cond.notify_all();
    }
    data->message_queue->clear();
    data->syncDataFirst(); // 次の接続時の最初のメッセージ

    for (auto &handle : data->curl_handles) {
        if (handle) {
            curl_easy_cleanup(static_cast<CURL *>(handle));
        }
    }
}
void recv(std::shared_ptr<Internal::ClientData> data) {
    // data->logger_internal->trace("loop");
    CURL *handle = static_cast<CURL *>(data->current_curl_handle);
    CURLcode ret;

    std::size_t rlen = 0;
    // data->logger_internal->trace("recv");
    const curl_ws_frame *meta = nullptr;
    char buffer[1024];
    do {
        ret = curl_ws_recv(handle, buffer, sizeof(buffer), &rlen, &meta);
        if (meta && meta->flags & CURLWS_CLOSE) {
            data->logger_internal->debug("connection closed");
            data->current_curl_closed = true;
            break;
        } else if (meta && static_cast<std::size_t>(meta->offset) >
                               data->current_ws_buf.size()) {
            data->current_ws_buf.append(static_cast<std::size_t>(meta->offset) -
                                            data->current_ws_buf.size(),
                                        '\0');
            data->current_ws_buf.append(buffer, rlen);
        } else if (meta && static_cast<std::size_t>(meta->offset) <
                               data->current_ws_buf.size()) {
            data->current_ws_buf.replace(static_cast<std::size_t>(meta->offset),
                                         rlen, buffer, rlen);
        } else {
            data->current_ws_buf.append(buffer, rlen);
        }
        if (ret != CURLE_AGAIN && ret != CURLE_OK) {
            data->logger_internal->debug("connection closed {}",
                                         static_cast<int>(ret));
            data->current_curl_closed = true;
            break;
        }
    } while (meta && meta->bytesleft > 0 && ret != CURLE_AGAIN);
    if (ret == CURLE_OK && meta && meta->bytesleft == 0 &&
        !data->current_ws_buf.empty()) {
        data->logger_internal->trace("message received len={}",
                                     data->current_ws_buf.size());
        std::size_t sent;
        curl_ws_send(handle, nullptr, 0, &sent, 0, CURLWS_PONG);
        // data->recv_queue.push(data->current_ws_buf);
        data->onRecv(data->current_ws_buf);
        data->current_ws_buf.clear();
    }
}
void send(std::shared_ptr<Internal::ClientData> data, const std::string &msg) {
    data->logger_internal->trace("sending message");
    std::size_t sent;
    CURL *handle = static_cast<CURL *>(data->current_curl_handle);
    curl_ws_send(handle, msg.c_str(), msg.size(), &sent, 0, CURLWS_BINARY);
    // data->logger_internal->trace("sending done");
}

} // namespace WebSocket
} // namespace Internal
WEBCFACE_NS_END
