#pragma once
#include <msgpack.hpp>
#include <string>

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    namespace adaptor {

    template <>
    struct convert<std::u8string> {
        msgpack::object const &operator()(msgpack::object const &o,
                                          std::u8string &v) const {
            v = std::u8string(o.via.str.ptr, o.via.str.ptr + o.via.str.size);
            return o;
        }
    };
    template <>
    struct pack<std::u8string> {
        template <typename Stream>
        msgpack::packer<Stream> &operator()(msgpack::packer<Stream> &o,
                                            const std::u8string &v) {
            o.pack(std::string(v.cbegin(), v.cend()));
            return o;
        }
    };
    } // namespace adaptor
}
} // namespace msgpack
