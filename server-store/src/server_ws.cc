#include "webcface/server/server_ws.h"
#include <string>
#include <functional>
#include <memory>

WEBCFACE_NS_BEGIN
namespace server {
static std::string static_dir;
}
WEBCFACE_NS_END

#define CROW_STATIC_DIRECTORY webcface::server::static_dir
#define CROW_STATIC_ENDPOINT "/<path>"
#include <crow.h>

WEBCFACE_NS_BEGIN
namespace server {
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
}
void AppWrapper::setException(const char *what) noexcept {
    exception_str = what;
}

void AppWrapper::run() noexcept {
    try {
        static_cast<crow::SimpleApp *>(app)->run();
    } catch (const std::exception &e) {
        setException(e.what());
    }
}
const char *AppWrapper::exception() noexcept {
    if (!exception_str.empty()) {
        return exception_str.c_str();
    }
    return nullptr;
}
void AppWrapper::stop() noexcept {
    static_cast<crow::SimpleApp *>(app)->stop();
}
void AppWrapper::send(wsConnPtr conn, const char *msg,
                      std::size_t size) noexcept {
    static_cast<crow::websocket::connection *>(conn)->send_binary(
        std::string(msg, size));
}
void AppWrapper::close(wsConnPtr conn) noexcept {
    static_cast<crow::websocket::connection *>(conn)->close();
}

AppWrapper::AppWrapper(const LoggerCallback &callback, const char *static_dir_s,
                       std::uint16_t port, const char *unix_path,
                       const OpenCallback &on_open,
                       const CloseCallback &on_close,
                       const MessageCallback &on_message) noexcept {
    try {
        crow_custom_logger = std::make_unique<CustomLogger>(callback);
        crow::logger::setHandler(crow_custom_logger.get());

        webcface::server::static_dir = static_dir_s;

        crow::SimpleApp *crow_app = new crow::SimpleApp();
        this->app = crow_app;
        if (unix_path == nullptr) {
            crow_app->port(port);
        } else {
            crow_app->local_socket_path(unix_path);
        }
        crow_app->loglevel(crow::LogLevel::Warning);

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
        CROW_WEBSOCKET_ROUTE((*crow_app), "/")
            .onopen([on_open](crow::websocket::connection &conn) {
                on_open(&conn, conn.get_remote_ip().c_str());
            })
            .onclose([on_close](crow::websocket::connection &conn,
                                const std::string &reason,
                                std::uint16_t status) {
                on_close(&conn, reason.c_str(), status);
            })
            .onmessage([on_message](crow::websocket::connection &conn,
                                    const std::string &data,
                                    bool /*is_binary*/) {
                on_message(&conn, data.data(), data.size());
            });

    } catch (const std::exception &e) {
        setException(e.what());
    }
}
} // namespace server
WEBCFACE_NS_END
