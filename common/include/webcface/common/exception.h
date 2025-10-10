#pragma once
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif
#include <stdexcept>

WEBCFACE_NS_BEGIN

/*!
 * \brief その他のパラメーターエラー
 * \since ver3.0
 * 
 */
struct WEBCFACE_DLL InvalidArgument : public std::invalid_argument {
    explicit InvalidArgument(const char *message);
    explicit InvalidArgument(const std::string &message);
};
/*!
 * \brief その他のパラメーターエラー
 * \since ver3.0
 * 
 */
struct WEBCFACE_DLL OutOfRange : public std::out_of_range {
    explicit OutOfRange(const char *message);
    explicit OutOfRange(const std::string &message);
};

/*!
 * \brief ValAdaptorの変換エラー
 * \since ver3.0
 * 
 */
struct WEBCFACE_DLL ValTypeMismatch : public InvalidArgument {
    explicit ValTypeMismatch(const char *message);
    explicit ValTypeMismatch(const std::string &message);
};
WEBCFACE_NS_END
