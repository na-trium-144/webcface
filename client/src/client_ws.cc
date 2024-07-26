#include "webcface/internal/client_ws.h"
#include "webcface/internal/client_internal.h"
#include "webcface/client.h"
#include "webcface/internal/unix_path.h"
#include <curl/curl.h>
#include <string>
#include <cstdlib>

WEBCFACE_NS_BEGIN
namespace internal {
namespace WebSocket {

void init(const std::shared_ptr<internal::ClientData> &data) {
    if (data->host.empty()) {
        data->host = SharedString::fromU8String("127.0.0.1");
    }

    // try TCP, unixSocketPathWSLInterop and unixSocketPath
    // use latter if multiple connections were available
    for (std::size_t attempt = 0; attempt < 3 && !data->closing.load();
         attempt++) {
        // std::lock_guard ws_lock(data->curl_m);
        CURL *handle = data->current_curl_handle = curl_easy_init();
        data->current_ws_buf.clear();
        if (std::getenv("WEBCFACE_TRACE") != nullptr) {
            curl_easy_setopt(handle, CURLOPT_VERBOSE, 1L);
        }
        switch (attempt) {
        case 2:
            data->current_curl_path =
                data->host.decode() + ':' + std::to_string(data->port);
            curl_easy_setopt(handle, CURLOPT_URL,
                             ("ws://" + data->host.decode() + "/").c_str());
            break;
        case 1:
            if (data->host.decode() != "127.0.0.1") {
                continue;
            }
            if (internal::detectWSL1()) {
                data->current_curl_path =
                    internal::unixSocketPathWSLInterop(data->port).string();
                curl_easy_setopt(handle, CURLOPT_UNIX_SOCKET_PATH,
                                 data->current_curl_path.c_str());
                curl_easy_setopt(handle, CURLOPT_URL,
                                 ("ws://" + data->host.decode() + "/").c_str());
                break;
            }
            if (internal::detectWSL2()) {
                std::string win_host = internal::wsl2Host();
                if (!win_host.empty()) {
                    data->current_curl_path =
                        win_host + ':' + std::to_string(data->port);
                    curl_easy_setopt(handle, CURLOPT_URL,
                                     ("ws://" + win_host + "/").c_str());
                    break;
                }
            }
            continue;
        case 0:
        default:
            if (data->host.decode() != "127.0.0.1") {
                continue;
            }
            data->current_curl_path =
                internal::unixSocketPath(data->port).string();
            curl_easy_setopt(handle, CURLOPT_UNIX_SOCKET_PATH,
                             data->current_curl_path.c_str());
            curl_easy_setopt(handle, CURLOPT_URL,
                             ("ws://" + data->host.decode() + "/").c_str());
            break;
        }
        data->logger_internal->trace("trying {}...", data->current_curl_path);
        curl_easy_setopt(handle, CURLOPT_PORT, static_cast<long>(data->port));
        curl_easy_setopt(handle, CURLOPT_CONNECT_ONLY, 2L);
        auto ret = curl_easy_perform(handle);
        if (ret == CURLE_OK) {
            send(data, data->syncDataFirst());
            data->logger_internal->debug("connected to {}",
                                         data->current_curl_path);
            data->current_curl_connected = true;
            return;
        } else {
            data->logger_internal->trace("connection failed {}",
                                         static_cast<int>(ret));
            curl_easy_cleanup(handle);
            data->current_curl_handle = nullptr;
        }
    }
}
void close(const std::shared_ptr<internal::ClientData> &data) {
    if (data->current_curl_handle) {
        curl_easy_cleanup(static_cast<CURL *>(data->current_curl_handle));
        data->current_curl_handle = nullptr;
    }
    data->current_curl_connected = false;
}
bool recv(const std::shared_ptr<internal::ClientData> &data,
          const std::function<void(const std::string &)> &cb) {
    CURL *handle = static_cast<CURL *>(data->current_curl_handle);
    CURLcode ret;
    // data->logger_internal->trace("recv");
    bool has_recv = false;
    do {
        std::size_t rlen = 0;
        const curl_ws_frame *meta = nullptr;
        char buffer[1024];
        bool recv_ok = false;
        {
            // std::lock_guard ws_lock(data->curl_m);
            ret = curl_ws_recv(handle, buffer, sizeof(buffer), &rlen, &meta);
            if (meta && meta->flags & CURLWS_CLOSE) {
                data->logger_internal->debug("connection closed");
                WebSocket::close(data);
                break;
            } else if (meta && static_cast<std::size_t>(meta->offset) >
                                   data->current_ws_buf.size()) {
                data->current_ws_buf.append(
                    static_cast<std::size_t>(meta->offset) -
                        data->current_ws_buf.size(),
                    '\0');
                data->current_ws_buf.append(buffer, rlen);
            } else if (meta && static_cast<std::size_t>(meta->offset) <
                                   data->current_ws_buf.size()) {
                data->current_ws_buf.replace(
                    static_cast<std::size_t>(meta->offset), rlen, buffer, rlen);
            } else {
                data->current_ws_buf.append(buffer, rlen);
            }
            if (ret != CURLE_AGAIN && ret != CURLE_OK) {
                data->logger_internal->debug("connection closed {}",
                                             static_cast<int>(ret));
                WebSocket::close(data);
                break;
            }
            if (ret == CURLE_OK && meta && meta->bytesleft == 0 &&
                !data->current_ws_buf.empty()) {
                data->logger_internal->trace("message received len={}",
                                             data->current_ws_buf.size());
                std::size_t sent;
                curl_ws_send(handle, nullptr, 0, &sent, 0, CURLWS_PONG);
                recv_ok = true;
            }
        }
        if (recv_ok) { // ここにはmutexかからない
            // data->recv_queue.push(data->current_ws_buf);
            // data->onRecv(data->current_ws_buf);
            cb(data->current_ws_buf);
            data->current_ws_buf.clear();
            has_recv = true;
        }
    } while (ret != CURLE_AGAIN);
    return has_recv;
}
void send(const std::shared_ptr<internal::ClientData> &data,
          const std::string &msg) {
    // std::lock_guard ws_lock(data->curl_m);
    data->logger_internal->trace("sending message {} bytes", msg.size());
    std::size_t sent;
    CURL *handle = static_cast<CURL *>(data->current_curl_handle);
    auto ret =
        curl_ws_send(handle, msg.c_str(), msg.size(), &sent, 0, CURLWS_BINARY);
    if (ret != CURLE_OK) {
        data->logger_internal->error("error sending message {}",
                                     static_cast<int>(ret));
    }
    if (sent != msg.size()) {
        data->logger_internal->error("failed to send message (sent = {} bytes)",
                                     sent);
    }
    // data->logger_internal->trace("sending done");
}

} // namespace WebSocket
} // namespace internal
WEBCFACE_NS_END
