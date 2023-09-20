#include "../message/message.h"
#include <cinatra.hpp>
#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <thread>
#include "dummy_server.h"

using namespace WebCFace;
DummyServer::~DummyServer() {
    std::static_pointer_cast<cinatra::http_server>(server_)->stop();
    t.join();
}
DummyServer::DummyServer()
    : t([this] {
          using namespace cinatra;
          static int sn = 0;
          auto dummy_logger =
              spdlog::stdout_color_mt("dummy_server_" + std::to_string(sn++));
          dummy_logger->set_level(spdlog::level::trace);

          auto server = std::make_shared<http_server>(1);
          server_ = std::static_pointer_cast<void>(server);

          server->listen("0.0.0.0", "17530");
          server->set_http_handler<GET, POST>(
              "/", [&](request &req, response &res) {
                  req.on(ws_open, [&](request &req) {
                      connPtr = req.get_conn<cinatra::NonSSL>();
                      dummy_logger->info("ws_open");
                  });
                  req.on(ws_message, [&](request &req) {
                      dummy_logger->info("ws_message");
                      auto part_data = req.get_part_data();
                      std::string str =
                          std::string(part_data.data(), part_data.length());
                      auto unpacked = Message::unpack(str, dummy_logger);
                      for (const auto &m : unpacked) {
                          dummy_logger->info("kind {}", m.first);
                          recv_data.push_back(m);
                      }
                      // なんか送り返さないと受信できなくなるっぽい? 謎
                      send("");
                  });

                  req.on(ws_error,
                         [&](request &req) { dummy_logger->info("ws_error"); });

                  req.on(ws_close,
                         [&](request &req) { dummy_logger->info("ws_close"); });
              });
          server->run();
      }) {}

void DummyServer::send(std::string msg) {
    std::static_pointer_cast<cinatra::connection<cinatra::NonSSL>>(connPtr)
        ->send_ws_binary(msg);
}

bool DummyServer::connected() { return connPtr != nullptr; }


DummyClient::~DummyClient() {
    async_simple::coro::syncAwait(
        std::static_pointer_cast<cinatra::coro_http_client>(client_)
            ->async_send_ws_close());
    t.join();
}
DummyClient::DummyClient()
    : t([this] {
          using namespace cinatra;
          static int sn = 0;
          auto dummy_logger =
              spdlog::stdout_color_mt("dummy_client_" + std::to_string(sn++));
          dummy_logger->set_level(spdlog::level::trace);

          auto client = std::make_shared<coro_http_client>();
          client_ = std::static_pointer_cast<void>(client);
          auto client_keep =
              std::make_shared<std::shared_ptr<coro_http_client>>(client);

          client->on_ws_msg([this, client_keep, dummy_logger](resp_data rdata) {
              if (rdata.net_err) {
                  dummy_logger->error("recv error {}", rdata.net_err.message());
                  client_keep->reset();
              } else {
                  dummy_logger->trace("message received");

                  std::string str = std::string(rdata.resp_body);
                  auto unpacked = Message::unpack(str, dummy_logger);
                  for (const auto &m : unpacked) {
                      dummy_logger->info("kind {}", m.first);
                      recv_data.push_back(m);
                  }
              }
          });
          client->on_ws_close([client_keep, dummy_logger](auto &&) {
              dummy_logger->debug("connection closed");
              client_keep->reset();
          });
          async_simple::coro::syncAwait(
              client->async_ws_connect("ws://127.0.0.1:27530"));
      }) {}

void DummyClient::send(std::string msg) {
    async_simple::coro::syncAwait(
        std::static_pointer_cast<cinatra::coro_http_client>(client_)
            ->async_send_ws(msg, true, cinatra::opcode::binary));
}
