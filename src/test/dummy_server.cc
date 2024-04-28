#include "../message/message.h"
#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <thread>
#include "dummy_server.h"
#include <crow.h>
#include "../server/custom_logger.h"
#include "../message/unix_path.h"
#ifdef _WIN32
#include <fileapi.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#endif

using namespace webcface;
DummyServer::~DummyServer() {
    std::static_pointer_cast<crow::SimpleApp>(server_)->stop();
    t.join();
}
DummyServer::DummyServer(bool use_unix)
    : t([this, use_unix] {
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
                  std::lock_guard lock(m);
                  connPtr = &conn;
                  dummy_logger->info("ws_open");
                  conn.send_binary("");
              })
              .onclose([&](crow::websocket::connection &, const std::string &) {
                  std::lock_guard lock(m);
                  connPtr = nullptr;
                  dummy_logger->info("ws_close");
              })
              .onmessage([&](crow::websocket::connection &,
                             const std::string &data, bool) {
                  std::lock_guard lock(m);
                  dummy_logger->info("ws_message");
                  auto unpacked = Message::unpack(data, dummy_logger);
                  for (const auto &m : unpacked) {
                      dummy_logger->info("kind {}", m.first);
                      recv_data.push_back(m);
                  }
              });

          if (use_unix) {
              auto unix_path = Message::Path::unixSocketPath(17530);
              Message::Path::initUnixSocket(unix_path, dummy_logger);
              server->unix_path(unix_path.string()).run();
          } else {
              server->port(17530).run();
          }
          std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }) {}

void DummyServer::send(std::string msg) {
    std::lock_guard lock(m);
    if (connPtr) {
        dummy_logger->info("send {} bytes", msg.size());
        reinterpret_cast<crow::websocket::connection *>(connPtr)->send_binary(
            msg);
    }
}

bool DummyServer::connected() { return connPtr != nullptr; }
