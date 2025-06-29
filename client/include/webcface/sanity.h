#pragma once
#include <stdexcept>
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif

WEBCFACE_NS_BEGIN

/*!
 * \brief 未初期化のClientにアクセスしようとした際に投げる例外
 * \since ver2.10
 *
 * ver2.9までは std::runtime_error を投げていた
 *
 */
struct SanityFailure : public std::runtime_error {
    SanityFailure() noexcept
        : std::runtime_error(
              "Tried to access uninitialized or destroyed WebCFace Client?") {}
};

class SanityChecker {
    inline static constexpr std::size_t expected =
        static_cast<std::size_t>(0x942086107530faceuLL);
    std::size_t value;

  public:
    SanityChecker() noexcept : value(expected) {}
    ~SanityChecker() noexcept { value = 0; }
    void check() const {
        if (value != expected) {
            throw SanityFailure();
        }
    }
};

WEBCFACE_NS_END
