#pragma once
#include <msgpack.hpp>
#include <string>
#include <utility>
#include <vector>
#include <deque>
#include <any>
#include <cstdint>
#include <spdlog/logger.h>
#include <webcface/common/func.h>
#include <webcface/common/log.h>
#include <webcface/common/view.h>
#include <webcface/common/image.h>
#include <webcface/common/robot_model.h>
#include <webcface/common/canvas3d.h>
#include <webcface/common/canvas2d.h>
#include <webcface/common/def.h>
#include "val_adaptor.h"

MSGPACK_ADD_ENUM(WEBCFACE_NS::Common::ValType)
MSGPACK_ADD_ENUM(WEBCFACE_NS::Common::ViewComponentType)
MSGPACK_ADD_ENUM(WEBCFACE_NS::Common::ViewColor)
MSGPACK_ADD_ENUM(WEBCFACE_NS::Common::ImageCompressMode)
MSGPACK_ADD_ENUM(WEBCFACE_NS::Common::ImageColorMode)
MSGPACK_ADD_ENUM(WEBCFACE_NS::Common::RobotJointType)
MSGPACK_ADD_ENUM(WEBCFACE_NS::Common::GeometryType)
MSGPACK_ADD_ENUM(WEBCFACE_NS::Common::Canvas3DComponentType)
MSGPACK_ADD_ENUM(WEBCFACE_NS::Common::Canvas2DComponentType)

namespace WEBCFACE_NS::Message {
// 新しいメッセージの定義は
// kind追記→struct作成→message.ccに追記→s_client_data.ccに追記→client.ccに追記

namespace MessageKind {
enum MessageKindEnum {
    unknown = -1,
    value = 0,
    text = 1,
    binary = 2,
    view = 3,
    canvas2d = 4,
    image = 5,
    robot_model = 6,
    canvas3d = 7,
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

/*!
 * \brief 型からkindを取得するためだけのベースクラス
 *
 */
template <int k>
struct MessageBase {
    static constexpr int kind = k;
};
/*!
 * \brief client初期化(client->server->client)
 *
 * clientは接続後最初に1回、
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
    /*!
     * \brief member名
     *
     */
    std::string member_name;
    /*!
     * \brief member id (1以上)
     *
     */
    unsigned int member_id;
    /*!
     * \brief clientライブラリの名前(id) このライブラリでは"cpp"
     *
     * 新しくライブラリ作ることがあったら変えて識別できるようにすると良いかも
     *
     */
    std::string lib_name;
    std::string lib_ver;
    std::string addr;

    MSGPACK_DEFINE_MAP(MSGPACK_NVP("M", member_name),
                       MSGPACK_NVP("m", member_id), MSGPACK_NVP("l", lib_name),
                       MSGPACK_NVP("v", lib_ver), MSGPACK_NVP("a", addr))
};

/*!
 * \brief serverのバージョン情報(server->client)
 *
 * serverはSyncInit受信後にこれを返す
 *
 */
struct SvrVersion : public MessageBase<MessageKind::svr_version> {
    /*!
     * \brief serverの名前
     *
     * このライブラリでは "webcface"
     *
     */
    std::string svr_name;
    /*!
     * \brief serverのバージョン
     *
     */
    std::string ver;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("n", svr_name), MSGPACK_NVP("v", ver))
};
/*!
 * \brief ping(server->client->server)
 *
 * serverが一定間隔でこれをclientに送る
 *
 * 内容は空のmap
 *
 * clientは即座に送り返さなければならない
 * (送り返さなくても何も起きないが)
 *
 */
struct Ping : public MessageBase<MessageKind::ping> {
    Ping() = default;
};
/*!
 * \brief 各クライアントのping状況 (server->client)
 *
 */
struct PingStatus : public MessageBase<MessageKind::ping_status> {
    /*!
     * \brief member_id: ping応答時間(ms) のmap
     *
     */
    std::shared_ptr<std::unordered_map<unsigned int, int>> status;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("s", status))
};
/*!
 * \brief ping状況のリクエスト (client->server)
 *
 * これを送ると以降serverが一定間隔でPingStatusを送り返す
 *
 */
