#pragma once
#include <streambuf>
#include <memory>
#include <string>
#include "webcface/log.h"

WEBCFACE_NS_BEGIN

template <typename CharT>
class BasicLoggerBuf final : public std::basic_streambuf<CharT> {
    using traits_type = typename std::basic_streambuf<CharT>::traits_type;
    using char_type = typename std::basic_streambuf<CharT>::char_type;
    using int_type = typename std::basic_streambuf<CharT>::int_type;

    static constexpr int buf_size = 1024;
    CharT buf[buf_size];
    // bufからあふれた分を入れる
    std::basic_string<CharT> overflow_buf;

    // どうせclientが消える時にはLoggerBufも消える
    internal::ClientData *data_p;
    SharedString field;

    int sync() override;
    int_type overflow(int_type c) override;

  public:
    explicit BasicLoggerBuf(internal::ClientData *data_p, const SharedString &field);
    ~BasicLoggerBuf() = default;
};
#if WEBCFACE_SYSTEM_DLLEXPORT
extern template class BasicLoggerBuf<char>;
extern template class BasicLoggerBuf<wchar_t>;
#endif
using LoggerBuf = BasicLoggerBuf<char>;
using LoggerBufW = BasicLoggerBuf<wchar_t>;

WEBCFACE_NS_END
