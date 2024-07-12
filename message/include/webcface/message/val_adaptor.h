#pragma once
#include <webcface/encoding/val_adaptor.h>
#include <msgpack.hpp>
#include <string>
#include <cstdint>
#include <webcface/common/def.h>
#include <utf8.h>
#include "webcface/message/u8string.h"

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    namespace adaptor {

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
    } // namespace adaptor
}
} // namespace msgpack
