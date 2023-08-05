#pragma once
#include <string>
#include <memory>
#include <future>
#include <vector>
#include <thread>
#include <atomic>
#include <eventpp/eventqueue.h>
#include "data.h"
#include "func_result.h"
#include "func.h"
#include "member.h"

namespace WebCFace {

//! サーバーに接続するクライアント。
class Client {
  private:
    std::shared_ptr<ClientData> data;

    std::shared_ptr<drogon::WebSocketClient> ws;
    //! close()が呼ばれたらtrue
    std::atomic<bool> closing = false;
    //! 接続が完了したかどうかを取得する
    std::future<void> connection_finished;
    //! 再接続
    //! 切れたら再帰的に呼ばれる(正確には別スレッドで呼び出されるので再帰ではない)
    void reconnect();

    //! サーバーのホスト
    std::string host;
    //! サーバーのポート
    int port;

    
    Member self_;
    std::string name_;

    //! イベントを処理するスレッド
    std::thread event_thread;

    //! func_call_queueを処理するスレッド
    std::thread func_call_thread;

    //! 受信時の処理
    void onRecv(const std::string &message);
    //! データを送信する
    void send(const std::vector<char> &m);
    //! 接続を切り、今後再接続しない
    void close();

  public:
    template <typename T>
    friend class SyncData;
    template <typename T, typename V>
    friend class SyncDataWithEvent;
    friend Value;
    friend Text;
    friend Func;
    friend Member;
    template <typename V>
    friend class MemberEvent;

    Client() : Client("") {}
    Client(const Client &) = delete;
    const Client &operator=(const Client &) = delete;
    //! 自分自身のmemberとしての名前を指定しサーバーに接続する
    //! サーバーのホストとポートを省略した場合localhost:80になる
    explicit Client(const std::string &name,
                    const std::string &host = "127.0.0.1", int port = 80);
    //! サーバーに接続できているときtrueを返す。
    bool connected() const;
    //! デストラクタで接続を切る。
    ~Client();

    //! データをまとめて送信する。
    /*! value,textにセットしたデータをすべて送る。
     * 他クライアントのvalue,textを参照する場合、そのリクエストを送るのもsend()で行う。
     * clientを使用する時は必ずsendを適当なタイミングで繰り返し呼ぶこと。
     */
    void send();

    //! 自分自身を表すMember
    const Member &self() const { return self_; }
    //! 自分自身の名前
    /*! self().name() は "" になるので注意
     */
    std::string name() const { return name_; }

    //! 他のmemberにアクセスする。
    Member member(const std::string &name) { return Member(data, name); }
    //! サーバーに接続されている他のmemberのリストを得る。
    std::vector<Member> members() {
        auto keys = value_store.getMembers();
        std::vector<Member> ret(keys.size());
        for (std::size_t i = 0; i < keys.size(); i++) {
            ret[i] = member(keys[i]);
        }
        return ret;
    }
    //! Memberが追加された時のイベントリスト
    EventTarget<Member> membersChange() {
        return EventTarget<Member>{EventType::member_entry, this,
                                   &this->event_queue};
    }
};

} // namespace WebCFace
