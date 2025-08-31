#pragma once
#ifndef SPDLOG_FMT_EXTERNAL
#error "SPDLOG_FMT_EXTERNAL must be enabled. Clear the build directory and try again."
#endif

#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wabi"
#endif
#include <fmt/base.h>
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

#define WEBCFACE_MESSAGE_FMT(Type)                                             \
    template <>                                                                \
    struct fmt::formatter<Type> : formatter<string_view> {                     \
        static constexpr int msg_kind = Type::kind;                            \
        auto format(const Type &,                                              \
                    format_context &) const -> format_context::iterator;       \
    };