struct PingStatusReq : public MessageBase<MessageKind::ping_status_req> {
    PingStatusReq() = default;
};
/*!
 * \brief syncの時刻(client->server->client)
 *
 * clientは各sync()ごとに1回、他のメッセージより先に現在時刻を送る
 *
 * serverはそのclientのデータを1つ以上requestしているクライアントに対して
 * member_idを載せて送る
 *
 */
struct Sync : public MessageBase<MessageKind::sync> {
    unsigned int member_id; //!< member id
    /*!
     * \brief 1970/1/1 0:00(utc) からの経過ミリ秒数で表し、閏秒はカウントしない
     *
     */
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
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("m", member_id), MSGPACK_NVP("t", time))
};

/*!
 * \brief 関数呼び出し (client(caller)->server->client(receiver))
 *
 * caller側clientが一意のcaller_idを振る(0以上の整数)
 *
 * serverはcaller_member_idをつけてreceiverに送る
 *
 */
struct Call : public MessageBase<MessageKind::call>, public Common::FuncCall {
    Call() = default;
    Call(const Common::FuncCall &c)
        : MessageBase<MessageKind::call>(), Common::FuncCall(c) {}
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("i", caller_id),
                       MSGPACK_NVP("c", caller_member_id),
                       MSGPACK_NVP("r", target_member_id),
                       MSGPACK_NVP("f", field), MSGPACK_NVP("a", args))
};
/*!
 * \brief 関数呼び出しの応答1 (client(receiver)->server->client(caller))
 *
 * clientはcalled_id,caller_member_idと、関数の実行を開始したかどうかを返す
 *
 * * 関数の実行に時間がかかる場合も実行完了を待たずにstartedをtrueにして送る
 * * 対象の関数が存在しない場合、startedをfalseにして送る
 *
 * serverはそれをそのままcallerに送る
 *
 */
struct CallResponse : public MessageBase<MessageKind::call_response> {
    std::size_t caller_id;
    unsigned int caller_member_id;
    bool started; //!< 関数の実行を開始したかどうか
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("i", caller_id),
                       MSGPACK_NVP("c", caller_member_id),
                       MSGPACK_NVP("s", started))
};
/*!
 * \brief 関数呼び出しの応答2 (client(receiver)->server->client(caller))
 *
 * clientはcalled_id,caller_member_idと、関数の実行結果を返す
 *
 * resultに結果を文字列または数値または真偽値で返す
 * * 結果が無い場合は "" を返す
 * * 例外が発生した場合はis_errorをtrueにしresultに例外の内容を文字列で入れて返す
 *
 * serverはそれをそのままcallerに送る
 *
 */
