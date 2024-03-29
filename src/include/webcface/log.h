#pragma once
#include <memory>
#include <optional>
#include <vector>
#include "event_target.h"
#include "field.h"
#include "common/log.h"
#include "common/def.h"

namespace WEBCFACE_NS {

class Log;
extern template class WEBCFACE_IMPORT EventTarget<Log, std::string>;

/*!
 * \brief ログの送受信データを表すクラス
 *
 * fieldを継承しているがfield名は使用していない
 *
 */
class WEBCFACE_DLL Log : protected Field, public EventTarget<Log, std::string> {
    void onAppend() const override;

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
};
} // namespace WEBCFACE_NS
