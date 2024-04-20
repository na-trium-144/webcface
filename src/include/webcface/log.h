#pragma once
#include <memory>
#include <optional>
#include <vector>
#include "event_target.h"
#include "field.h"
#include "common/log.h"
#include "common/def.h"

WEBCFACE_NS_BEGIN

class Log;
extern template class WEBCFACE_IMPORT EventTarget<Log, std::string>;

/*!
 * \brief ログの送受信データを表すクラス
 *
 * fieldを継承しているがfield名は使用していない
 *
 */
class WEBCFACE_DLL Log : protected Field, public EventTarget<Log, MemberNameRef> {
    void onAppend() const override final;

  public:
    Log() = default;
    Log(const Field &base);

    using Field::member;

    /*!
     * \brief ログをリクエストする
     * \since ver1.7
     *
     */
    void request() const;
    /*!
     * \brief ログを取得する
     *
     */
    std::optional<std::vector<LogLine>> tryGet() const;
    /*!
     * \brief ログを取得する
     *
     */
    std::vector<LogLine> get() const {
        return tryGet().value_or(std::vector<LogLine>{});
    }

    /*!
     * \brief (ver1.1.5で追加) 受信したログをクリアする
     *
     * リクエスト状態は解除しない
     */
    Log &clear();

    /*!
     * \brief Logの参照先を比較
     * \since ver1.11
     *
     */
    template <typename T>
        requires std::same_as<T, Log>
    bool operator==(const T &other) const {
        return static_cast<Field>(*this) == static_cast<Field>(other);
    }
    /*!
     * \brief Logの参照先を比較
     * \since ver1.11
     *
     */
    template <typename T>
        requires std::same_as<T, Log>
    bool operator!=(const T &other) const {
        return !(*this == other);
    }
};
WEBCFACE_NS_END