struct CallResult : public MessageBase<MessageKind::call_result> {
    std::size_t caller_id;
    unsigned int caller_member_id;
    bool is_error;
    Common::ValAdaptor result;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("i", caller_id),
                       MSGPACK_NVP("c", caller_member_id),
                       MSGPACK_NVP("e", is_error), MSGPACK_NVP("r", result))
};
struct Value : public MessageBase<MessageKind::value> {
    std::string field;
    std::shared_ptr<std::vector<double>> data;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("f", field), MSGPACK_NVP("d", data))
};
struct Text : public MessageBase<MessageKind::text> {
    std::string field;
    std::shared_ptr<std::string> data;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("f", field), MSGPACK_NVP("d", data))
};
struct RobotModel : public MessageBase<MessageKind::robot_model> {
    std::string field;
    struct RobotLink {
        std::string name;
        std::string joint_name;
        std::size_t joint_parent;
        Common::RobotJointType joint_type;
        std::array<double, 3> joint_origin_pos, joint_origin_rot;
        double joint_angle = 0;
        Common::GeometryType geometry_type;
        std::vector<double> geometry_properties;
        Common::ViewColor color;
        RobotLink() = default;
        RobotLink(const Common::RobotLink &link,
                  const std::vector<std::string> &link_names)
            : name(link.name), joint_name(link.joint.name),
              joint_parent(
                  std::distance(link_names.begin(),
                                std::find(link_names.begin(), link_names.end(),
                                          link.joint.parent_name))),
              joint_type(link.joint.type),
              joint_origin_pos(link.joint.origin.pos()),
              joint_origin_rot(link.joint.origin.rot()),
              joint_angle(link.joint.angle), geometry_type(link.geometry.type),
              geometry_properties(link.geometry.properties), color(link.color) {
        }
        Common::RobotLink
        toCommonLink(const std::vector<std::string> &link_names) const {
            return Common::RobotLink{
                name,
                {joint_name,
                 joint_parent < link_names.size() ? link_names.at(joint_parent)
                                                  : "",
                 joint_type,
                 {joint_origin_pos, joint_origin_rot},
                 joint_angle},
                Common::Geometry{geometry_type, geometry_properties},
                color,
            };
        }
        MSGPACK_DEFINE_MAP(
            MSGPACK_NVP("n", name), MSGPACK_NVP("jn", joint_name),
            MSGPACK_NVP("jp", joint_parent), MSGPACK_NVP("jt", joint_type),
            MSGPACK_NVP("js", joint_origin_pos),
            MSGPACK_NVP("jr", joint_origin_rot), MSGPACK_NVP("ja", joint_angle),
            MSGPACK_NVP("gt", geometry_type),
            MSGPACK_NVP("gp", geometry_properties), MSGPACK_NVP("c", color))
    };
    std::shared_ptr<std::vector<RobotLink>> data;
    RobotModel() = default;
    RobotModel(const std::string &field,
               const std::shared_ptr<std::vector<RobotLink>> &data)
        : field(field), data(data) {}
    RobotModel(const std::string &field,
               const std::shared_ptr<std::vector<Common::RobotLink>> &common_links)
        : field(field),
          data(std::make_shared<std::vector<RobotLink>>()) {
        data->reserve(common_links->size());
        std::vector<std::string> link_names;
        link_names.reserve(common_links->size());
        for (std::size_t i = 0; i < common_links->size(); i++) {
            data->emplace_back(common_links->at(i), link_names);
            link_names.push_back((*data)[i].name);
        }
    }
    std::shared_ptr<std::vector<Common::RobotLink>> commonLinks() const {
        auto common_links = std::make_shared<std::vector<Common::RobotLink>>();
        common_links->reserve(data->size());
        std::vector<std::string> link_names;
        link_names.reserve(data->size());
        for (std::size_t i = 0; i < data->size(); i++) {
            common_links->push_back((*data)[i].toCommonLink(link_names));
            link_names.push_back((*data)[i].name);
        }
        return common_links;
    }

