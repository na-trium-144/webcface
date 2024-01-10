#include "../message/message.h"
#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <thread>
#include "dummy_server.h"
#include <crow.h>
#include "../server/custom_logger.h"

using namespace WEBCFACE_NS;
DummyServer::~DummyServer() {
    std::static_pointer_cast<crow::SimpleApp>(server_)->stop();
    t.join();
}
DummyServer::DummyServer()
    : t([this] {
          static int sn = 0;
          dummy_logger =
              spdlog::stdout_color_mt("dummy_server_" + std::to_string(sn++));
          dummy_logger->set_level(spdlog::level::trace);

          auto crow_logger =
              spdlog::stdout_color_mt("crow_server_" + std::to_string(sn++));
          crow_logger->set_level(spdlog::level::trace);
          static std::unique_ptr<Server::CustomLogger> crow_custom_logger;
          crow_custom_logger =
              std::make_unique<Server::CustomLogger>(crow_logger);
          crow::logger::setHandler(crow_custom_logger.get());

          auto server = std::make_shared<crow::SimpleApp>();
          server_ = std::static_pointer_cast<void>(server);


          CROW_WEBSOCKET_ROUTE((*server), "/")
              // route.websocket<std::remove_reference<decltype(*app)>::type>(app.get())
              .onopen([&](crow::websocket::connection &conn) {
                  connPtr = &conn;
                  dummy_logger->info("ws_open");
              })
              .onclose([&](crow::websocket::connection &, const std::string &) {
                  connPtr = nullptr;
                  dummy_logger->info("ws_close");
              })
              .onmessage([&](crow::websocket::connection &,
                             const std::string &data, bool) {
                  dummy_logger->info("ws_message");
                  auto unpacked = Message::unpack(data, dummy_logger);
                  for (const auto &m : unpacked) {
                      dummy_logger->info("kind {}", m.first);
                      recv_data.push_back(m);
                  }
              });

          server->port(17530).run();
          std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }) {}

void DummyServer::send(std::string msg) {
    if (connPtr) {
        dummy_logger->info("send {} bytes", msg.size());
        reinterpret_cast<crow::websocket::connection *>(connPtr)->send_binary(
            msg);
    }
}

bool DummyServer::connected() { return connPtr != nullptr; }
