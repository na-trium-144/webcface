#pragma once
#include <msgpack.hpp>
#include <string>
#include <utility>
#include <vector>
#include <any>
#include <cstdint>
#include <spdlog/logger.h>
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
namespace MessageKind {
enum MessageKindEnum {
    unknown = -1,
    value = 0,
    text = 1,
    view = 3,
    entry = 20,
    req = 40,
    res = 60,
    sync_init = 80,
    call = 81,
    call_response = 82,
    call_result = 83,
    func_info = 84,
    log = 85,
    log_req = 86,
    sync = 87,
    svr_version = 88,
    ping = 89,
    ping_status = 90,
    ping_status_req = 91,
};
}

//! 型からkindを取得するためだけのベースクラス
template <int k>
struct MessageBase {
    static constexpr int kind = k;
};
//! client初期化(client->server->client)
/*! clientは接続後最初に1回、
 * member_name,lib_name,lib_verを送る
 *
 * member_nameが空文字列でない場合、同時に接続している他のクライアントと被ってはいけない
 * 過去に同名で接続したクライアントがある場合同じmember_idが振られる
 *
 * member_nameが空文字列の場合、他のクライアントとの被りは問題ないが、
 * 他のクライアントにはこのクライアントの存在が通知されず、
 * valueなどのデータを送ることはできない
 *
 * serverはmember_idを振り、
 * member_nameが空でなかった場合は他の全クライアントにmember_idとaddrを載せて通知する
 */
struct SyncInit : public MessageBase<MessageKind::sync_init> {
    std::string member_name; //!< member名
    unsigned int member_id;  //!< member id (1以上)
    //! clientライブラリの名前(id) このライブラリでは"cpp"
    /*! 新しくライブラリ作ることがあったら変えて識別できるようにすると良いかも
     */
    std::string lib_name;
    std::string lib_ver; //!< clientライブラリのバージョン
    std::string addr;    //!< clientのipアドレス
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("M", member_name),
                       MSGPACK_NVP("m", member_id), MSGPACK_NVP("l", lib_name),
                       MSGPACK_NVP("v", lib_ver), MSGPACK_NVP("a", addr));
};

//! serverのバージョン情報(server->client)
/*! serverはSyncInit受信後にこれを返す
 */
struct SvrVersion : public MessageBase<MessageKind::svr_version> {
    std::string svr_name; //!< serverの名前 このライブラリでは"webcface"
    std::string ver;      //!< serverのバージョン
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("n", svr_name), MSGPACK_NVP("v", ver));
};
//! ping(server->client->server)
/*! serverが一定間隔でこれをclientに送る
 *
 * 内容は空のmap
 *
 * clientは即座に送り返さなければならない
 * (送り返さなくても何も起きないが)
 */
struct Ping : public MessageBase<MessageKind::ping> {
    Ping() = default;
};
//! 各クライアントのping状況 (server->client)
struct PingStatus : public MessageBase<MessageKind::ping_status> {
    //! member_id: ping応答時間(ms) のmap
    std::shared_ptr<std::unordered_map<unsigned int, int>> status;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("s", status));
};
//! ping状況のリクエスト (client->server)
/*! これを送ると以降serverが一定間隔でPingStatusを送り返す
 */
struct PingStatusReq : public MessageBase<MessageKind::ping_status_req> {
    PingStatusReq() = default;
};
//! syncの時刻(client->server->client)
/*! clientは各sync()ごとに1回、他のメッセージより先に現在時刻を送る
 *
 * serverはそのclientのデータを1つ以上requestしているクライアントに対して
 * member_idを載せて送る
 */
struct Sync : public MessageBase<MessageKind::sync> {
    unsigned int member_id; //!< member id
    //! 1970/1/1 0:00(utc) からの経過ミリ秒数で表し、閏秒はカウントしない
    std::uint64_t time;
    Sync(unsigned int member_id,
         const std::chrono::system_clock::time_point &time)
        : member_id(member_id),
          time(std::chrono::duration_cast<std::chrono::milliseconds>(
                   time.time_since_epoch())
                   .count()) {}
    Sync() : Sync(0, std::chrono::system_clock::now()) {}
    std::chrono::system_clock::time_point getTime() const {
        return std::chrono::system_clock::time_point(
            std::chrono::milliseconds(time));
    }
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("m", member_id), MSGPACK_NVP("t", time));
};

//! 関数呼び出し (client(caller)->server->client(receiver))
/*! caller側clientが一意のcaller_idを振る(0以上の整数)
 *
 * serverはcaller_member_idをつけてreceiverに送る
 */