    MSGPACK_DEFINE_MAP(MSGPACK_NVP("f", field), MSGPACK_NVP("d", data))
};
struct View : public MessageBase<MessageKind::view> {
    std::string field;
    struct ViewComponent {
        Common::ViewComponentType type = Common::ViewComponentType::text;
        std::string text;
        std::optional<std::string> on_click_member, on_click_field;
        Common::ViewColor text_color = Common::ViewColor::inherit,
                          bg_color = Common::ViewColor::inherit;
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
                           MSGPACK_NVP("b", bg_color))
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
                       MSGPACK_NVP("l", length))
};
struct Canvas3D : public MessageBase<MessageKind::canvas3d> {
    std::string field;
    struct Canvas3DComponent {
        Common::Canvas3DComponentType type =
            Common::Canvas3DComponentType::geometry;
        std::array<double, 3> origin_pos, origin_rot;
        ViewColor color;
        std::optional<Common::GeometryType> geometry_type;
        std::vector<double> geometry_properties;
        std::optional<std::string> field_member, field_field;
        std::unordered_map<std::string, double> angles;
        Canvas3DComponent() = default;
        Canvas3DComponent(const Common::Canvas3DComponentBase &vc)
            : type(vc.type_), origin_pos(vc.origin_.pos()),
              origin_rot(vc.origin_.rot()), color(vc.color_), angles() {
            if (vc.geometry_ != std::nullopt) {
                geometry_type = vc.geometry_->type;
                geometry_properties = vc.geometry_->properties;
            }
            if (vc.field_base_ != std::nullopt) {
                field_member = vc.field_base_->member_;
                field_field = vc.field_base_->field_;
            }
            for (const auto &a : vc.angles_) {
                angles.emplace(std::to_string(a.first), a.second);
            }
        }
        operator Common::Canvas3DComponentBase() const {
            Common::Canvas3DComponentBase vc;
            vc.type_ = type;
            vc.origin_ = {origin_pos, origin_rot};
            vc.color_ = color;
            if (geometry_type != std::nullopt) {
                vc.geometry_ = {*geometry_type, geometry_properties};
            }
            if (field_member != std::nullopt) {
                vc.field_base_ = {*field_member, *field_field};
            }
            for (const auto &a : angles) {
                vc.angles_.emplace(std::stoi(a.first), a.second);
            }
            return vc;
        }
        MSGPACK_DEFINE_MAP(MSGPACK_NVP("t", type),
                           MSGPACK_NVP("op", origin_pos),
                           MSGPACK_NVP("or", origin_rot),
                           MSGPACK_NVP("c", color),
                           MSGPACK_NVP("gt", geometry_type),
                           MSGPACK_NVP("gp", geometry_properties),
                           MSGPACK_NVP("fm", field_member),
                           MSGPACK_NVP("ff", field_field),
                           MSGPACK_NVP("a", angles))
    };
    std::shared_ptr<std::unordered_map<std::string, Canvas3DComponent>>
        data_diff;
    std::size_t length;
    Canvas3D() = default;
    Canvas3D(
        const std::string &field,
        const std::shared_ptr<
            std::unordered_map<int, Common::Canvas3DComponentBase>> &data_diff,
        std::size_t length)
        : field(field),
          data_diff(std::make_shared<
                    std::unordered_map<std::string, Canvas3DComponent>>()),
          length(length) {
        for (const auto &vc : *data_diff) {
            this->data_diff->emplace(std::to_string(vc.first), vc.second);
        }
    }
    Canvas3D(const std::string &field,
             const std::shared_ptr<
                 std::unordered_map<std::string, Canvas3DComponent>> &data_diff,
             std::size_t length)
        : field(field), data_diff(data_diff), length(length) {}
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("f", field), MSGPACK_NVP("d", data_diff),
                       MSGPACK_NVP("l", length))
};
struct Canvas2D : public MessageBase<MessageKind::canvas2d> {
    std::string field;
    double width, height;
    struct Canvas2DComponent {
        Common::Canvas2DComponentType type;
        std::array<double, 2> origin_pos;
        double origin_rot;
        ViewColor color, fill;
        double stroke_width;
        Common::GeometryType geometry_type;
        std::vector<double> properties;
        std::optional<std::string> on_click_member, on_click_field;
        Canvas2DComponent() = default;
        Canvas2DComponent(const Common::Canvas2DComponentBase &vc)
            : type(vc.type_),
              origin_pos({vc.origin_.pos(0), vc.origin_.pos(1)}),
              origin_rot(vc.origin_.rot(0)), color(vc.color_), fill(vc.fill_),
              stroke_width(vc.stroke_width_), properties() {
            if (vc.geometry_) {
                geometry_type = vc.geometry_->type;
                properties = vc.geometry_->properties;
            }
            if (vc.on_click_func_) {
                on_click_member = vc.on_click_func_->member_;
                on_click_field = vc.on_click_func_->field_;
            }
        }
        Canvas2DComponent(Canvas2DComponentType type,
                          const Common::Transform &origin, ViewColor color,
                          ViewColor fill, double stroke_width,
                          GeometryType geometry_type,
                          const std::vector<double> &properties)
            : type(type), origin_pos({origin.pos(0), origin.pos(1)}),
              origin_rot(origin.rot(0)), color(color), fill(fill),
              stroke_width(stroke_width), geometry_type(geometry_type),
              properties(properties) {}
        operator Common::Canvas2DComponentBase() const {
            Common::Canvas2DComponentBase vc;
            vc.type_ = type;
            vc.origin_ = {origin_pos, origin_rot};
            vc.color_ = color;
            vc.fill_ = fill;
            vc.stroke_width_ = stroke_width;
            vc.geometry_ = {geometry_type, properties};
            if (on_click_member && on_click_field) {
                vc.on_click_func_ = std::make_optional<FieldBase>(
                    *on_click_member, *on_click_field);
            }
            return vc;
        }
        MSGPACK_DEFINE_MAP(
            MSGPACK_NVP("t", type), MSGPACK_NVP("op", origin_pos),
            MSGPACK_NVP("or", origin_rot), MSGPACK_NVP("c", color),
            MSGPACK_NVP("f", fill), MSGPACK_NVP("s", stroke_width),
            MSGPACK_NVP("gt", geometry_type), MSGPACK_NVP("gp", properties),
            MSGPACK_NVP("L", on_click_member), MSGPACK_NVP("l", on_click_field))
    };
    std::shared_ptr<std::unordered_map<std::string, Canvas2DComponent>>
        data_diff;
    std::size_t length;
    Canvas2D() = default;
    Canvas2D(
        const std::string &field, double width, double height,
        const std::shared_ptr<
            std::unordered_map<int, Common::Canvas2DComponentBase>> &data_diff,
        std::size_t length)
        : field(field), width(width), height(height),
          data_diff(std::make_shared<
                    std::unordered_map<std::string, Canvas2DComponent>>()),
          length(length) {
        for (const auto &vc : *data_diff) {
            this->data_diff->emplace(std::to_string(vc.first), vc.second);
        }
    }
    Canvas2D(const std::string &field, double width, double height,
             const std::shared_ptr<
                 std::unordered_map<std::string, Canvas2DComponent>> &data_diff,
             std::size_t length)
        : field(field), width(width), height(height), data_diff(data_diff),
          length(length) {}
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("f", field), MSGPACK_NVP("w", width),
                       MSGPACK_NVP("h", height), MSGPACK_NVP("d", data_diff),
                       MSGPACK_NVP("l", length))
};
struct Image : public MessageBase<MessageKind::image>,
               public Common::ImageBase {
    std::string field;
    Image() = default;
    Image(const std::string &field, const Common::ImageBase &img)
        : ImageBase(img), field(field) {}
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("f", field), MSGPACK_NVP("d", data_),
                       MSGPACK_NVP("h", rows_), MSGPACK_NVP("w", cols_),
                       MSGPACK_NVP("l", color_mode_),
                       MSGPACK_NVP("p", cmp_mode_))
};
/*!
 * \brief client(member)->server->client logを追加
 *
 * client->server時はmemberは無視
 *
 */
