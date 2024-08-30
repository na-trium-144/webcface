#include "webcface/message/message.h"
#include <vector>
#include <utility>
#include <thread>
#include "webcface/internal/queue.h"

using namespace webcface;
struct DummyClient {
    std::vector<std::pair<int, message::MessageVariant>> recv_data;

    // clientからT型のメッセージを受信していればf1, そうでなければf2を実行する
    template <typename T, typename F1, typename F2>
    void recv(const F1 &on_ok, const F2 &on_ng) {
        std::lock_guard lock(client_m);
        for (const auto &m : recv_data) {
            if (m.first == T::kind) {
                on_ok(std::get<T>(m.second));
                return;
            }
        }
        on_ng();
    }
    // clientからT型のメッセージを受信するまで待機
    template <typename T, typename F1>
    void waitRecv(const F1 &on_ok) {
        while (true) {
            {
                std::lock_guard lock(client_m);
                for (const auto &m : recv_data) {
                    if (m.first == T::kind) {
                        on_ok(std::get<T>(m.second));
                        return;
                    }
                }
            }
            std::this_thread::sleep_for(
                std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
        }
    }
    inline void recvClear() { recv_data.clear(); }

    void send(std::string msg);
    void send(const char *msg) { send(std::string(msg)); }

    // clientにメッセージを送信する
    template <typename T>
    void send(const T &msg) {
        send(message::packSingle(msg));
    }

    std::atomic<bool> closing = false;
    Queue<std::string> msg_queue;
    std::thread t;
    std::mutex client_m;
    bool connected_ = false;
    bool connected() {
        std::lock_guard lock(client_m);
        return connected_;
    }
    explicit DummyClient(bool use_unix = false);
    ~DummyClient();
};
