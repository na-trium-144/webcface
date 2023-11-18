#pragma once
#include <ostream>
#include <optional>
#include <chrono>
#include <memory>
#include "common/dict.h"
#include "field.h"
#include "client_data.h"
#include "event_target.h"

namespace WebCFace {

//! 文字列の送受信データを表すクラス
/*! コンストラクタではなく Member::text() を使って取得してください
 */
class Text : protected Field, public EventTarget<Text> {

    void onAppend() const override { tryGet(); }

    // set(Dict)の内部でつかう
    Text &set(const std::shared_ptr<std::string> &v) {
        setCheck();
        dataLock()->text_store.setSend(*this, v);
        this->triggerEvent(*this);
        return *this;
    }

  public:
    Text() = default;
    Text(const Field &base)
        : Field(base), EventTarget<Text>(&this->dataLock()->text_change_event,
                                         *this) {}
    Text(const Field &base, const std::string &field)
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
    Text &set(const Dict &v) {
        if (v.hasValue()) {
            set(v.getRaw());
        } else {
            for (const auto &it : v.getChildren()) {
                child(it.first).set(it.second);
            }
        }
        return *this;
    }
    //! 文字列をセットする
    Text &set(const std::string &v) {
        return set(std::make_shared<std::string>(v));
    }
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
    std::optional<std::string> tryGet() const {
        auto v = dataLock()->text_store.getRecv(*this);
        if (v) {
            return **v;
        } else {
            return std::nullopt;
        }
    }
    //! 文字列を再帰的に取得しDictで返す
    std::optional<Dict> tryGetRecurse() const {
        return dataLock()->text_store.getRecvRecurse(*this);
    }
    //! 値を返す
    std::string get() const { return tryGet().value_or(""); }
    //! 値を再帰的に取得しDictで返す
    Dict getRecurse() const { return tryGetRecurse().value_or(Dict{}); }
    operator std::string() const { return get(); }
    operator Dict() const { return getRecurse(); }
    //! syncの時刻を返す
    std::chrono::system_clock::time_point time() const {
        return dataLock()
            ->sync_time_store.getRecv(this->member_)
            .value_or(std::chrono::system_clock::time_point());
    }

    // //! このtext非表示にする
    // //! (他clientのentryに表示されなくする)
    // auto &hidden(bool hidden) {
    //     setCheck();
    //     dataLock()->text_store.setHidden(*this, hidden);
    //     return *this;
    // }

    //! 値やリクエスト状態をクリア
    Text &free() {
        dataLock()->text_store.unsetRecv(*this);
        return *this;
    }

    bool operator==(const std::string &rhs) const { return this->get() == rhs; }
    bool operator!=(const std::string &rhs) const { return this->get() != rhs; }
};

inline std::ostream &operator<<(std::ostream &os,
                                            const Text &data) {
    return os << data.get();
}
} // namespace WebCFace