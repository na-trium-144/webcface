#include <webcface/client.h>
#include "client_internal.h"
#include <curl/curl.h>
#include <string>
#include <chrono>
#include <thread>
#include <cstdint>
#include <cstdlib>

namespace webcface {

void Internal::messageThreadMain(std::shared_ptr<Internal::ClientData> data,
                                 std::string host, int port) {
    while (!data->closing.load() && port > 0) {
        CURL *handle = curl_easy_init();
        if (std::getenv("WEBCFACE_TRACE") != nullptr) {
            curl_easy_setopt(handle, CURLOPT_VERBOSE, 1L);
        }
        curl_easy_setopt(handle, CURLOPT_URL, ("ws://" + host).c_str());
        curl_easy_setopt(handle, CURLOPT_PORT, static_cast<long>(port));
        curl_easy_setopt(handle, CURLOPT_CONNECT_ONLY, 2L);
        CURLcode ret = curl_easy_perform(handle);
        if (ret != CURLE_OK) {
            data->logger_internal->trace("connection failed {}",
                                         static_cast<int>(ret));
        } else {
            data->logger_internal->debug("connected");
            data->sync_init.store(false);
            data->connected_.store(true);
            while (!data->closing.load() && data->connected_.load()) {
                while (true) {
                    std::size_t rlen = 0;
                    const curl_ws_frame *meta = nullptr;
                    char buffer[1024];
                    ret = curl_ws_recv(handle, buffer, sizeof(buffer), &rlen,
                                       &meta);
                    if (meta && meta->flags & CURLWS_CLOSE) {
                        data->logger_internal->debug("connection closed");
                        data->connected_.store(false);
                    }
                    if (rlen == 0) {
                        break;
                    }
                    data->logger_internal->trace("message received");
                    data->recv_queue.push(std::string(buffer, rlen));
                    std::size_t sent;
                    curl_ws_send(handle, nullptr, 0, &sent, 0, CURLWS_PONG);
                }
                if (ret != CURLE_AGAIN) {
                    data->logger_internal->debug("connection closed {}",
                                                 static_cast<int>(ret));
                    break;
                }
                auto msg =
                    data->message_queue.pop(std::chrono::milliseconds(0));
                if (msg) {
                    data->logger_internal->trace("sending message");
                    std::size_t sent;
                    curl_ws_send(handle, msg->c_str(), msg->size(), &sent, 0,
                                 CURLWS_BINARY);
                }
                std::this_thread::yield();
            }
        }
        curl_easy_cleanup(handle);
        data->connected_.store(false);
        if (!data->closing.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}

} // namespace webcface
