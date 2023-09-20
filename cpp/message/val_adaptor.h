#pragma once
#include <webcface/common/val.h>
#include <msgpack.hpp>
#include <string>
#include <cstdint>

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    namespace adaptor {

    template <>
    struct convert<WebCFace::ValAdaptor> {
        msgpack::object const &operator()(msgpack::object const &o,
                                          WebCFace::ValAdaptor &v) const {
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
                v = std::string(o.via.bin.ptr, o.via.bin.size);
                break;
            case msgpack::type::STR:
                v = std::string(o.via.str.ptr, o.via.str.size);
                break;
            default:
                throw msgpack::type_error();
            }
            return o;
        }
    };
    template <>
    struct pack<WebCFace::ValAdaptor> {
        template <typename Stream>
        msgpack::packer<Stream> &operator()(msgpack::packer<Stream> &o,
                                            const WebCFace::ValAdaptor &v) {
            switch (v.valType()) {
            case WebCFace::ValType::bool_:
                o.pack(static_cast<bool>(v));
                break;
            case WebCFace::ValType::int_:
                o.pack(static_cast<std::int64_t>(v));
                break;
            case WebCFace::ValType::float_:
                o.pack(static_cast<double>(v));
                break;
            case WebCFace::ValType::string_:
            default:
                o.pack(static_cast<std::string>(v));
                break;
            }
            return o;
        }
    };
    } // namespace adaptor
}
} // namespace msgpack