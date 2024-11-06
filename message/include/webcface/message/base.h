#pragma once

#ifdef min
// clang-format off
// #pragma message("warning: Disabling macro definition of 'min' and 'max', since they conflicts in message.h.")
// clang-format on
#undef min
#undef max
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <msgpack.hpp>
#include <string>
#include <utility>
#include <vector>
#include <spdlog/logger.h>
#include "webcface/message/u8string.h"
#include "webcface/message/val_adaptor.h"
#include "webcface/encoding/image_mode.h"

MSGPACK_ADD_ENUM(webcface::encoding::ValType)
MSGPACK_ADD_ENUM(webcface::encoding::ImageColorMode)
MSGPACK_ADD_ENUM(webcface::encoding::ImageCompressMode)

WEBCFACE_NS_BEGIN
namespace message {
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
    log = 8,
    entry = 20,
    req = 40,
    res = 60,
    sync_init = 80,
    call = 81,
    call_response = 82,
    call_result = 83,
    func_info = 84,
    log_default = 85,
    log_req_default = 86,
    sync = 87,
    sync_init_end = 88,
    // svr_version = 88,
    ping = 89,
    ping_status = 90,
    ping_status_req = 91,
    log_entry_default = 92,
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
 * \brief client->server 以降Recvを送るようリクエスト
 *
 * todo: 解除できるようにする
 *
 */
template <typename T>
struct Req : public MessageBase<T::kind + MessageKind::req> {
    SharedString member;
    SharedString field;
    unsigned int req_id = 0;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("i", req_id), MSGPACK_NVP("M", member),
                       MSGPACK_NVP("f", field))
};
/*!
 * \brief server->client 新しいvalueなどの報告
 *
 * Funcの場合はこれではなくFuncInfoを使用
 *
 */
template <typename T>
struct Entry : public MessageBase<T::kind + MessageKind::entry> {
    unsigned int member_id = 0;
    SharedString field;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("m", member_id), MSGPACK_NVP("f", field))
};
template <typename T>
struct Res {};


/*!
 * \brief msgpackのメッセージをパースし返す
 *
 */
std::vector<std::pair<int, std::shared_ptr<void>>>
unpack(const std::string &message,
       const std::shared_ptr<spdlog::logger> &logger);

std::shared_ptr<void> unpackCanvas2DSingle(int kind, const msgpack::object &obj, std::size_t index,
             const std::shared_ptr<spdlog::logger> &logger);
std::shared_ptr<void> unpackCanvas3DSingle(int kind, const msgpack::object &obj, std::size_t index,
             const std::shared_ptr<spdlog::logger> &logger);
std::shared_ptr<void> unpackFuncSingle(int kind, const msgpack::object &obj, std::size_t index,
             const std::shared_ptr<spdlog::logger> &logger);
std::shared_ptr<void> unpackImageSingle(int kind, const msgpack::object &obj, std::size_t index,
             const std::shared_ptr<spdlog::logger> &logger);
std::shared_ptr<void> unpackLogSingle(int kind, const msgpack::object &obj, std::size_t index,
             const std::shared_ptr<spdlog::logger> &logger);
std::shared_ptr<void> unpackRobotModelSingle(int kind, const msgpack::object &obj, std::size_t index,
             const std::shared_ptr<spdlog::logger> &logger);
std::shared_ptr<void> unpackSyncSingle(int kind, const msgpack::object &obj, std::size_t index,
             const std::shared_ptr<spdlog::logger> &logger);
std::shared_ptr<void> unpackTextSingle(int kind, const msgpack::object &obj, std::size_t index,
             const std::shared_ptr<spdlog::logger> &logger);
std::shared_ptr<void> unpackValueSingle(int kind, const msgpack::object &obj, std::size_t index,
             const std::shared_ptr<spdlog::logger> &logger);
std::shared_ptr<void> unpackViewSingle(int kind, const msgpack::object &obj, std::size_t index,
             const std::shared_ptr<spdlog::logger> &logger);

template <typename MsgType>
std::shared_ptr<void>
unpackSingleT(const msgpack::object &obj, std::size_t index,
             const std::shared_ptr<spdlog::logger> &logger) {
        try {
            return std::make_shared<MsgType>(
                obj.via.array.ptr[index].as<MsgType>());
        } catch (const std::exception &e) {
            logger->error("unpack error: {} at index={}, kind={}", e.what(),
                          index, static_cast<int>(MsgType::kind));
            return nullptr;
        }
}

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

} // namespace message
WEBCFACE_NS_END
