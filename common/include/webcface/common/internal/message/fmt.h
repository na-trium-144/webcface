#pragma once
#ifdef SPDLOG_FMT_EXTERNAL
#include <fmt/core.h>
#else
#include <spdlog/fmt/bundled/base.h>
#endif

#define WEBCFACE_MESSAGE_FMT(Type)                                             \
    template <>                                                                \
    struct fmt::formatter<Type> : formatter<string_view> {                     \
        static constexpr int msg_kind = Type::kind;                            \
        auto format(const Type &,                                              \
                    format_context &) const -> format_context::iterator;       \
    };
