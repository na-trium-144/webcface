#include "webcface/common/internal/message/value.h"
#include "webcface/common/internal/message/text.h"
#include "webcface/common/internal/message/view.h"
#include "webcface/common/internal/message/canvas2d.h"
#include "webcface/common/internal/message/canvas3d.h"
#include "webcface/common/internal/message/image.h"
#include "webcface/common/internal/message/func.h"
#include "webcface/common/internal/message/log.h"
#include "webcface/common/internal/message/robot_model.h"
#include "webcface/common/internal/message/sync.h"

#ifdef WEBCFACE_COMPILER_IS_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wabi"
#endif
#include <fmt/std.h>
#include <fmt/chrono.h>
#ifdef WEBCFACE_COMPILER_IS_GCC
#pragma GCC diagnostic pop
#endif

#define WEBCFACE_MESSAGE_FMT_DEF(Type)                                         \
    auto fmt::formatter<Type>::format([[maybe_unused]] const Type &m,          \
                                      format_context &ctx)                     \
        const -> format_context::iterator
#define WEBCFACE_MESSAGE_FMT_DEF_ENTRY(Type)                                   \
    WEBCFACE_MESSAGE_FMT_DEF(                                                  \
        webcface::message::Entry<webcface::message::Type>) {                   \
        return fmt::format_to(ctx.out(),                                       \
                              "{}-" #Type "Entry('{}' from member_id={})",     \
                              msg_kind, m.field.decode(), m.member_id);        \
    }
#define WEBCFACE_MESSAGE_FMT_DEF_REQ(Type)                                     \
    WEBCFACE_MESSAGE_FMT_DEF(                                                  \
        webcface::message::Req<webcface::message::Type>) {                     \
        return fmt::format_to(                                                 \
            ctx.out(), "{}-" #Type "Req('{}' from member '{}' as req_id={})",  \
            msg_kind, m.field.decode(), m.member.decode(), m.req_id);          \
    }

WEBCFACE_MESSAGE_FMT_DEF(webcface::message::SyncInit) {
    return fmt::format_to(ctx.out(),
                          "{}-SyncInit('{}', member_id={}, lib_name='{}', "
                          "lib_ver='{}', addr='{}')",
                          msg_kind, m.member_name.decode(), m.member_id,
                          m.lib_name, m.lib_ver, m.addr);
}
WEBCFACE_MESSAGE_FMT_DEF(webcface::message::SyncInitEnd) {
    return fmt::format_to(ctx.out(),
                          "{}-SyncInitEnd(member_id={}, server_name='{}', "
                          "server_ver='{}', hostname='{}')",
                          msg_kind, m.member_id, m.svr_name, m.ver, m.hostname);
}
WEBCFACE_MESSAGE_FMT_DEF(webcface::message::Ping) {
    return fmt::format_to(ctx.out(), "{}-Ping()", msg_kind);
}
WEBCFACE_MESSAGE_FMT_DEF(webcface::message::PingStatus) {
    return fmt::format_to(ctx.out(), "{}-PingStatus({} data)", msg_kind,
                          m.status->size());
}
WEBCFACE_MESSAGE_FMT_DEF(webcface::message::PingStatusReq) {
    return fmt::format_to(ctx.out(), "{}-PingStatusReq()", msg_kind);
}
WEBCFACE_MESSAGE_FMT_DEF(webcface::message::Sync) {
    return fmt::format_to(ctx.out(), "{}-Sync(member_id={}, time={:%T})",
                          msg_kind, m.member_id, m.getTime());
}

/// \private
template <typename T,
          std::enable_if_t<std::is_enum_v<T>, std::nullptr_t> = nullptr>
static int toInt(const T &v) {
    return static_cast<int>(v);
}
/// \private
template <typename T,
          std::enable_if_t<std::is_enum_v<T>, std::nullptr_t> = nullptr>
static std::optional<int> toInt(const std::optional<T> &v) {
    if (v) {
        return toInt(*v);
    } else {
        return std::nullopt;
    }
}

