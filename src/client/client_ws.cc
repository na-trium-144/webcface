#include "client_ws.h"
#include "client_internal.h"
#include <bit>
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

void init(const std::shared_ptr<Internal::ClientData> &data) {
    if (data->host.empty()) {
        data->host = u8"127.0.0.1";
    }
    std::string host_s = Encoding::decode(data->host);

    // try TCP, unixSocketPathWSLInterop and unixSocketPath
    // use latter if multiple connections were available
    for (std::size_t attempt = 0; attempt < 3 && !data->closing.load();
         attempt++) {
        CURL *handle = data->current_curl_handle = curl_easy_init();
        data->current_curl_closed = false;
        data->current_ws_buf.clear();
        if (std::getenv("WEBCFACE_TRACE") != nullptr) {
            curl_easy_setopt(handle, CURLOPT_VERBOSE, 1L);
        }
        switch (attempt) {
        case 2:
            data->current_curl_path = host_s + ':' + std::to_string(data->port);
            curl_easy_setopt(handle, CURLOPT_URL,
                             ("ws://" + host_s + "/").c_str());
            break;
        case 1:
            if (host_s != "127.0.0.1") {
                continue;
            }
            if (Message::Path::detectWSL1()) {
                data->current_curl_path =
                    Message::Path::unixSocketPathWSLInterop(data->port)
                        .string();
                curl_easy_setopt(handle, CURLOPT_UNIX_SOCKET_PATH,
                                 data->current_curl_path.c_str());
                curl_easy_setopt(handle, CURLOPT_URL,
                                 ("ws://" + host_s + "/").c_str());
                break;
            }
            if (Message::Path::detectWSL2()) {
                std::string win_host = Message::Path::wsl2Host();
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
            if (host_s != "127.0.0.1") {
                continue;
            }
            data->current_curl_path =
                Message::Path::unixSocketPath(data->port).string();
            curl_easy_setopt(handle, CURLOPT_UNIX_SOCKET_PATH,
                             data->current_curl_path.c_str());
            curl_easy_setopt(handle, CURLOPT_URL,
                             ("ws://" + host_s + "/").c_str());
            break;
        }
        data->logger_internal->trace("trying {}...", data->current_curl_path);
        curl_easy_setopt(handle, CURLOPT_PORT, static_cast<long>(data->port));
        curl_easy_setopt(handle, CURLOPT_CONNECT_ONLY, 2L);
        auto ret = curl_easy_perform(handle);
        if (ret == CURLE_OK) {
            send(data, data->syncDataFirst());
            {
                std::lock_guard lock(data->connect_state_m);
                data->connected.store(true);
                data->connect_state_cond.notify_all();
            }
            data->logger_internal->debug("connected to {}",
                                         data->current_curl_path);
            return;
        } else {
            data->logger_internal->trace("connection failed {}",
                                         static_cast<int>(ret));
            curl_easy_cleanup(handle);
            data->current_curl_handle = nullptr;
        }
    }
}
void close(const std::shared_ptr<Internal::ClientData> &data) {
    {
        std::lock_guard lock(data->connect_state_m);
        data->connected.store(false);
        data->connect_state_cond.notify_all();
    }
    if (data->current_curl_handle) {
        curl_easy_cleanup(static_cast<CURL *>(data->current_curl_handle));
        data->current_curl_handle = nullptr;
    }
}
void recv(const std::shared_ptr<Internal::ClientData> &data) {
    CURL *handle = static_cast<CURL *>(data->current_curl_handle);
    CURLcode ret;
    // data->logger_internal->trace("recv");
    do {
        std::size_t rlen = 0;
        const curl_ws_frame *meta = nullptr;
        char buffer[1024];
        bool recv_ok = false;
        {
            std::lock_guard ws_lock(data->ws_m);
            ret = curl_ws_recv(handle, buffer, sizeof(buffer), &rlen, &meta);
            if (meta && meta->flags & CURLWS_CLOSE) {
                data->logger_internal->debug("connection closed");
                data->current_curl_closed = true;
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
                data->current_curl_closed = true;
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
            data->onRecv(data->current_ws_buf);
            data->current_ws_buf.clear();
        }
    } while (ret != CURLE_AGAIN);
}
void send(const std::shared_ptr<Internal::ClientData> &data,
          const std::string &msg) {
    std::lock_guard ws_lock(data->ws_m);
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
} // namespace Internal
WEBCFACE_NS_END
