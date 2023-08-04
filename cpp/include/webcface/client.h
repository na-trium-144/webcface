#pragma once
#include <string>
#include <future>
#include <vector>

namespace drogon {
class WebSocketClient;
}

namespace WebCFace {

class Client;

//! 他のクライアントを参照することを表すクラス
/*! 参照元のClientが破棄されているとセグフォします
 */
class Member {
    Client *cli = nullptr;
    std::string name_;

  public:
    Member() = default;
    Member(Client *cli, const std::string &name);

    std::string name() const { return name_; }

    //! このmemberの指定した名前のvalueを参照する。
    Value value(const std::string &name) const {
        return Value{cli, this->name(), name};
    }
    //! このmemberの指定した名前のtextを参照する。
    Text text(const std::string &name) const {
        return Text{cli, this->name(), name};
    }
    //! このmemberの指定した名前のfuncを参照する。
    Func func(const std::string &name) const {
        return Func{cli, this->name(), name};
    }
    //! このmemberが公開しているvalueのリストを返す。
    std::vector<Value> values() const;
    //! このmemberが公開しているtextのリストを返す。
    std::vector<Text> texts() const;
    //! このmemberが公開しているfuncのリストを返す。
    std::vector<Func> funcs() const;
};

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

    //! 受信時の処理
    void onRecv(const std::string &message);
    //! データを送信する
    void send(const std::vector<char> &m);
    //! 接続を切り、今後再接続しない
    void close();

  public:
    template <typename T>
    friend class SyncData;
    friend Func;
    friend Member;

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
};

} // namespace WebCFace
