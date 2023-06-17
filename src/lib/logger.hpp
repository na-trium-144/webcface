#pragma once
#include <iostream>
#include <streambuf>
#include <webcface/logger.hpp>
#include <string>
#include <cstddef>
#include <webcface/server.hpp>  // getTime()
#include <deque>
#include <utility>

namespace WebCFace
{
inline namespace Logger
{
struct LogLine {
    int64_t timestamp;
    int level;
    std::string text;
};

// 参考にしたもの: https://wordaligned.org/articles/cpp-streambufs

// coutのrdbufはflushで空になってしまうので、
// coutの出力をcoutのrdbufと別のstreambufの2つに送る必要がある
class teebuf : public std::streambuf
{
public:
    teebuf();
    std::streambuf* sb1 = nullptr;
    std::string tmp_s;

    void set(std::streambuf* sb1) { this->sb1 = sb1; }

private:
    char buf[1024];

    // virtual int overflow(int c);

    // Sync both teed buffers.
    virtual int sync();
};


inline class StdLogger
{
public:
    void set()
    {
        if (!init) {
            buf1.set(std::cout.rdbuf());
            buf2.set(std::cerr.rdbuf());
            std::cout.rdbuf(&buf1);
            std::cerr.rdbuf(&buf2);
            init = true;
        }
    }
    ~StdLogger()
    {
        if (!init) {
            std::cout.rdbuf(buf1.sb1);
            std::cerr.rdbuf(buf2.sb1);
        }
    }

private:
    teebuf buf1, buf2;
    bool init;
    std::deque<LogLine> log_buffer;
    std::size_t log_pos;

public:
    using iterator = decltype(log_buffer)::iterator;
    iterator next()
    {
        std::size_t log_pos2 = log_buffer.size();
        auto itr_next = log_buffer.begin() + log_pos;
        log_pos = log_pos2;
        return itr_next;
    }
    iterator end() { return log_buffer.end(); }
    iterator begin() { return log_buffer.begin(); }
    iterator current() { return log_buffer.begin() + log_pos; }

    void appendLogLine(int level, std::string text);
} std_logger;
}  // namespace Logger
}  // namespace WebCFace
