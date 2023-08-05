#pragma once
#include <msgpack.hpp>
#include <string>
#include <utility>
#include <vector>
#include <any>
#include <webcface/func.h>
#include "arg_adaptor.h"

namespace WebCFace::Message {
// 新しいメッセージの定義は
// kind追記→struct作成→message.ccに追記→s_client_data.ccに追記→client.ccに追記
enum class MessageKind {
    unknown = -1,
    value = 0,
    text = 1,
    // recv = 50,       // 50〜
    subscribe = 100, // 100〜
    entry = 50,      // 50〜
    // 150〜: other
    name = 150,
    call = 151,
    call_response = 155,
    call_result = 152,
    // entry = 153,
    func_info = 154,
};
inline constexpr MessageKind kind_subscribe(MessageKind k) {
    return static_cast<MessageKind>(static_cast<int>(k) +
                                    static_cast<int>(MessageKind::subscribe));
}
// inline constexpr MessageKind kind_recv(MessageKind k) {
//     return static_cast<MessageKind>(static_cast<int>(k) +
//                                     static_cast<int>(MessageKind::recv));
// }
inline constexpr MessageKind kind_entry(MessageKind k) {
    return static_cast<MessageKind>(static_cast<int>(k) +
                                    static_cast<int>(MessageKind::entry));
}

//! 型からkindを取得するためだけのベースクラス
template <MessageKind k>
struct MessageBase {
    static constexpr MessageKind kind = k;
};
//! client->server->client 自身の名前を送る
struct Name : public MessageBase<MessageKind::name> {
    std::string name;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("n", name));
};
//! client(caller)->server->client(receiver) 関数呼び出し
//! client->server時はcallerは無視
struct Call : public MessageBase<MessageKind::call> {
    int caller_id;
    std::string caller, receiver, name;
    std::vector<WebCFace::ValAdaptor> args;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("i", caller_id), MSGPACK_NVP("c", caller),
                       MSGPACK_NVP("r", receiver), MSGPACK_NVP("n", name),
                       MSGPACK_NVP("a", args));
};
//! client(receiver)->server->client(caller) 関数の実行を開始したかどうか
struct CallResponse : public MessageBase<MessageKind::call_response> {
    int caller_id;
    std::string caller;
    bool started;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("i", caller_id), MSGPACK_NVP("c", caller),
                       MSGPACK_NVP("s", started));
};
//! client(receiver)->server->client(caller) 関数の戻り値
struct CallResult : public MessageBase<MessageKind::call_result> {
    int caller_id;
    std::string caller;
    bool is_error;
    std::string result;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("i", caller_id), MSGPACK_NVP("c", caller),
                       MSGPACK_NVP("e", is_error), MSGPACK_NVP("r", result));
};
//! client(member)->server->client Valueを更新
//! client->server時はmemberは無視
struct Value : public MessageBase<MessageKind::value> {
    std::string member, name;
    double data;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("m", member), MSGPACK_NVP("n", name),
                       MSGPACK_NVP("d", data));
};
//! client(member)->server->client Textを更新
//! client->server時はmemberは無視
struct Text : public MessageBase<MessageKind::text> {
    std::string member, name;
    std::string data;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("m", member), MSGPACK_NVP("n", name),
                       MSGPACK_NVP("d", data));
};
//! client(member)->server->client func登録
//! client->server時はmemberは無視
struct FuncInfo : public MessageBase<MessageKind::func_info> {
    std::string member, name;
    int return_type;
    std::vector<int> args_type;
    FuncInfo() = default;
    explicit FuncInfo(const std::string &member, const std::string &name,
                      const WebCFace::FuncInfo &info)
        : MessageBase<MessageKind::func_info>(), member(member), name(name),
          return_type(static_cast<int>(info.return_type)) {
        args_type.resize(info.args_type.size());
        for (std::size_t i = 0; i < info.args_type.size(); i++) {
            args_type[i] = static_cast<int>(info.args_type[i]);
        }
    }
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("m", member), MSGPACK_NVP("n", name),
                       MSGPACK_NVP("r", return_type),
                       MSGPACK_NVP("a", args_type));
};
//! client->server 以降Recvを送るようリクエスト
//! todo: 解除できるようにする
template <typename T>
struct Subscribe : public MessageBase<kind_subscribe(T::kind)> {
    std::string from, name;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("f", from), MSGPACK_NVP("n", name));
};
//! server->client 新しいvalueなどの報告
//! Funcの場合はこれではなくFuncInfoを使用
template <typename T>
struct Entry : public MessageBase<kind_entry(T::kind)> {
    std::string member, name;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("m", member), MSGPACK_NVP("n", name));
};

//! msgpackのメッセージをパースしstd::anyで返す
std::pair<MessageKind, std::any> unpack(const std::string &message);

//! メッセージをシリアル化
template <typename T>
std::vector<char> pack(T obj) {
    msgpack::type::tuple<int, T> src(static_cast<int>(T::kind), obj);
    std::stringstream buffer;
    msgpack::pack(buffer, src);
    buffer.seekg(0);
    std::string bs = buffer.str();
    return std::vector<char>(bs.begin(), bs.end());
}

} // namespace WebCFace::Message
