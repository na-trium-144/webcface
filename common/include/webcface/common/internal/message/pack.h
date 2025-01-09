#pragma once
#ifdef MSGPACK_DEFINE_MAP
#error "webcface/common/internal/pack.h must be included first"
#endif

#include <msgpack.hpp>
#include <string>
#include <cstdint>
#include <spdlog/logger.h>
#include <utf8.h>
#include "webcface/common/val_adaptor.h"
#include "./base.h"

MSGPACK_ADD_ENUM(webcface::ValType)

WEBCFACE_NS_BEGIN
namespace message {
struct Ping;
struct PingStatusReq;

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
struct Value;
template <>
struct Entry<Value>;

/*!
 * \brief バイナリを16進数の文字列に変換
 * \since ver2.6
 */
std::string messageTrace(const std::string &message);

/*!
 * \brief msgpackのメッセージをパースし返す
 *
 */
std::vector<std::pair<int, std::shared_ptr<void>>>
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

} // namespace message
WEBCFACE_NS_END

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    namespace adaptor {

    template <>
    struct convert<webcface::SharedString> {
        msgpack::object const &operator()(msgpack::object const &o,
                                          webcface::SharedString &v) const {
            v = webcface::SharedString::fromU8String(utf8::replace_invalid(
                std::string(o.via.bin.ptr, o.via.bin.size)));
            return o;
        }
    };
    template <>
    struct pack<webcface::SharedString> {
        template <typename Stream>
        msgpack::packer<Stream> &operator()(msgpack::packer<Stream> &o,
                                            const webcface::SharedString &v) {
            o.pack(v.u8String());
            return o;
        }
    };

    template <>
    struct convert<webcface::ValAdaptor> {
        msgpack::object const &operator()(msgpack::object const &o,
                                          webcface::ValAdaptor &v) const {
            switch (o.type) {
            case msgpack::type::FLOAT32:
            case msgpack::type::FLOAT64:
                v = o.via.f64;
                break;
            case msgpack::type::POSITIVE_INTEGER:
                v = o.via.u64;
                break;
            case msgpack::type::NEGATIVE_INTEGER:
                v = o.via.i64;
                break;
            case msgpack::type::BOOLEAN:
                v = o.via.boolean;
                break;
            case msgpack::type::BIN:
            case msgpack::type::STR:
                v = webcface::SharedString::fromU8String(utf8::replace_invalid(
                    std::string(o.via.bin.ptr, o.via.bin.size)));
                break;
            default:
                throw msgpack::type_error();
            }
            return o;
        }
    };
    template <>
    struct pack<webcface::ValAdaptor> {
        template <typename Stream>
        msgpack::packer<Stream> &operator()(msgpack::packer<Stream> &o,
                                            const webcface::ValAdaptor &v) {
            switch (v.valType()) {
            case webcface::ValType::bool_:
                o.pack(static_cast<bool>(v));
                break;
            case webcface::ValType::int_:
                o.pack(static_cast<std::int64_t>(v));
                break;
            case webcface::ValType::float_:
                o.pack(static_cast<double>(v));
                break;
            case webcface::ValType::string_:
            default:
                o.pack(v.asU8StringRef());
                break;
            }
            return o;
        }
    };

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
    struct convert<webcface::message::Ping>
        : public EmptyConvert<webcface::message::Ping> {};
    template <>
    struct convert<webcface::message::PingStatusReq>
        : public EmptyConvert<webcface::message::PingStatusReq> {};
    template <>
    struct pack<webcface::message::Ping>
        : public EmptyPack<webcface::message::Ping> {};
    template <>
    struct pack<webcface::message::PingStatusReq>
        : public EmptyPack<webcface::message::PingStatusReq> {};

    } // namespace adaptor
}
} // namespace msgpack
