#pragma once
#include <msgpack.hpp>
#include <string>
#include <utility>
#include <vector>
#include <any>
#include <cstdint>
#include <webcface/common/func.h>
#include <webcface/common/log.h>
#include <webcface/common/view.h>
#include "val_adaptor.h"

MSGPACK_ADD_ENUM(WebCFace::Common::ValType);
MSGPACK_ADD_ENUM(WebCFace::Common::ViewComponentType);
MSGPACK_ADD_ENUM(WebCFace::Common::ViewColor);

namespace WebCFace::Message {
// 新しいメッセージの定義は
// kind追記→struct作成→message.ccに追記→s_client_data.ccに追記→client.ccに追記
enum class MessageKind {
    unknown = -1,
    value = 0,
    text = 1,
    view = 3,
    entry = 50, // 50〜
    req = 100,  // 100〜
    // 150〜: other
    sync_init = 150,
    call = 151,
    call_response = 155,
    call_result = 152,
    func_info = 154,
    log = 156,
    log_req = 157,
    sync = 158,
};
inline constexpr MessageKind kind_req(MessageKind k) {
    return static_cast<MessageKind>(static_cast<int>(k) +
                                    static_cast<int>(MessageKind::req));
}
inline constexpr MessageKind kind_entry(MessageKind k) {
    return static_cast<MessageKind>(static_cast<int>(k) +
                                    static_cast<int>(MessageKind::entry));
}

namespace Common = WebCFace::Common;

//! 型からkindを取得するためだけのベースクラス
template <MessageKind k>
struct MessageBase {
    static constexpr MessageKind kind = k;
};
//! client->server->client 自身の名前を送る
struct SyncInit : public MessageBase<MessageKind::sync_init> {
    std::string member;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("m", member));
};
//! client->server->client syncの時刻
//! 各sync()ごとに1回、他のメッセージより先に現在時刻を送る
struct Sync : public MessageBase<MessageKind::sync> {
    std::string member;
    std::uint64_t time;
    Sync()
        : time(std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::system_clock::now().time_since_epoch())
                   .count()) {}
    std::chrono::system_clock::time_point getTime() const {
        return std::chrono::system_clock::time_point(
            std::chrono::milliseconds(time));
    }
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("m", member), MSGPACK_NVP("t", time));
};

