#pragma once
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif

WEBCFACE_NS_BEGIN

template <typename Mtx>
struct ScopedUnlock {
    Mtx &m;
    explicit ScopedUnlock(Mtx &m) : m(m) { m.unlock(); }
    ~ScopedUnlock() { m.lock(); }
};

WEBCFACE_NS_END
