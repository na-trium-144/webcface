#include <webcface/client.h>
#include <curl/curl.h>
#include <string>
#include <chrono>
#include <thread>
#include <cstdint>

namespace WebCFace {

void Client::messageThreadMain(std::shared_ptr<ClientData> data,
                               std::string host, int port) {
    while (!data->closing.load()) {
        CURL *handle = curl_easy_init();
        curl_easy_setopt(handle, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(handle, CURLOPT_URL, ("ws://" + host).c_str());
        curl_easy_setopt(handle, CURLOPT_PORT, static_cast<long>(port));
        curl_easy_setopt(handle, CURLOPT_CONNECT_ONLY, 2L);
        CURLcode ret = curl_easy_perform(handle);
        if (ret != CURLE_OK) {
            data->logger_internal->error("connection error {}", ret);
        } else {
            data->logger_internal->debug("connected");
            data->sync_init.store(false);
            data->connected_.store(true);
            while (!data->closing.load()) {
                std::size_t rlen;
                const curl_ws_frame *meta;
                char buffer[1024];
                CURLcode ret =
                    curl_ws_recv(handle, buffer, sizeof(buffer), &rlen, &meta);
                if (ret == CURLE_OK) {
                    data->logger_internal->trace("message received");
                    data->recv_queue.push(std::string(buffer, rlen));
                } else if (ret != CURLE_AGAIN) {
                    data->logger_internal->debug("connection closed");
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

} // namespace WebCFace