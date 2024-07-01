#include "webcface/server/internal/server_ws.h"
#include <string>
#include <functional>

namespace webcface {
namespace server_internal {
static std::string static_dir;
}
} // namespace webcface

#define CROW_STATIC_DIRECTORY webcface::server_internal::static_dir
#define CROW_STATIC_ENDPOINT "/<path>"
#include <crow.h>

namespace webcface {
namespace server_internal {

class CustomLogger : public crow::ILogHandler {
    LoggerCallback callback;

  public:
    CustomLogger(const LoggerCallback &callback) : callback(callback) {}
    void log(std::string message, crow::LogLevel level) override {
        callback(message.data(), message.size(), static_cast<int>(level));
    }
};

static std::unique_ptr<CustomLogger> crow_custom_logger;

AppWrapper::~AppWrapper() noexcept {
    delete static_cast<crow::SimpleApp *>(app);
    if (exception_str) {
        delete static_cast<std::string *>(exception_str);
    }
}
void AppWrapper::setException(const char *what) noexcept {
    try {
        exception_str = new std::string(what);
    } catch (...) {
    }
}

void AppWrapper::run() noexcept {
    try {
        static_cast<crow::SimpleApp *>(app)->run();
    } catch (const std::exception &e) {
        setException(e.what());
    }
}
const char *AppWrapper::exception() noexcept {
    if (exception_str) {
        return static_cast<std::string *>(exception_str)->c_str();
    }
    return nullptr;
}
void AppWrapper::stop() noexcept {
    static_cast<crow::SimpleApp *>(app)->stop();
}
void AppWrapper::send(wsConnPtr conn, const char *msg,
                      unsigned long long size) noexcept {
    static_cast<crow::websocket::connection *>(conn)->send_binary(
        std::string(msg, size));
}

AppWrapper::AppWrapper(const LoggerCallback &callback, const char *static_dir,
                       int port, const char *unix_path,
                       const OpenCallback &on_open,
                       const CloseCallback &on_close,
                       const MessageCallback &on_message,
                       const StartCallback &on_start) noexcept {
    try {
        crow_custom_logger = std::make_unique<CustomLogger>(callback);
        crow::logger::setHandler(crow_custom_logger.get());

        webcface::server_internal::static_dir = static_dir;

        crow::SimpleApp *app = new crow::SimpleApp();
        this->app = app;
        if (unix_path == nullptr) {
            app->port(port);
        } else {
            app->unix_path(unix_path);
        }
        app->loglevel(crow::LogLevel::Warning);

        /*
        / にアクセスしたときindex.htmlへリダイレクトさせようとしたが、
        windowsでなんかうまくいかなかったので諦めた

        auto &route = CROW_ROUTE((*app), "/");
        route([](crow::response &res) {
            res.redirect("index.html");
            res.end();
        });
        route.websocket<std::remove_reference<decltype(*app)>::type>(app.get())
        */
        CROW_WEBSOCKET_ROUTE((*app), "/")
            .onopen([on_open](crow::websocket::connection &conn) {
                on_open(&conn, conn.get_remote_ip().c_str());
            })
            .onclose([on_close](crow::websocket::connection &conn,
                                const std::string &reason) {
                on_close(&conn, reason.c_str());
            })
            .onmessage([on_message](crow::websocket::connection &conn,
                                    const std::string &data,
                                    bool /*is_binary*/) {
                on_message(&conn, data.data(), data.size());
            });

        std::thread([app, on_start] {
            app->wait_for_server_start();
            on_start();
        }).detach();
    } catch (const std::exception &e) {
        setException(e.what());
    }
}
} // namespace server_internal
} // namespace webcface
