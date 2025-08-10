#include "webcface/internal/client_ws.h"
#include "webcface/common/internal/safe_global.h"
#include "webcface/internal/client_internal.h"
#include "webcface/client.h"
#include "webcface/common/internal/unix_path.h"
#include <curl/curl.h>
#include <string>
#include <cstdlib>
#include <thread>
#include <cassert>

WEBCFACE_NS_BEGIN
namespace internal {
namespace WebSocket {

/* if the request did not complete correctly, show the error
information. if no detailed error information was written to errbuf
show the more generic information from curl_easy_strerror instead.
*/
static std::string_view
getCurlError(const std::shared_ptr<internal::ClientData> &data, CURLcode res) {
    if (res != CURLE_OK) {
        size_t len = std::strlen(data->curl_err_buffer.data());
        if (len) {
            if (data->curl_err_buffer[len - 1] == '\n') {
                --len;
            }
            return {data->curl_err_buffer.data(), len};
        } else {
            return curl_easy_strerror(res);
        }
    }
    return {};
}

static internal::SafeGlobal<std::mutex> init_m;
static int global_init_count = 0;
void globalInit() {
    if (init_m) {
        std::lock_guard lock(*init_m);
        if (global_init_count++ == 0) {
            curl_global_init(CURL_GLOBAL_ALL);
            curl_global_trace("+ws");
        }
    }
}
void globalDeinit() {
    if (init_m) {
        std::lock_guard lock(*init_m);
        if (--global_init_count == 0) {
            curl_global_cleanup();
        }
    }
}

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
        data->curl_err_buffer.resize(CURL_ERROR_SIZE);
        curl_easy_setopt(handle, CURLOPT_ERRORBUFFER,
                         data->curl_err_buffer.data());
        data->current_ws_buf.clear();
        if (std::getenv("WEBCFACE_TRACE") != nullptr) {
            curl_easy_setopt(handle, CURLOPT_VERBOSE, 1L);
        }
        switch (attempt) {
        case 2:
            data->current_curl_path = strJoin<char>(data->host.decode(), ":",
                                                    std::to_string(data->port));
            curl_easy_setopt(
                handle, CURLOPT_URL,
                strJoin<char>("ws://", data->host.decode(), "/").c_str());
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
                curl_easy_setopt(
                    handle, CURLOPT_URL,
                    strJoin<char>("ws://", data->host.decode(), "/").c_str());
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
            curl_easy_setopt(
                handle, CURLOPT_URL,
                strJoin<char>("ws://", data->host.decode(), "/").c_str());
            break;
        }
        data->logger_internal->debug("trying {}...", data->current_curl_path);
        curl_easy_setopt(handle, CURLOPT_PORT, static_cast<long>(data->port));
        curl_easy_setopt(handle, CURLOPT_CONNECT_ONLY, 2L);
        auto ret = curl_easy_perform(handle);
        if (ret == CURLE_OK) {
            data->logger_internal->info("connected to {}",
                                        data->current_curl_path);
            data->current_curl_connected = true;
            return;
        } else {
            data->logger_internal->debug("connection failed {}: {}",
                                         static_cast<int>(ret),
                                         getCurlError(data, ret));
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
    data->logger_internal->info("connection closed");
}
bool recv(const std::shared_ptr<internal::ClientData> &data,
          const std::function<void(std::string &&)> &cb) {
    CURL *handle = static_cast<CURL *>(data->current_curl_handle);
    assert(handle);
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
                data->logger_internal->debug("recv failed {}: {}",
                                             static_cast<int>(ret),
                                             getCurlError(data, ret));
                WebSocket::close(data);
                break;
            }
            if (ret == CURLE_OK && meta && meta->bytesleft == 0 &&
                !data->current_ws_buf.empty()) {
                std::size_t sent;
                curl_ws_send(handle, nullptr, 0, &sent, 0, CURLWS_PONG);
                recv_ok = true;
            }
        }
        if (recv_ok) { // ここにはmutexかからない
            // data->recv_queue.push(data->current_ws_buf);
            // data->onRecv(data->current_ws_buf);
            cb(std::move(data->current_ws_buf));
            data->current_ws_buf.clear();
            has_recv = true;
        }
    } while (ret != CURLE_AGAIN);
    return has_recv;
}
void send(const std::shared_ptr<internal::ClientData> &data,
          const std::string &msg) {
    // std::lock_guard ws_lock(data->curl_m);
    std::size_t sent_total = 0;
    CURL *handle = static_cast<CURL *>(data->current_curl_handle);
    assert(handle);
    while (true) {
        std::size_t sent;
        auto ret =
            curl_ws_send(handle, msg.c_str() + sent_total,
                         msg.size() - sent_total, &sent, 0, CURLWS_BINARY);
        sent_total += sent;
        if (ret == CURLE_AGAIN || ret == CURLE_OK) {
            if (sent_total < msg.size()) {
                std::this_thread::sleep_for(std::chrono::microseconds(10));
                continue;
            } else {
                break;
            }
        } else {
            data->logger_internal->error("error sending message {}: {}",
                                         static_cast<int>(ret),
                                         getCurlError(data, ret));
            WebSocket::close(data);
            break;
        }
    }
}

} // namespace WebSocket
} // namespace internal
WEBCFACE_NS_END
