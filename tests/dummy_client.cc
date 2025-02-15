#include <curl/curl.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <thread>
#include "dummy_client.h"
#include "webcface/common/internal/unix_path.h"
#include "webcface/internal/client_ws.h"

using namespace webcface;
DummyClient::~DummyClient() {
    closing.store(true);
    t.join();
    internal::WebSocket::globalDeinit();
}
DummyClient::DummyClient(bool use_unix)
    : t([this, use_unix] {
          static std::atomic<int> sn = 1;
          internal::WebSocket::globalInit();
          CURL *handle = curl_easy_init();
          curl_easy_setopt(handle, CURLOPT_VERBOSE, 1L);
          if (use_unix) {
              curl_easy_setopt(
                  handle, CURLOPT_UNIX_SOCKET_PATH,
                  internal::unixSocketPath(27530).string().c_str());
          }
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
              {
                  std::lock_guard lock(client_m);
                  connected_ = true;
              }
              std::string buf_s;
              while (!closing.load()) {
                  while (true) {
                      std::size_t rlen = 0;
                      const curl_ws_frame *meta = nullptr;
                      char buffer[1024];
                      do {
                          ret = curl_ws_recv(handle, buffer, sizeof(buffer),
                                             &rlen, &meta);
                          if (meta && meta->flags & CURLWS_CLOSE) {
                              dummy_logger->debug("connection closed");
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
                          dummy_logger->trace("message received len={}",
                                              buf_s.size());
                          auto unpacked = message::unpack(buf_s, dummy_logger);
                          for (const auto &m : unpacked) {
                              std::lock_guard lock(client_m);
                              dummy_logger->info("kind {}", m.first);
                              recv_data.push_back(m);
                          }
                          std::size_t sent;
                          curl_ws_send(handle, nullptr, 0, &sent, 0,
                                       CURLWS_PONG);
                          buf_s.clear();
                      }
                  }
                  if (ret != CURLE_AGAIN) {
                      dummy_logger->debug("connection closed {}",
                                          static_cast<int>(ret));
                      break;
                  }
                  {
                      std::lock_guard lock(client_m);
                      if (auto msg = msg_queue.pop()) {
                          dummy_logger->trace("sending message");
                          std::size_t sent_total = 0;
                          while (sent_total < msg->size()) {
                              std::size_t sent;
                              curl_ws_send(handle, msg->c_str() + sent_total,
                                           msg->size() - sent_total, &sent, 0,
                                           CURLWS_BINARY);
                              sent_total += sent;
                          }
                      }
                  }
                  std::this_thread::yield();
              }
          }
          curl_easy_cleanup(handle);
      }) {}

void DummyClient::send(std::string msg) {
    std::lock_guard lock(client_m);
    msg_queue.push(std::move(msg));
}
