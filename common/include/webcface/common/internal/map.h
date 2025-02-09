#pragma once
#include <map>
#include <set>
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif
#include "webcface/common/encoding.h"

WEBCFACE_NS_BEGIN
namespace internal {

template <typename T>
using StrMap1 = std::map<SharedString, T>;
template <typename T>
using StrMap2 = StrMap1<StrMap1<T>>;
using StrSet1 = std::set<SharedString>;
using StrSet2 = StrMap1<StrSet1>;

template <typename M, typename K1, typename K2>
static auto findFromMap2(const M &map, const K1 &key1, const K2 &key2)
    -> std::decay_t<decltype(map.at(key1).at(key2))> {
    auto s_it = map.find(key1);
    if (s_it != map.end()) {
        auto it = s_it->second.find(key2);
        if (it != s_it->second.end()) {
            return it->second;
        }
    }
    return {};
}
template <typename M, typename K1>
static auto findFromMap1(const M &map, const K1 &key1)
    -> std::decay_t<decltype(map.at(key1))> {
    auto it = map.find(key1);
    if (it != map.end()) {
        return it->second;
    }
    return {};
}

} // namespace internal

WEBCFACE_NS_END
