#pragma once
#include <ostream>
#include <optional>
#include <chrono>
#include <memory>
#include "common/dict.h"
#include "field.h"
#include "event_target.h"

namespace webcface {
namespace Internal {
struct ClientData;
}
class Member;

//! 文字列の送受信データを表すクラス
/*! コンストラクタではなく Member::text() を使って取得してください
 */
class Text : protected Field, public EventTarget<Text> {
    WEBCFACE_DLL void onAppend() const override;

  public:
    Text() = default;
    WEBCFACE_DLL Text(const Field &base);
    WEBCFACE_DLL Text(const Field &base, const std::string &field)
        : Text(Field{base, field}) {}

    using Field::member;
    using Field::name;

    //! 子フィールドを返す
    /*!
     * \return「(thisのフィールド名).(子フィールド名)」をフィールド名とするValue
     */
    Text child(const std::string &field) {
        return Text{*this, this->field_ + "." + field};
    }

    using Dict = Common::Dict<std::shared_ptr<std::string>>;
    //! Dictの値を再帰的にセットする
    WEBCFACE_DLL Text &set(const Dict &v);
    //! 文字列をセットする
    WEBCFACE_DLL Text &set(const std::string &v);
    //! Dictの値を再帰的にセットする
    Text &operator=(const Dict &v) {
        this->set(v);
        return *this;
    }
    //! 文字列をセットする
    Text &operator=(const std::string &v) {
        this->set(v);
        return *this;
    }

    //! 文字列を返す
    WEBCFACE_DLL std::optional<std::string> tryGet() const;
    //! 文字列を再帰的に取得しDictで返す
    WEBCFACE_DLL std::optional<Dict> tryGetRecurse() const;
    //! 値を返す
    std::string get() const { return tryGet().value_or(""); }
    //! 値を再帰的に取得しDictで返す
    Dict getRecurse() const { return tryGetRecurse().value_or(Dict{}); }
    operator std::string() const { return get(); }
    operator Dict() const { return getRecurse(); }
    //! syncの時刻を返す
    WEBCFACE_DLL std::chrono::system_clock::time_point time() const;

    //! 値やリクエスト状態をクリア
    WEBCFACE_DLL Text &free();

    bool operator==(const std::string &rhs) const { return this->get() == rhs; }
    bool operator!=(const std::string &rhs) const { return this->get() != rhs; }
};

inline std::ostream &operator<<(std::ostream &os, const Text &data) {
    return os << data.get();
}
} // namespace webcface