struct Log : public MessageBase<MessageKind::log> {
    unsigned int member_id = 0;
    struct LogLine {
        int level = 0;
        /*!
         * \brief 1970/1/1からの経過ミリ秒
         *
         */
        std::uint64_t time = 0;
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
                           MSGPACK_NVP("m", message))
    };
    std::shared_ptr<std::deque<LogLine>> log;
    Log() = default;
    Log(unsigned int member_id, const std::shared_ptr<std::deque<LogLine>> &log)
        : member_id(member_id), log(log) {}
    template <typename It>
    Log(const It &begin, const It &end) : member_id(0) {
        this->log = std::make_shared<std::deque<LogLine>>();
        for (auto it = begin; it < end; it++) {
            this->log->push_back(*it);
        }
    }
    explicit Log(const Common::LogLine &ll) : member_id(0) {
        this->log = std::make_shared<std::deque<LogLine>>(1);
        this->log->front() = ll;
    }
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("m", member_id), MSGPACK_NVP("l", log))
};
struct LogReq : public MessageBase<MessageKind::log_req> {
    std::string member;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("M", member))
};
/*!
 * \brief client(member)->server->client func登録
 *
 * client->server時はmemberは無視
 *
 */
struct FuncInfo : public MessageBase<MessageKind::func_info> {
    unsigned int member_id = 0;
    std::string field;
    Common::ValType return_type;
    struct Arg : public Common::Arg {
        Arg() = default;
        Arg(const Common::Arg &a) : Common::Arg(a) {}
        MSGPACK_DEFINE_MAP(MSGPACK_NVP("n", name_), MSGPACK_NVP("t", type_),
                           MSGPACK_NVP("i", init_), MSGPACK_NVP("m", min_),
                           MSGPACK_NVP("x", max_), MSGPACK_NVP("o", option_))
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
          return_type(info.return_type),
          args(std::make_shared<std::vector<Arg>>(info.args.size())) {
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
                       MSGPACK_NVP("r", return_type), MSGPACK_NVP("a", args))
};
/*!
 * \brief client->server 以降Recvを送るようリクエスト
 *
 * todo: 解除できるようにする
 *
 */
template <typename T>
struct Req : public MessageBase<T::kind + MessageKind::req> {
    std::string member;
    std::string field;
    unsigned int req_id;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("i", req_id), MSGPACK_NVP("M", member),
                       MSGPACK_NVP("f", field))
};
template <>
struct Req<Image> : public MessageBase<MessageKind::image + MessageKind::req>,
                    public Common::ImageReq {
    std::string member;
    std::string field;
    unsigned int req_id;

    Req() = default;
    Req(const std::string &member, const std::string &field,
        unsigned int req_id, const Common::ImageReq &ireq)
        : Common::ImageReq(ireq), member(member), field(field), req_id(req_id) {
    }

    MSGPACK_DEFINE_MAP(MSGPACK_NVP("i", req_id), MSGPACK_NVP("M", member),
                       MSGPACK_NVP("f", field), MSGPACK_NVP("w", cols),
                       MSGPACK_NVP("h", rows), MSGPACK_NVP("l", color_mode),
                       MSGPACK_NVP("p", cmp_mode), MSGPACK_NVP("q", quality),
                       MSGPACK_NVP("r", frame_rate))
};
/*!
 * \brief server->client 新しいvalueなどの報告
 *
 * Funcの場合はこれではなくFuncInfoを使用
 *
 */