/// \private
static std::string fmtValue(const std::vector<double> &v) {
    if (v.size() == 1) {
        return std::to_string(v[0]);
    } else {
        return "<" + std::to_string(v.size()) + " values>";
    }
}
WEBCFACE_MESSAGE_FMT_DEF(webcface::message::Value) {
    return fmt::format_to(ctx.out(), "{}-Value('{}', {})", msg_kind,
                          m.field.decode(), fmtValue(*m.data));
}
WEBCFACE_MESSAGE_FMT_DEF(webcface::message::Res<webcface::message::Value>) {
    return fmt::format_to(ctx.out(), "{}-ValueRes(req_id={} + '{}', {})",
                          msg_kind, m.req_id, m.sub_field.decode(),
                          fmtValue(*m.data));
}
WEBCFACE_MESSAGE_FMT_DEF_ENTRY(Value)
WEBCFACE_MESSAGE_FMT_DEF_REQ(Value)

/// \private
static std::string fmtValAdaptor(const webcface::ValAdaptor &v) {
    switch (v.valType()) {
    case webcface::ValType::string_:
        if (v.asStringRef().size() <= 10) {
            return "'" + v.asStringRef() + "'";
        } else {
            return "'" + v.asStringRef().substr(0, 10) + "'...";
        }
    case webcface::ValType::int_:
    case webcface::ValType::float_:
    case webcface::ValType::bool_:
        return v.asStringRef();
    case webcface::ValType::none_:
        return "<none>";
    default:
        return "<unknown type " + std::to_string(toInt(v.valType())) + ">";
    }
}
WEBCFACE_MESSAGE_FMT_DEF(webcface::message::Text) {
    return fmt::format_to(ctx.out(), "{}-Text('{}', {})", msg_kind,
                          m.field.decode(), fmtValAdaptor(*m.data));
}
WEBCFACE_MESSAGE_FMT_DEF(webcface::message::Res<webcface::message::Text>) {
    return fmt::format_to(ctx.out(), "{}-TextRes(req_id={} + '{}', {})",
                          msg_kind, m.req_id, m.sub_field.decode(),
                          fmtValAdaptor(*m.data));
}
WEBCFACE_MESSAGE_FMT_DEF_ENTRY(Text)
WEBCFACE_MESSAGE_FMT_DEF_REQ(Text)

WEBCFACE_MESSAGE_FMT_DEF(webcface::message::RobotModel) {
    return fmt::format_to(ctx.out(), "{}-RobotModel('{}', size={})", msg_kind,
                          m.field.decode(), m.data.size());
}
WEBCFACE_MESSAGE_FMT_DEF(
    webcface::message::Res<webcface::message::RobotModel>) {
    return fmt::format_to(
        ctx.out(), "{}-RobotModelRes(req_id={} + '{}', size={})", msg_kind,
        m.req_id, m.sub_field.decode(), m.data.size());
}
WEBCFACE_MESSAGE_FMT_DEF_ENTRY(RobotModel)
WEBCFACE_MESSAGE_FMT_DEF_REQ(RobotModel)

WEBCFACE_MESSAGE_FMT_DEF(webcface::message::Log) {
    return fmt::format_to(ctx.out(), "{}-Log('{}', {} lines)", msg_kind,
                          m.field.decode(), m.log->size());
}
WEBCFACE_MESSAGE_FMT_DEF(webcface::message::LogDefault) {
    return fmt::format_to(ctx.out(), "{}-LogDefault(member_id={}, {} lines)",
                          msg_kind, m.member_id, m.log->size());
}
WEBCFACE_MESSAGE_FMT_DEF(webcface::message::LogReqDefault) {
    return fmt::format_to(ctx.out(), "{}-LogReqDefault(from '{}')", msg_kind,
                          m.member.decode());
}
WEBCFACE_MESSAGE_FMT_DEF(webcface::message::LogEntryDefault) {
    return fmt::format_to(ctx.out(), "{}-LogEntryDefault(member_id={})",
                          msg_kind, m.member_id);
}
WEBCFACE_MESSAGE_FMT_DEF(webcface::message::Res<webcface::message::Log>) {
    return fmt::format_to(ctx.out(), "{}-LogRes(req_id={} + '{}', {} lines)",
                          msg_kind, m.req_id, m.sub_field.decode(),
                          m.log->size());
}
WEBCFACE_MESSAGE_FMT_DEF_ENTRY(Log)
WEBCFACE_MESSAGE_FMT_DEF_REQ(Log)

