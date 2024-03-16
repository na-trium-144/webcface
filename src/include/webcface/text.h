#pragma once
#include <ostream>
#include <optional>
#include <chrono>
#include <memory>
#include "common/dict.h"
#include "field.h"
#include "event_target.h"
#include "common/def.h"

namespace WEBCFACE_NS {
namespace Internal {
struct ClientData;
}
class Member;

class Text;
extern template class WEBCFACE_IMPORT EventTarget<Text>;

/*!
 * \brief 文字列の送受信データを表すクラス
 *
 * コンストラクタではなく Member::text() を使って取得してください
 *
 */
class WEBCFACE_DLL Text : protected Field, public EventTarget<Text> {
    void onAppend() const override;

  public:
    Text() = default;
    Text(const Field &base);
    Text(const Field &base, const std::string &field)
        : Text(Field{base, field}) {}

    using Field::member;
    using Field::name;

    /*!
     * \brief 子フィールドを返す
     *
     * \return「(thisのフィールド名).(子フィールド名)」をフィールド名とするText
     *
     */
    Text child(const std::string &field) const {
        return Text{*this, this->field_ + "." + field};
    }

    using Dict = Common::Dict<std::shared_ptr<std::string>>;
    /*!
     * \brief Dictの値を再帰的にセットする
     *
     */
    Text &set(const Dict &v);
    /*!
     * \brief 文字列をセットする
     *
     */
    Text &set(const std::string &v);
    /*!
     * \brief Dictの値を再帰的にセットする
     *
     */
    Text &operator=(const Dict &v) {
        this->set(v);
        return *this;
    }
    /*!
     * \brief 文字列をセットする
     *
     */
    Text &operator=(const std::string &v) {
        this->set(v);
        return *this;
    }

    /*!
     * \brief 文字列をリクエストする
     * \since ver1.7
     *
     */
    void request() const;
    /*!
     * \brief 文字列を返す
     *
     */
    std::optional<std::string> tryGet() const;
    /*!
     * \brief 文字列を再帰的に取得しDictで返す
     *
     */
    std::optional<Dict> tryGetRecurse() const;
    /*!
     * \brief 値を返す
     *
     */
    std::string get() const { return tryGet().value_or(""); }
    /*!
     * \brief 値を再帰的に取得しDictで返す
     *
     */
    Dict getRecurse() const { return tryGetRecurse().value_or(Dict{}); }
    operator std::string() const { return get(); }
    operator Dict() const { return getRecurse(); }
    /*!
     * \brief syncの時刻を返す
     * \deprecated 1.7でMember::syncTime()に変更
     */
    [[deprecated]] std::chrono::system_clock::time_point
    time() const;

    /*!
     * \brief 値やリクエスト状態をクリア
     *
     */
    Text &free();

    bool operator==(const std::string &rhs) const { return this->get() == rhs; }
    bool operator!=(const std::string &rhs) const { return this->get() != rhs; }
};

/*!
 * \brief Textをostreamに渡すとTextの中身を表示
 *
 */
WEBCFACE_DLL std::ostream &operator<<(std::ostream &os, const Text &data);

} // namespace WEBCFACE_NS
