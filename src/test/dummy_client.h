#include "../message/message.h"
#include <any>
#include <vector>
#include <utility>
#include <thread>
#include <webcface/common/queue.h>

using namespace webcface;
struct DummyClient {
    std::vector<std::pair<int, std::any>> recv_data;

    // clientからT型のメッセージを受信していればf1, そうでなければf2を実行する
    template <typename T, typename F1, typename F2>
    void recv(const F1 &on_ok, const F2 &on_ng) {
        for (const auto &m : recv_data) {
            if (m.first == T::kind) {
                on_ok(std::any_cast<T>(m.second));
                return;
            }
        }
        on_ng();
    }
    inline void recvClear() { recv_data.clear(); }

    void send(std::string msg);
    void send(const char *msg) { send(std::string(msg)); }

    // clientにメッセージを送信する
    template <typename T>
    void send(const T &msg) {
        send(Message::packSingle(msg));
    }

    std::atomic<bool> closing = false;
    Common::Queue<std::string> msg_queue;
    std::thread t;
    explicit DummyClient(bool unix = false);
    ~DummyClient();
};
