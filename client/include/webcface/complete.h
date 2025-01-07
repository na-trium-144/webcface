#pragma once
#include <cstddef>
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif

WEBCFACE_NS_BEGIN
namespace traits {
constexpr bool isComplete(...) { return false; }
template <typename T, int = sizeof(T)>
constexpr bool isComplete(T *) {
    return true;
}

#define define_assert_complete(Type, header)                                   \
    template <typename T>                                                      \
    constexpr std::nullptr_t assertComplete##Type() {                          \
        static_assert(isComplete((T *)nullptr),                                \
                      "Please include <webcface/" #header                      \
                      "> to use class " #Type "!");                            \
        return nullptr;                                                        \
    }

// clang-format off
define_assert_complete(Member, member.h)
define_assert_complete(Value, value.h)
define_assert_complete(Text, text.h)
define_assert_complete(View, view.h)
define_assert_complete(Image, image.h)
define_assert_complete(Func, func.h)
define_assert_complete(FuncListener, func.h)
define_assert_complete(RobotModel, robot_model.h)
define_assert_complete(Canvas2D, canvas2d.h)
define_assert_complete(Canvas3D, canvas3d.h)
define_assert_complete(Log, log.h)
// clang-format on

#undef define_assert_complete

#define WEBCFACE_COMPLETE(Type)                                                \
    typename Type##_ = Type,                                                   \
             std::nullptr_t =                                                  \
                 ::webcface::traits::assertComplete##Type<Type##_>()

} // namespace traits
WEBCFACE_NS_END