WEBCFACE_MESSAGE_FMT_DEF(webcface::message::Image) {
    return fmt::format_to(
        ctx.out(), "{}-Image('{}', {} x {}, {} bytes, color={}, compress={})",
        msg_kind, m.field.decode(), m.width_, m.height_,
        m.data_ ? m.data_->size() : 0, toInt(m.color_mode_),
        toInt(m.cmp_mode_));
}
WEBCFACE_MESSAGE_FMT_DEF(webcface::message::Res<webcface::message::Image>) {
    return fmt::format_to(ctx.out(),
                          "{}-ImageRes(req_id={} + '{}', {} x {}, {} bytes, "
                          "color={}, compress={})",
                          msg_kind, m.req_id, m.sub_field.decode(), m.width_,
                          m.height_, m.data_ ? m.data_->size() : 0,
                          toInt(m.color_mode_), toInt(m.cmp_mode_));
}
WEBCFACE_MESSAGE_FMT_DEF_ENTRY(Image)
WEBCFACE_MESSAGE_FMT_DEF(webcface::message::Req<webcface::message::Image>) {
    return fmt::format_to(ctx.out(),
                          "{}-ImageReq('{}' from member '{}' as req_id={}, {} "
                          "x {}, color={}, cmp={}, q={}, r={})",
                          msg_kind, m.field.decode(), m.member.decode(),
                          m.req_id, m.rows, m.cols, toInt(m.color_mode),
                          toInt(m.cmp_mode), m.quality, m.frame_rate);
}

WEBCFACE_MESSAGE_FMT_DEF(webcface::message::View) {
    return fmt::format_to(ctx.out(), "{}-View('{}', {} diffs)", msg_kind,
                          m.field.decode(), m.data_diff.size());
}
WEBCFACE_MESSAGE_FMT_DEF(webcface::message::Res<webcface::message::View>) {
    return fmt::format_to(ctx.out(), "{}-ViewRes(req_id={} + '{}', {} diffs)",
                          msg_kind, m.req_id, m.sub_field.decode(),
                          m.data_diff.size());
}
WEBCFACE_MESSAGE_FMT_DEF_ENTRY(View)
WEBCFACE_MESSAGE_FMT_DEF_REQ(View)
WEBCFACE_MESSAGE_FMT_DEF(webcface::message::ViewOld) {
    return fmt::format_to(ctx.out(), "{}-ViewOld('{}', {} diffs, length={})",
                          msg_kind, m.field.decode(), m.data_diff.size(),
                          m.length);
}
WEBCFACE_MESSAGE_FMT_DEF(webcface::message::Res<webcface::message::ViewOld>) {
    return fmt::format_to(
        ctx.out(), "{}-ViewOldRes(req_id={} + '{}', {} diffs, length={})",
        msg_kind, m.req_id, m.sub_field.decode(), m.data_diff.size(), m.length);
}
WEBCFACE_MESSAGE_FMT_DEF_ENTRY(ViewOld)
WEBCFACE_MESSAGE_FMT_DEF_REQ(ViewOld)

