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
    recv = 50,       // 50〜
    subscribe = 100, // 100〜
    // 150〜: other
    name = 150,
    call = 151,
    call_response = 152,
    entry = 153,
    func_info = 154,
};
inline constexpr MessageKind kind_subscribe(MessageKind k) {
    return static_cast<MessageKind>(static_cast<int>(k) +
                                    static_cast<int>(MessageKind::subscribe));
}
inline constexpr MessageKind kind_recv(MessageKind k) {
    return static_cast<MessageKind>(static_cast<int>(k) +
                                    static_cast<int>(MessageKind::recv));
}

template <MessageKind k>
struct MessageBase {
    static constexpr MessageKind kind = k;
};
struct Name : public MessageBase<MessageKind::name> {
    std::string name;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("n", name));
};
struct Call : public MessageBase<MessageKind::call> {
    int caller_id;
    std::string caller, receiver, name;
    std::vector<WebCFace::ValAdaptor> args;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("i", caller_id), MSGPACK_NVP("c", caller),
                       MSGPACK_NVP("r", receiver), MSGPACK_NVP("n", name),
                       MSGPACK_NVP("a", args));
};
struct CallResponse : public MessageBase<MessageKind::call_response> {
    int caller_id;
    std::string caller;
    bool found;
    bool is_error;
    std::string response;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("i", caller_id), MSGPACK_NVP("c", caller),
                       MSGPACK_NVP("f", found), MSGPACK_NVP("e", is_error),
                       MSGPACK_NVP("r", response));
};
struct Value : public MessageBase<MessageKind::value> {
    std::string name;
    using DataType = double;
    DataType data;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("n", name), MSGPACK_NVP("d", data));
};
struct Text : public MessageBase<MessageKind::text> {
    std::string name;
    using DataType = std::string;
    DataType data;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("n", name), MSGPACK_NVP("d", data));
};
struct FuncInfo : public MessageBase<MessageKind::func_info> {
    std::string name;
    int return_type;
    std::vector<int> args_type;
    FuncInfo() = default;
    explicit FuncInfo(const std::string &name, const WebCFace::FuncInfo &info)
        : MessageBase<MessageKind::func_info>(), name(name),
          return_type(static_cast<int>(info.return_type)) {
        args_type.resize(info.args_type.size());
        for (std::size_t i = 0; i < info.args_type.size(); i++) {
            args_type[i] = static_cast<int>(info.args_type[i]);
        }
    }
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("n", name), MSGPACK_NVP("r", return_type),
                       MSGPACK_NVP("a", args_type));
};
template <typename T>
struct Recv : public MessageBase<kind_recv(T::kind)> {
    std::string from, name;
    typename T::DataType data;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("f", from), MSGPACK_NVP("n", name),
                       MSGPACK_NVP("d", data));
};
template <typename T>
struct Subscribe : public MessageBase<kind_subscribe(T::kind)> {
    std::string from, name;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("f", from), MSGPACK_NVP("n", name));
};
struct Entry : public MessageBase<MessageKind::entry> {
    std::string name;
    struct EValue {
        std::string name;
        MSGPACK_DEFINE_MAP(MSGPACK_NVP("n", name));
    };
    // ほんとはsetにするべきだけどめんどくさいにゃー
    std::vector<EValue> value;
    using EText = EValue;
    std::vector<EText> text;
    std::vector<FuncInfo> func_info;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("f", name), MSGPACK_NVP("v", value),
                       MSGPACK_NVP("t", text), MSGPACK_NVP("u", func_info));
};

std::pair<MessageKind, std::any> unpack(const std::string &message);

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
