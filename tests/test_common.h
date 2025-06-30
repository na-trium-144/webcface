#pragma once
#include <thread>
#include <chrono>
#include <webcface/common/encoding.h>
#include <string_view>

using namespace webcface;

#ifndef WEBCFACE_TEST_TIMEOUT
#define WEBCFACE_TEST_TIMEOUT 10
#endif

inline void wait() {
    std::this_thread::sleep_for(
        std::chrono::milliseconds(WEBCFACE_TEST_TIMEOUT));
}
inline SharedString operator""_ss(const char *str, std::size_t len) {
    return SharedString::fromU8String(std::string_view(str, len));
}