struct Call : public MessageBase<MessageKind::call>, public Common::FuncCall {
    Call() = default;
    Call(const Common::FuncCall &c)
        : MessageBase<MessageKind::call>(), Common::FuncCall(c) {}
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("i", caller_id),
                       MSGPACK_NVP("c", caller_member_id),
                       MSGPACK_NVP("r", target_member_id),
                       MSGPACK_NVP("f", field), MSGPACK_NVP("a", args));
};
//! 関数呼び出しの応答1 (client(receiver)->server->client(caller))
/*! clientはcalled_id,caller_member_idと、関数の実行を開始したかどうかを返す
 *
 * 関数の実行に時間がかかる場合も実行完了を待たずにstartedをtrueにして送る
 *
 * 対象の関数が存在しない場合、startedをfalseにして送る
 *
 * serverはそれをそのままcallerに送る
 */
struct CallResponse : public MessageBase<MessageKind::call_response> {
    std::size_t caller_id;
    unsigned int caller_member_id;
    bool started; //!< 関数の実行を開始したかどうか
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("i", caller_id),
                       MSGPACK_NVP("c", caller_member_id),
                       MSGPACK_NVP("s", started));
};
//! 関数呼び出しの応答2 (client(receiver)->server->client(caller))
/*! clientはcalled_id,caller_member_idと、関数の実行結果を返す
 *
 * resultに結果を文字列または数値または真偽値で返す
 * 結果が無い場合は "" を返す
 *
 * 例外が発生した場合はis_errorをtrueにしresultに例外の内容を文字列で入れて返す
 *
 * serverはそれをそのままcallerに送る
 */
