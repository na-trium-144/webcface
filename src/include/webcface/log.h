#pragma once
#include <memory>
#include <optional>
#include <vector>
#include "event_target.h"
#include "field.h"
#include "common/log.h"

namespace webcface {

//! ログの送受信データを表すクラス
/*!
 * fieldを継承しているがfield名は使用していない
 */
class Log : protected Field, public EventTarget<Log, std::string> {
    WEBCFACE_DLL void onAppend() const override;

  public:
    Log() = default;
    WEBCFACE_DLL Log(const Field &base);

    using Field::member;

    //! ログを取得する
    WEBCFACE_DLL std::optional<std::vector<LogLine>> tryGet() const;
    //! ログを取得する
    std::vector<LogLine> get() const {
        return tryGet().value_or(std::vector<LogLine>{});
    }

    //! 受信したログをクリアする
    /*! (v1.1.5で追加)
     *
     * リクエスト状態は解除しない
     */
    WEBCFACE_DLL Log &clear();
};
} // namespace webcface