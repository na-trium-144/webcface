#pragma once
#include <memory>
#include <optional>
#include <vector>
#include "event_target.h"
#include "field.h"
#include "common/log.h"

namespace WebCFace {

//! ログの送受信データを表すクラス
/*!
 * fieldを継承しているがfield名は使用していない
 */
class Log : protected Field, public EventTarget<Log, std::string> {

    WEBCFACE_DLL
    std::optional<std::shared_ptr<std::vector<std::shared_ptr<LogLine>>>>
    getRaw() const;
    void setRaw(const std::shared_ptr<std::vector<std::shared_ptr<LogLine>>>
                    &raw) const;

    void onAppend() const override { tryGet(); }

  public:
    Log() = default;
    WEBCFACE_DLL Log(const Field &base);

    using Field::member;

    //! ログを取得する
    std::optional<std::vector<LogLine>> tryGet() const {
        auto v = getRaw();
        if (v) {
            std::vector<LogLine> lv((*v)->size());
            for (std::size_t i = 0; i < (*v)->size(); i++) {
                lv[i] = *(**v)[i];
            }
            return lv;
        } else {
            return std::nullopt;
        }
    }
    //! ログを取得する
    std::vector<LogLine> get() const {
        return tryGet().value_or(std::vector<LogLine>{});
    }

    //! 受信したログをクリアする
    /*! (v1.1.5で追加)
     *
     * リクエスト状態は解除しない
     */
    Log &clear() {
        setRaw(std::make_shared<std::vector<std::shared_ptr<LogLine>>>());
        return *this;
    }
};
} // namespace WebCFace