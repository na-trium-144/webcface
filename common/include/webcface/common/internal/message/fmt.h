#pragma once
#ifndef SPDLOG_FMT_EXTERNAL
#error "SPDLOG_FMT_EXTERNAL must be enabled. Clear the build directory and try again."
#endif
#include <fmt/base.h>

#define WEBCFACE_MESSAGE_FMT(Type)                                             \
    template <>                                                                \
    struct fmt::formatter<Type> : formatter<string_view> {                     \
        static constexpr int msg_kind = Type::kind;                            \
        auto format(const Type &,                                              \
                    format_context &) const -> format_context::iterator;       \
    };
