#pragma once
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif
#include <stdexcept>

WEBCFACE_NS_BEGIN
struct FieldBase;

/*
 * MSVCではexceptionをexportしないほうがよいらしくC4275警告を出すが、
 * Macではexportしないとcatchできなくなる
 *
 */

/*!
 * \brief Funcの実行ができなかった場合発生する例外
 *
 * ValueやTextで参照先が見つからなかった場合はこれではなく単にnulloptが返る
 *
 */
struct WEBCFACE_DLL FuncNotFound : public std::runtime_error {
    explicit FuncNotFound(const FieldBase &base);
};

/*!
 * \brief Clientが破棄されたあとにフィールドにアクセスした場合に発生する例外
 * \since ver2.10
 * 
 * ver2.9までは std::runtime_error を投げていた
 * 
 */
struct WEBCFACE_DLL SanityError : public std::runtime_error {
    explicit SanityError(const char *message) noexcept;
};
/*!
 * \brief 自分以外のmemberのフィールドに値を設定しようとしたときに発生する例外
 * \since ver2.10
 * 
 * ver2.9までは std::invalid_argument を投げていた
 * 
 */
struct WEBCFACE_DLL Intrusion : public std::invalid_argument {
    explicit Intrusion(const FieldBase &base);
};


WEBCFACE_NS_END
