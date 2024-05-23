#pragma once
#include "utf8/cpp20.h"
#include "webcface/encoding.h"
#include <msgpack.hpp>
#include <string>
#include <webcface/common/def.h>

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    namespace adaptor {

    template <>
    struct convert<std::u8string> {
        msgpack::object const &operator()(msgpack::object const &o,
                                          std::u8string &v) const {
            v = utf8::replace_invalid(std::u8string(
                webcface::Encoding::castToU8(o.via.bin.ptr, o.via.bin.size)));
            return o;
        }
    };
    template <>
    struct pack<std::u8string> {
        template <typename Stream>
        msgpack::packer<Stream> &operator()(msgpack::packer<Stream> &o,
                                            const std::u8string &v) {
            o.pack(webcface::Encoding::castFromU8(v));
            return o;
        }
    };
    } // namespace adaptor
}
} // namespace msgpack
