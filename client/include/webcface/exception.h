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
 * \brief 未初期化の変数にアクセスした場合に発生する例外
 * \since ver2.10
 * 
 * * Clientが破棄されたあとにフィールドにアクセスした場合など
 * * ver2.9までは std::runtime_error を投げていた
 * 
 */
struct WEBCFACE_DLL SanityError : public std::runtime_error {
    explicit SanityError(const char *message);
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

/*!
 * \brief Funcの実行結果の処理中に発生した例外
 * \since ver2.10
 * 
 * * ver2.9までは std::runtime_error を投げていた
 * * 実行結果として返されるエラーそのものはPromiseErrorではなく std::runtime_error
 * 
 * \todo std::future_error はlogic_errorを継承している。合わせるべきか？
 * 
 */
struct WEBCFACE_DLL PromiseError : public std::runtime_error {
    explicit PromiseError(const char *message);
};

/*!
 * \brief Funcにセットしようとしたパラメーターが実際の関数と一致しない場合の例外
 * \since ver2.10
 * 
 */
struct WEBCFACE_DLL FuncSignatureMismatch : public std::invalid_argument {
    explicit FuncSignatureMismatch(const char *message);
    explicit FuncSignatureMismatch(const std::string &message);
};

/*!
 * \brief その他のパラメーターエラー
 * \since ver2.10
 * 
 */
struct WEBCFACE_DLL InvalidArgument : public std::invalid_argument {
    explicit InvalidArgument(const char *message);
    explicit InvalidArgument(const std::string &message);
};
/*!
 * \brief その他のパラメーターエラー
 * \since ver2.10
 * 
 */
struct WEBCFACE_DLL OutOfRange : public std::out_of_range {
    explicit OutOfRange(const char *message);
    explicit OutOfRange(const std::string &message);
};

WEBCFACE_NS_END