//! client(caller)->server->client(receiver) 関数呼び出し
//! client->server時はcallerは無視
struct Call : public MessageBase<MessageKind::call>, public Common::FuncCall {
    Call() = default;
    Call(const Common::FuncCall &c)
        : MessageBase<MessageKind::call>(), Common::FuncCall(c) {}
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("i", caller_id), MSGPACK_NVP("c", caller),
                       MSGPACK_NVP("r", member), MSGPACK_NVP("f", field),
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
    Common::ValAdaptor result;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("i", caller_id), MSGPACK_NVP("c", caller),
                       MSGPACK_NVP("e", is_error), MSGPACK_NVP("r", result));
};
//! client(member)->server->client Valueを更新
//! client->server時はmemberは無視
struct Value : public MessageBase<MessageKind::value> {
    std::string member, field;
    double data;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("m", member), MSGPACK_NVP("f", field),
                       MSGPACK_NVP("d", data));
};
//! client(member)->server->client Textを更新
//! client->server時はmemberは無視
struct Text : public MessageBase<MessageKind::text> {
    std::string member, field;
    std::string data;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("m", member), MSGPACK_NVP("f", field),
                       MSGPACK_NVP("d", data));
};
struct View : public MessageBase<MessageKind::view> {
    std::string member, field;
    struct ViewComponent {
        Common::ViewComponentType type;
        std::string text;
        std::optional<std::string> on_click_member, on_click_field;
        Common::ViewColor text_color, bg_color;
        ViewComponent() = default;
        ViewComponent(const Common::ViewComponentBase &vc)
            : type(vc.type_), text(vc.text_), text_color(vc.text_color_),
              bg_color(vc.bg_color_) {
            if (vc.on_click_func_ != std::nullopt) {
                on_click_member = vc.on_click_func_->member_;
                on_click_field = vc.on_click_func_->field_;
            }
        }
        operator Common::ViewComponentBase() const {
            Common::ViewComponentBase vc;
            vc.type_ = type;
            vc.text_ = text;
            if (on_click_member != std::nullopt) {
                vc.on_click_func_ =
                    Common::FieldBase{*on_click_member, *on_click_field};
            }
            vc.text_color_ = text_color;
            vc.bg_color_ = bg_color;
            return vc;
        }
        MSGPACK_DEFINE_MAP(MSGPACK_NVP("t", type), MSGPACK_NVP("x", text),
                           MSGPACK_NVP("L", on_click_member),
                           MSGPACK_NVP("l", on_click_field),
                           MSGPACK_NVP("c", text_color),
                           MSGPACK_NVP("b", bg_color));
    };
    std::unordered_map<int, ViewComponent> data_diff;
    int length;
    View() = default;
    View(const std::string &member, const std::string &field,
         const std::unordered_map<int, Common::ViewComponentBase> &data_diff,
         int length)
        : member(member), field(field), length(length) {
        for (const auto &vc : data_diff) {
            this->data_diff[vc.first] = vc.second;
        }
    }
    View(const std::string &member, const std::string &field,
         const std::unordered_map<int, ViewComponent> &data_diff, int length)
        : member(member), field(field), data_diff(data_diff), length(length) {}
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("m", member), MSGPACK_NVP("f", field),
                       MSGPACK_NVP("d", data_diff), MSGPACK_NVP("l", length));
};
//! client(member)->server->client logを追加
//! client->server時はmemberは無視
struct Log : public MessageBase<MessageKind::log> {
    std::string member;
    struct LogLine {
        int level;
        //! 1970/1/1からの経過ミリ秒
        std::uint64_t time;
        std::string message;
        LogLine() = default;
        LogLine(const Common::LogLine &l)
            : level(l.level),
              time(std::chrono::duration_cast<std::chrono::milliseconds>(
                       l.time.time_since_epoch())
                       .count()),
              message(l.message) {}
        operator Common::LogLine() const {
            return {level,
                    std::chrono::system_clock::time_point(
                        std::chrono::milliseconds(time)),
                    message};
        }
        MSGPACK_DEFINE_MAP(MSGPACK_NVP("v", level), MSGPACK_NVP("t", time),
                           MSGPACK_NVP("m", message));
    };
    std::vector<LogLine> log;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("m", member), MSGPACK_NVP("l", log));
};
//! Logのリクエストはメンバ名のみ
struct LogReq : public MessageBase<MessageKind::log_req> {
    std::string member;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("m", member));
};
//! client(member)->server->client func登録
//! client->server時はmemberは無視
struct FuncInfo : public MessageBase<MessageKind::func_info> {
    std::string member, field;
    Common::ValType return_type;
    struct Arg : public Common::Arg {
        Arg() = default;
        Arg(const Common::Arg &a) : Common::Arg(a) {}
        MSGPACK_DEFINE_MAP(MSGPACK_NVP("n", name_), MSGPACK_NVP("t", type_),
                           MSGPACK_NVP("i", init_), MSGPACK_NVP("m", min_),
                           MSGPACK_NVP("x", max_), MSGPACK_NVP("o", option_));
    };
    std::vector<Arg> args;
    FuncInfo() = default;
    explicit FuncInfo(const std::string &member, const std::string &field,
                      const Common::FuncInfo &info)
        : MessageBase<MessageKind::func_info>(), member(member), field(field),
          return_type(info.return_type) {
        args.resize(info.args.size());
        for (std::size_t i = 0; i < info.args.size(); i++) {
            args[i] = info.args[i];
        }
    }
    operator Common::FuncInfo() const {
        Common::FuncInfo info;
        info.return_type = return_type;
        info.args.resize(args.size());
        for (std::size_t j = 0; j < args.size(); j++) {
            info.args[j] = args[j];
        }
        return info;
    }
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("m", member), MSGPACK_NVP("f", field),
                       MSGPACK_NVP("r", return_type), MSGPACK_NVP("a", args));
};
//! client->server 以降Recvを送るようリクエスト
//! todo: 解除できるようにする
template <typename T>
struct Req : public MessageBase<kind_req(T::kind)> {
    std::string member, field;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("m", member), MSGPACK_NVP("f", field));
};
//! server->client 新しいvalueなどの報告
//! Funcの場合はこれではなくFuncInfoを使用
template <typename T>
struct Entry : public MessageBase<kind_entry(T::kind)> {
    std::string member, field;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("m", member), MSGPACK_NVP("f", field));
};

//! msgpackのメッセージをパースしstd::anyで返す
std::vector<std::pair<MessageKind, std::any>>
unpack(const std::string &message);

//! メッセージ1つを要素数2の配列としてシリアル化
template <typename T>
std::string packSingle(const T &obj) {
    msgpack::type::tuple<int, T> src(static_cast<int>(T::kind), obj);
    std::stringstream buffer;
    msgpack::pack(buffer, src);
    return buffer.str();
}

//! メッセージをシリアル化しbufferに追加
template <typename T>
void pack(std::stringstream &buffer, int &len, const T &obj) {
    msgpack::pack(buffer, static_cast<int>(T::kind));
    msgpack::pack(buffer, obj);
    len += 2;
}

inline std::string packDone(std::stringstream &buffer, int len) {
    std::stringstream buffer2;
    msgpack::packer packer(buffer2);
    packer.pack_array(len);
    buffer2 << buffer.rdbuf();
    return buffer2.str();
}

} // namespace WebCFace::Message