template <typename T>
struct Entry : public MessageBase<T::kind + MessageKind::entry> {
    unsigned int member_id;
    std::string field;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("m", member_id), MSGPACK_NVP("f", field))
};
template <typename T>
struct Res {};
/*!
 * \brief server->client  Value,Textなどのfieldをreqidに変えただけのもの
 *
 * requestしたフィールドの子フィールドの場合sub_fieldにフィールド名を入れて返す
 * →その場合clientが再度requestを送るはず
 *
 */
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
                       MSGPACK_NVP("d", data))
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
                       MSGPACK_NVP("d", data))
};
template <>
struct Res<RobotModel>
    : public MessageBase<MessageKind::robot_model + MessageKind::res> {
    unsigned int req_id;
    std::string sub_field;
    std::shared_ptr<std::vector<RobotModel::RobotLink>> data;
    Res() = default;
    Res(unsigned int req_id, const std::string &sub_field,
        const std::shared_ptr<std::vector<RobotModel::RobotLink>> &data)
        : req_id(req_id), sub_field(sub_field), data(data) {}
    Res(unsigned int req_id, const std::string &sub_field,
        const std::shared_ptr<std::vector<Common::RobotLink>> &common_links)
        : req_id(req_id), sub_field(sub_field),
          data(std::make_shared<std::vector<RobotModel::RobotLink>>()) {
        data->reserve(common_links->size());
        std::vector<std::string> link_names;
        link_names.reserve(common_links->size());
        for (std::size_t i = 0; i < common_links->size(); i++) {
            data->emplace_back(common_links->at(i), link_names);
            link_names.push_back((*data)[i].name);
        }
    }
    std::shared_ptr<std::vector<Common::RobotLink>> commonLinks() const {
        auto common_links = std::make_shared<std::vector<Common::RobotLink>>();
        common_links->reserve(data->size());
        std::vector<std::string> link_names;
        link_names.reserve(data->size());
        for (std::size_t i = 0; i < data->size(); i++) {
            common_links->push_back((*data)[i].toCommonLink(link_names));
            link_names.push_back((*data)[i].name);
        }
        return common_links;
    }

    MSGPACK_DEFINE_MAP(MSGPACK_NVP("i", req_id), MSGPACK_NVP("f", sub_field),
                       MSGPACK_NVP("d", data))
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
                       MSGPACK_NVP("d", data_diff), MSGPACK_NVP("l", length))
};
template <>
struct Res<Canvas3D>
    : public MessageBase<MessageKind::canvas3d + MessageKind::res> {
    unsigned int req_id;
    std::string sub_field;
    std::shared_ptr<
        std::unordered_map<std::string, Canvas3D::Canvas3DComponent>>
        data_diff;
    std::size_t length;
    Res() = default;
    Res(unsigned int req_id, const std::string &sub_field,
        const std::shared_ptr<
            std::unordered_map<std::string, Canvas3D::Canvas3DComponent>>
            &data_diff,
        std::size_t length)
        : req_id(req_id), sub_field(sub_field), data_diff(data_diff),
          length(length) {}
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("i", req_id), MSGPACK_NVP("f", sub_field),
                       MSGPACK_NVP("d", data_diff), MSGPACK_NVP("l", length))
};
template <>
struct Res<Canvas2D>
    : public MessageBase<MessageKind::canvas2d + MessageKind::res> {
    unsigned int req_id;
    std::string sub_field;
    double width, height;
    std::shared_ptr<
        std::unordered_map<std::string, Canvas2D::Canvas2DComponent>>
        data_diff;
    std::size_t length;
    Res() = default;
    Res(unsigned int req_id, const std::string &sub_field, double width,
        double height,
        const std::shared_ptr<
            std::unordered_map<std::string, Canvas2D::Canvas2DComponent>>
            &data_diff,
        std::size_t length)
        : req_id(req_id), sub_field(sub_field), width(width), height(height),
          data_diff(data_diff), length(length) {}
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("i", req_id), MSGPACK_NVP("f", sub_field),
                       MSGPACK_NVP("w", width), MSGPACK_NVP("h", height),
                       MSGPACK_NVP("d", data_diff), MSGPACK_NVP("l", length))
};

