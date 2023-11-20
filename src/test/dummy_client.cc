#include "../message/message.h"
#include <curl/curl.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <thread>
#include "dummy_client.h"

using namespace WebCFace;
DummyClient::~DummyClient() {
    closing.store(true);
    t.join();
}
DummyClient::DummyClient()
    : t([this] {
          static int sn = 1;
          CURL *handle = curl_easy_init();
          curl_easy_setopt(handle, CURLOPT_VERBOSE, 1L);
          curl_easy_setopt(handle, CURLOPT_URL, "ws://127.0.0.1");
          curl_easy_setopt(handle, CURLOPT_PORT, 27530L);
          curl_easy_setopt(handle, CURLOPT_CONNECT_ONLY, 2L);
          CURLcode ret = curl_easy_perform(handle);
          auto dummy_logger =
              spdlog::stdout_color_mt("dummy_client_" + std::to_string(sn++));
          dummy_logger->set_level(spdlog::level::trace);

          if (ret != CURLE_OK) {
              dummy_logger->error("connection error {}", static_cast<int>(ret));
          } else {
              dummy_logger->debug("connection done");
              while (!closing.load()) {
                  while (true) {
                      std::size_t rlen = 0;
                      const curl_ws_frame *meta = nullptr;
                      char buffer[1024];
                      ret = curl_ws_recv(handle, buffer, sizeof(buffer), &rlen,
                                         &meta);
                      if (meta && meta->flags & CURLWS_CLOSE) {
                          dummy_logger->debug("connection closed");
                          break;
                      }
                      if (rlen == 0) {
                          break;
                      }
                      dummy_logger->trace("message received");
                      auto unpacked = Message::unpack(std::string(buffer, rlen),
                                                      dummy_logger);
                      for (const auto &m : unpacked) {
                          dummy_logger->info("kind {}", m.first);
                          recv_data.push_back(m);
                      }
                      std::size_t sent;
                      curl_ws_send(handle, nullptr, 0, &sent, 0, CURLWS_PONG);
                  }
                  if (ret != CURLE_AGAIN) {
                      dummy_logger->debug("connection closed {}",
                                          static_cast<int>(ret));
                      break;
                  }
                  auto msg = msg_queue.pop(std::chrono::milliseconds(0));
                  if (msg) {
                      dummy_logger->trace("sending message");
                      std::size_t sent;
                      curl_ws_send(handle, msg->c_str(), msg->size(), &sent, 0,
                                   CURLWS_BINARY);
                  }
                  std::this_thread::yield();
              }
          }
          curl_easy_cleanup(handle);
      }) {}

void DummyClient::send(std::string msg) { msg_queue.push(msg); }
