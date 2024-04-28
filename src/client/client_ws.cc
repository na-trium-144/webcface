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

void init(std::shared_ptr<Internal::ClientData> data, bool use_cb) {
    if (data->host.empty()) {
        data->host = "127.0.0.1";
    }
    // try TCP, unixSocketPathWSLInterop and unixSocketPath
    // use latter if multiple connections were available
    for (std::size_t attempt = 0; attempt < 3 && !data->closing.load();
         attempt++) {
        CURL *handle = curl_easy_init();
        if (std::getenv("WEBCFACE_TRACE") != nullptr) {
            curl_easy_setopt(handle, CURLOPT_VERBOSE, 1L);
        }
        switch (attempt) {
        case 2:
            data->current_curl_path =
                data->host + ':' + std::to_string(data->port);
            curl_easy_setopt(handle, CURLOPT_URL,
                             ("ws://" + data->host + "/").c_str());
            break;
        case 1:
            if (data->host != "127.0.0.1") {
                continue;
            }
            if (Message::Path::detectWSL1()) {
                data->current_curl_path =
                    Message::Path::unixSocketPathWSLInterop(data->port)
                        .string();
                curl_easy_setopt(handle, CURLOPT_UNIX_SOCKET_PATH,
                                 data->current_curl_path.c_str());
                curl_easy_setopt(handle, CURLOPT_URL,
                                 ("ws://" + data->host + "/").c_str());
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
            if (data->host != "127.0.0.1") {
                continue;
            }
            data->current_curl_path =
                Message::Path::unixSocketPath(data->port).string();
            curl_easy_setopt(handle, CURLOPT_UNIX_SOCKET_PATH,
                             data->current_curl_path.c_str());
            curl_easy_setopt(handle, CURLOPT_URL,
                             ("ws://" + data->host + "/").c_str());
            break;
        }
        data->logger_internal->trace("trying {}...", data->current_curl_path);
        curl_easy_setopt(handle, CURLOPT_PORT, static_cast<long>(data->port));

        if (use_cb) {
            curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, cb);
            curl_easy_setopt(handle, CURLOPT_WRITEDATA, data.get());
        } else {
            curl_easy_setopt(handle, CURLOPT_CONNECT_ONLY, 2L);
        }
        data->current_curl_success = false;
        data->current_curl_handle = handle;
        // use_cbがtrueならここでcurl内部のループに入り繰り返しcbが呼ばれる
        // falseなら即座にreturnしretに結果が入る
        auto ret = curl_easy_perform(handle);

        if (ret != CURLE_OK) {
            data->logger_internal->trace("connection failed {}",
                                         static_cast<int>(ret));
        }
        if (!use_cb && ret == CURLE_OK) {
            data->current_curl_success = true;
            initSuccess(data.get());
            return;
        }
        if (use_cb && data->current_curl_success) {
            deinit(data);
            return;
        }
        curl_easy_cleanup(handle);
        data->current_curl_handle = nullptr;
    }
}
void initSuccess(Internal::ClientData *data) {
    send(data, data->syncDataFirst());
    {
        std::lock_guard lock(data->connect_state_m);
        data->connected.store(true);
        data->connect_state_cond.notify_all();
    }
    data->logger_internal->debug("connected to {}", data->current_curl_path);
    data->current_curl_closed = false;
}
void deinit(std::shared_ptr<Internal::ClientData> data) {
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

size_t cb(char *buffer, size_t size, size_t nmemb, void *clientp) {
    auto data = static_cast<Internal::ClientData *>(clientp);
    data->logger_internal->trace("callback called");
    if (!data->current_curl_success) {
        data->current_curl_success = true;
        initSuccess(data);
    }

    CURL *handle = static_cast<CURL *>(data->current_curl_handle);
    size_t rlen = size * nmemb;
    const curl_ws_frame *meta = curl_ws_meta(handle);
    recvFrame(data, buffer, rlen, meta);
    return rlen;
}

void recvFrame(Internal::ClientData *data, char *buffer, std::size_t rlen,
               const void *meta_v) {
    CURL *handle = static_cast<CURL *>(data->current_curl_handle);
    auto meta = static_cast<const curl_ws_frame *>(meta_v);
    if (meta && meta->flags & CURLWS_CLOSE) {
        data->logger_internal->debug("connection closed");
        data->current_curl_closed = true;
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
    if (meta && meta->bytesleft == 0 && !data->current_ws_buf.empty()) {
        data->logger_internal->trace("message received len={}",
                                     data->current_ws_buf.size());
        std::size_t sent;
        curl_ws_send(handle, nullptr, 0, &sent, 0, CURLWS_PONG);
        // data->recv_queue.push(data->current_ws_buf);
        data->onRecv(data->current_ws_buf);
        data->current_ws_buf.clear();
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
        recvFrame(data.get(), buffer, rlen, meta);
    } while (meta && meta->bytesleft > 0 && ret != CURLE_AGAIN);
}

void send(Internal::ClientData *data, const std::string &msg) {
    data->logger_internal->trace("sending message");
    std::size_t sent;
    CURL *handle = static_cast<CURL *>(data->current_curl_handle);
    curl_ws_send(handle, msg.c_str(), msg.size(), &sent, 0, CURLWS_BINARY);
    // data->logger_internal->trace("sending done");
}
void close(Internal::ClientData *data) {
    data->logger_internal->trace("closing");
    std::size_t sent;
    CURL *handle = static_cast<CURL *>(data->current_curl_handle);
    curl_ws_send(handle, nullptr, 0, &sent, 0, CURLWS_CLOSE);
    // data->logger_internal->trace("sending done");
}

} // namespace WebSocket
} // namespace Internal
WEBCFACE_NS_END