template <>
struct Res<Image> : public MessageBase<MessageKind::image + MessageKind::res>,
                    public Common::ImageBase {
    unsigned int req_id;
    std::string sub_field;
    Res() = default;
    Res(unsigned int req_id, const std::string &sub_field,
        const Common::ImageBase &img)
        : ImageBase(img), req_id(req_id), sub_field(sub_field) {}
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("i", req_id), MSGPACK_NVP("f", sub_field),
                       MSGPACK_NVP("d", data_), MSGPACK_NVP("w", cols_),
                       MSGPACK_NVP("h", rows_), MSGPACK_NVP("l", color_mode_),
                       MSGPACK_NVP("p", cmp_mode_))
};
/*!
 * \brief msgpackのメッセージをパースしstd::anyで返す
 *
 */
std::vector<std::pair<int, std::any>>
unpack(const std::string &message,
       const std::shared_ptr<spdlog::logger> &logger);

/*!
 * \brief メッセージ1つを要素数2の配列としてシリアル化
 *
 */
template <typename T>
std::string packSingle(const T &obj) {
    msgpack::type::tuple<int, T> src(static_cast<int>(T::kind), obj);
    std::stringstream buffer;
    msgpack::pack(buffer, src);
    return buffer.str();
}

/*!
 * \brief メッセージをシリアル化しbufferに追加
 *
 */
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

} // namespace WEBCFACE_NS::Message

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
    struct convert<WEBCFACE_NS::Message::Ping>
        : public EmptyConvert<WEBCFACE_NS::Message::Ping> {};
    template <>
    struct convert<WEBCFACE_NS::Message::PingStatusReq>
        : public EmptyConvert<WEBCFACE_NS::Message::PingStatusReq> {};
    template <>
    struct pack<WEBCFACE_NS::Message::Ping>
        : public EmptyPack<WEBCFACE_NS::Message::Ping> {};
    template <>
    struct pack<WEBCFACE_NS::Message::PingStatusReq>
        : public EmptyPack<WEBCFACE_NS::Message::PingStatusReq> {};
    } // namespace adaptor
}
} // namespace msgpack
