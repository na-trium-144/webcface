#pragma once
#include "utf8.h"
#include "webcface/common/encoding.h"
#include <msgpack.hpp>
#include <string>

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
    } // namespace adaptor
}
} // namespace msgpack
