#pragma once
#include "utf8/cpp20.h"
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
            v.assign(std::bit_cast<const char8_t *>(o.via.str.ptr),
                     o.via.str.size);
            v = utf8::replace_invalid(v);
            return o;
        }
    };
    template <>
    struct pack<std::u8string> {
        template <typename Stream>
        msgpack::packer<Stream> &operator()(msgpack::packer<Stream> &o,
                                            const std::u8string &v) {
            o.pack(std::string_view(std::bit_cast<const char *>(v.data()),
                                    v.size()));
            return o;
        }
    };
    } // namespace adaptor
}
} // namespace msgpack
