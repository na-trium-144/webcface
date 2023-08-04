#pragma once
#include <string>
#include <memory>
#include <future>
#include <vector>
#include <eventpp/callbacklist.h>
#include <eventpp/eventdispatcher.h>
#include "decl.h"
#include "data_store.h"
#include "data.h"
#include "func.h"
#include "data_event.h"
#include "member.h"

namespace WebCFace {

//! サーバーに接続するクライアント。
class Client {
  private:
    std::shared_ptr<drogon::WebSocketClient> ws;
    //! close()が呼ばれたらtrue
    bool closing = false;
    //! 接続が完了したかどうかを取得する
    std::future<void> connection_finished;
    //! 再接続
    //! 切れたら再帰的に呼ばれる(正確には別スレッドで呼び出されるので再帰ではない)
    void reconnect();

    //! サーバーのホスト
    std::string host;
    //! サーバーのポート
    int port;

    SyncDataStore<Value::DataType> value_store;
    SyncDataStore<Text::DataType> text_store;
    SyncDataStore<Func::DataType> func_store;
    FuncResultStore func_result_store;

    Member self_;
    std::string name_;

    //! 特定のValueが変更された時のイベント
    eventpp::EventDispatcher<SyncDataKey<Value::DataType>, void(Value)>
        value_change_event;
    //! 特定のTextが変更された時のイベント
    eventpp::EventDispatcher<SyncDataKey<Text::DataType>, void(Text)>
        text_change_event;

    //! Memberが追加された時のイベント
    eventpp::CallbackList<void(Member)> member_entry_event;
    //! Valueが追加された時のイベント
    eventpp::CallbackList<void(Value)> value_entry_event;
    //! Textが追加された時のイベント
    eventpp::CallbackList<void(Text)> text_entry_event;
    //! Funcが追加された時のイベント
    eventpp::CallbackList<void(Func)> func_entry_event;

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
    Member member(const std::string &name) { return Member(this, name); }
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
    MemberEvent<Member> membersChange() {
        return MemberEvent<Member>{&member_entry_event};
    }
};

} // namespace WebCFace