struct CallResult : public MessageBase<MessageKind::call_result> {
    std::size_t caller_id;
    unsigned int caller_member_id;
    bool is_error;
    Common::ValAdaptor result;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("i", caller_id),
                       MSGPACK_NVP("c", caller_member_id),
                       MSGPACK_NVP("e", is_error), MSGPACK_NVP("r", result));
};
//! client(member)->server->client Valueを更新
struct Value : public MessageBase<MessageKind::value> {
    std::string field;
    std::shared_ptr<std::vector<double>> data;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("f", field), MSGPACK_NVP("d", data));
};
//! client(member)->server->client Textを更新
struct Text : public MessageBase<MessageKind::text> {
    std::string field;
    std::shared_ptr<std::string> data;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("f", field), MSGPACK_NVP("d", data));
};
struct View : public MessageBase<MessageKind::view> {
    std::string field;
    struct ViewComponent {
        Common::ViewComponentType type = Common::ViewComponentType::text;
        std::string text;
        std::optional<std::string> on_click_member, on_click_field;
        Common::ViewColor text_color = Common::ViewColor::inherit, bg_color = Common::ViewColor::inherit;
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
    std::shared_ptr<std::unordered_map<std::string, ViewComponent>> data_diff;
    std::size_t length;
    View() = default;
    View(const std::string &field,
         const std::shared_ptr<
             std::unordered_map<int, Common::ViewComponentBase>> &data_diff,
         std::size_t length)
        : field(field),
          data_diff(std::make_shared<
                    std::unordered_map<std::string, ViewComponent>>()),
          length(length) {
        for (const auto &vc : *data_diff) {
            this->data_diff->emplace(std::to_string(vc.first), vc.second);
        }
    }
    View(const std::string &field,
         const std::shared_ptr<std::unordered_map<std::string, ViewComponent>>
             &data_diff,
         std::size_t length)
        : field(field), data_diff(data_diff), length(length) {}
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("f", field), MSGPACK_NVP("d", data_diff),
                       MSGPACK_NVP("l", length));
};
//! client(member)->server->client logを追加
//! client->server時はmemberは無視
struct Log : public MessageBase<MessageKind::log> {
    unsigned int member_id;
    struct LogLine {
        int level = 0;
        //! 1970/1/1からの経過ミリ秒
        std::uint64_t time = 0;
        std::string message;
        LogLine() = default;
        LogLine(const Common::LogLine &l)            : level(l.level),
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
    std::shared_ptr<std::vector<LogLine>> log;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("m", member_id), MSGPACK_NVP("l", log));
};
//! Logのリクエストはメンバ名のみ
struct LogReq : public MessageBase<MessageKind::log_req> {
    std::string member;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("M", member));
};
//! client(member)->server->client func登録
//! client->server時はmemberは無視
struct FuncInfo : public MessageBase<MessageKind::func_info> {
    unsigned int member_id = 0;
    std::string field;
    Common::ValType return_type;
    struct Arg : public Common::Arg {
        Arg() = default;
        Arg(const Common::Arg &a) : Common::Arg(a) {}
        MSGPACK_DEFINE_MAP(MSGPACK_NVP("n", name_), MSGPACK_NVP("t", type_),
                           MSGPACK_NVP("i", init_), MSGPACK_NVP("m", min_),
                           MSGPACK_NVP("x", max_), MSGPACK_NVP("o", option_));
    };
    std::shared_ptr<std::vector<Arg>> args;
    FuncInfo() = default;
    FuncInfo(unsigned int member_id, const std::string &field,
             Common::ValType return_type,
             std::shared_ptr<std::vector<Arg>> args)
        : member_id(member_id), field(field), return_type(return_type),
          args(args) {}
    explicit FuncInfo(const std::string &field, const Common::FuncInfo &info)
        : MessageBase<MessageKind::func_info>(), field(field),
          args(std::make_shared<std::vector<Arg>>(info.args.size())),
          return_type(info.return_type) {
        for (std::size_t i = 0; i < info.args.size(); i++) {
            (*args)[i] = info.args[i];
        }
    }
    operator Common::FuncInfo() const {
        Common::FuncInfo info;
        info.return_type = return_type;
        info.args.resize(args->size());
        for (std::size_t j = 0; j < args->size(); j++) {
            info.args[j] = (*args)[j];
        }
        return info;
    }
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("m", member_id), MSGPACK_NVP("f", field),
                       MSGPACK_NVP("r", return_type), MSGPACK_NVP("a", args));
};
//! client->server 以降Recvを送るようリクエスト
//! todo: 解除できるようにする
template <typename T>
struct Req : public MessageBase<T::kind + MessageKind::req> {
    std::string member;
    std::string field;
    //! 1以上
    unsigned int req_id;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("i", req_id), MSGPACK_NVP("M", member),
                       MSGPACK_NVP("f", field));
};
//! server->client 新しいvalueなどの報告
//! Funcの場合はこれではなくFuncInfoを使用
template <typename T>
struct Entry : public MessageBase<T::kind + MessageKind::entry> {
    unsigned int member_id;
    std::string field;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("m", member_id), MSGPACK_NVP("f", field));
};
template <typename T>
struct Res {};
//! server->client  Value,Textなどのfieldをreqidに変えただけのもの
//! requestしたフィールドの子フィールドの場合sub_fieldにフィールド名を入れて返す
//! →その場合clientが再度requestを送るはず
template <>
struct Res<Value> : public MessageBase<MessageKind::value + MessageKind::res> {
    unsigned int req_id;
    std::string sub_field;
    std::shared_ptr<std::vector<double>> data;
    Res() = default;
    Res(unsigned int req_id, const std::string &sub_field,
        const std::shared_ptr<std::vector<double>> &data)
        : req_id(req_id), sub_field(sub_field), data(data) {}
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("i", req_id), MSGPACK_NVP("f", sub_field),
                       MSGPACK_NVP("d", data));
};
template <>
struct Res<Text> : public MessageBase<MessageKind::text + MessageKind::res> {
    unsigned int req_id;
    std::string sub_field;
    std::shared_ptr<std::string> data;
    Res() = default;
    Res(unsigned int req_id, const std::string &sub_field,
        const std::shared_ptr<std::string> &data)
        : req_id(req_id), sub_field(sub_field), data(data) {}
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("i", req_id), MSGPACK_NVP("f", sub_field),
                       MSGPACK_NVP("d", data));
};
template <>
struct Res<View> : public MessageBase<MessageKind::view + MessageKind::res> {
    unsigned int req_id;
    std::string sub_field;
    std::shared_ptr<std::unordered_map<std::string, View::ViewComponent>>
        data_diff;
    std::size_t length;
    Res() = default;
    Res(unsigned int req_id, const std::string &sub_field,
        const std::shared_ptr<
            std::unordered_map<std::string, View::ViewComponent>> &data_diff,
        std::size_t length)
        : req_id(req_id), sub_field(sub_field), data_diff(data_diff),
          length(length) {}
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("i", req_id), MSGPACK_NVP("f", sub_field),
                       MSGPACK_NVP("d", data_diff), MSGPACK_NVP("l", length));
};

//! msgpackのメッセージをパースしstd::anyで返す
std::vector<std::pair<int, std::any>>
unpack(const std::string &message,
       const std::shared_ptr<spdlog::logger> &logger);

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

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    namespace adaptor {
    template <typename T>
    struct EmptyConvert {
        msgpack::object const &operator()(msgpack::object const &o, T &) const {
            return o;
        }
    };
    template <typename T>
    struct EmptyPack {
        template <typename Stream>
        msgpack::packer<Stream> &operator()(msgpack::packer<Stream> &o,
                                            const T &) {
            o.pack_map(0);
            return o;
        }
    };
    template <>
    struct convert<WebCFace::Message::Ping>
        : public EmptyConvert<WebCFace::Message::Ping> {};
    template <>
    struct convert<WebCFace::Message::PingStatusReq>
        : public EmptyConvert<WebCFace::Message::PingStatusReq> {};
    template <>
    struct pack<WebCFace::Message::Ping>
        : public EmptyPack<WebCFace::Message::Ping> {};
    template <>
    struct pack<WebCFace::Message::PingStatusReq>
        : public EmptyPack<WebCFace::Message::PingStatusReq> {};
    } // namespace adaptor
}
} // namespace msgpack
