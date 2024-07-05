#pragma once
#include <webcface/common/def.h>

WEBCFACE_NS_BEGIN

template <typename Mtx>
struct ScopedUnlock {
    Mtx &m;
    explicit ScopedUnlock(Mtx &m) : m(m) { m.unlock(); }
    ~ScopedUnlock() { m.lock(); }
};

WEBCFACE_NS_END
