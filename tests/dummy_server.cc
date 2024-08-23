#include "webcface/message/message.h"
#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <thread>
#include "dummy_server.h"
#include <crow.h>
#include "webcface/internal/unix_path.h"
#ifdef _WIN32
#include <fileapi.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#endif

using namespace webcface;

// 同じ実装がserver-internalにあるがimportもincludeもできないのでコピペしている
static inline spdlog::level::level_enum convertLevel(int level) {
    switch (level) {
    case 4:
        return spdlog::level::critical;
    case 3:
        return spdlog::level::err;
    case 2:
        return spdlog::level::warn;
    case 1:
        return spdlog::level::info;
    case 0:
    default:
        return spdlog::level::debug;
    }
}
using LoggerCallback =
    std::function<void(const char *, unsigned long long, int)>;
class CustomLogger : public crow::ILogHandler {
    LoggerCallback callback;

  public:
    CustomLogger(const LoggerCallback &callback) : callback(callback) {}
    void log(std::string message, crow::LogLevel level) override {
        callback(message.data(), message.size(), static_cast<int>(level));
    }
};

DummyServer::~DummyServer() {
    try {
        static_cast<crow::SimpleApp *>(server_)->stop();
        t.join();
    } catch (...) {
        // ignore exception
    }
}
DummyServer::DummyServer(bool use_unix)
    : t([this, use_unix] {
          static int sn = 0;
          dummy_logger =
              spdlog::stdout_color_mt("dummy_server_" + std::to_string(sn++));
          dummy_logger->set_level(spdlog::level::trace);

          static auto crow_logger =
              spdlog::stdout_color_mt("crow_server_" + std::to_string(sn++));
          crow_logger->set_level(spdlog::level::trace);
          static auto crow_logger_callback =
              [](const char *data, unsigned long long size, int level) {
                  crow_logger->log(
                      convertLevel(level),
                      std::string(data, static_cast<std::size_t>(size)));
              };
          static CustomLogger crow_custom_logger(crow_logger_callback);
          crow::logger::setHandler(&crow_custom_logger);

          crow::SimpleApp server;
          server_ = static_cast<void *>(&server);


          CROW_WEBSOCKET_ROUTE((server), "/")
              // route.websocket<std::remove_reference<decltype(*app)>::type>(app.get())
              .onopen([&](crow::websocket::connection &conn) {
                  std::lock_guard lock(server_m);
                  connPtr = &conn;
                  dummy_logger->info("ws_open");
              })
              .onclose([&](crow::websocket::connection &, const std::string &) {
                  std::lock_guard lock(server_m);
                  connPtr = nullptr;
                  dummy_logger->info("ws_close");
              })
              .onmessage([&](crow::websocket::connection &,
                             const std::string &data, bool) {
                  std::lock_guard lock(server_m);
                  dummy_logger->info("ws_message");
                  auto unpacked = message::unpack(data, dummy_logger);
                  for (const auto &m : unpacked) {
                      dummy_logger->info("kind {}", m.first);
                      recv_data.push_back(m);
                  }
              });

          if (use_unix) {
              auto unix_path = internal::unixSocketPath(17530);
              internal::initUnixSocket(unix_path, dummy_logger);
              server.unix_path(unix_path.string()).run();
          } else {
              server.port(17530).run();
          }
          std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }) {}

void DummyServer::send(std::string msg) {
    std::lock_guard lock(server_m);
    if (connPtr) {
        dummy_logger->info("send {} bytes", msg.size());
        reinterpret_cast<crow::websocket::connection *>(connPtr)->send_binary(
            std::move(msg));
    }
}

bool DummyServer::connected() { return connPtr != nullptr; }
