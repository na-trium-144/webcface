#pragma once
#include <chrono>
#include <thread>

inline void wait() { std::this_thread::sleep_for(std::chrono::milliseconds(200)); }