WEBCFACE_MESSAGE_FMT_DEF(webcface::message::Canvas3D) {
    return fmt::format_to(ctx.out(), "{}-Canvas3D('{}', {} diffs)", msg_kind,
                          m.field.decode(), m.data_diff.size());
}
WEBCFACE_MESSAGE_FMT_DEF(webcface::message::Res<webcface::message::Canvas3D>) {
    return fmt::format_to(
        ctx.out(), "{}-Canvas3DRes(req_id={} + '{}', {} diffs)", msg_kind,
        m.req_id, m.sub_field.decode(), m.data_diff.size());
}
WEBCFACE_MESSAGE_FMT_DEF_ENTRY(Canvas3D)
WEBCFACE_MESSAGE_FMT_DEF_REQ(Canvas3D)
WEBCFACE_MESSAGE_FMT_DEF(webcface::message::Canvas3DOld) {
    return fmt::format_to(ctx.out(),
                          "{}-Canvas3DOld('{}', {} diffs, length={})", msg_kind,
                          m.field.decode(), m.data_diff.size(), m.length);
}
WEBCFACE_MESSAGE_FMT_DEF(
    webcface::message::Res<webcface::message::Canvas3DOld>) {
    return fmt::format_to(
        ctx.out(), "{}-Canvas3DOldRes(req_id={} + '{}', {} diffs, length={})",
        msg_kind, m.req_id, m.sub_field.decode(), m.data_diff.size(), m.length);
}
WEBCFACE_MESSAGE_FMT_DEF_ENTRY(Canvas3DOld)
WEBCFACE_MESSAGE_FMT_DEF_REQ(Canvas3DOld)

WEBCFACE_MESSAGE_FMT_DEF(webcface::message::Canvas2D) {
    return fmt::format_to(ctx.out(), "{}-Canvas2D('{}', {} x {}, {} diffs)",
                          msg_kind, m.field.decode(), m.width, m.height,
                          m.data_diff.size());
}
WEBCFACE_MESSAGE_FMT_DEF(webcface::message::Res<webcface::message::Canvas2D>) {
    return fmt::format_to(ctx.out(),
                          "{}-Canvas2DRes(req_id={} + '{}', {} x {}, {} diffs)",
                          msg_kind, m.req_id, m.sub_field.decode(), m.width,
                          m.height, m.data_diff.size());
}
WEBCFACE_MESSAGE_FMT_DEF_ENTRY(Canvas2D)
WEBCFACE_MESSAGE_FMT_DEF_REQ(Canvas2D)
WEBCFACE_MESSAGE_FMT_DEF(webcface::message::Canvas2DOld) {
    return fmt::format_to(ctx.out(),
                          "{}-Canvas2DOld('{}', {} x {}, {} diffs, length={})",
                          msg_kind, m.field.decode(), m.width, m.height,
                          m.data_diff.size(), m.length);
}
WEBCFACE_MESSAGE_FMT_DEF(
    webcface::message::Res<webcface::message::Canvas2DOld>) {
    return fmt::format_to(
        ctx.out(),
        "{}-Canvas2DOldRes(req_id={} + '{}', {} x {}, {} diffs, length={})",
        msg_kind, m.req_id, m.sub_field.decode(), m.width, m.height,
        m.data_diff.size(), m.length);
}
WEBCFACE_MESSAGE_FMT_DEF_ENTRY(Canvas2DOld)
WEBCFACE_MESSAGE_FMT_DEF_REQ(Canvas2DOld)

WEBCFACE_MESSAGE_FMT_DEF(webcface::message::FuncInfo) {
    return fmt::format_to(
        ctx.out(), "{}-FuncInfo('{}' from member_id={}, {} args, return={})",
        msg_kind, m.field.decode(), m.member_id, m.args.size(),
        webcface::valTypeStr(m.return_type));
}
WEBCFACE_MESSAGE_FMT_DEF(webcface::message::Call) {
    return fmt::format_to(ctx.out(),
                          "{}-Call(caller_id={} from member_id={}, to '{}' of "
                          "member_id={}, {} args)",
                          msg_kind, m.caller_id, m.caller_member_id,
                          m.field.decode(), m.target_member_id, m.args.size());
}
WEBCFACE_MESSAGE_FMT_DEF(webcface::message::CallResponse) {
    return fmt::format_to(
        ctx.out(),
        "{}-CallResponse(for caller_id={} from member_id={}, started={})",
        msg_kind, m.caller_id, m.caller_member_id, m.started);
}
WEBCFACE_MESSAGE_FMT_DEF(webcface::message::CallResult) {
    return fmt::format_to(
        ctx.out(),
        "{}-CallResult(for caller_id={} from member_id={}, is_error={}, {})",
        msg_kind, m.caller_id, m.caller_member_id, m.is_error,
        fmtValAdaptor(m.result));
}
