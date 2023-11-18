#pragma once
#include <memory>
#include <optional>
#include <vector>
#include "client_data.h"
#include "event_target.h"
#include "field.h"

namespace WebCFace {

//! ログの送受信データを表すクラス
/*!
 * fieldを継承しているがfield名は使用していない
 */
class Log : protected Field, public EventTarget<Log, std::string> {

    void onAppend() const override { tryGet(); }

  public:
    Log() = default;
    Log(const Field &base)
        : Field(base), EventTarget<Log, std::string>(
                           &this->dataLock()->log_append_event, this->member_) {
    }

    using Field::member;

    //! ログを取得する
    std::optional<std::vector<LogLine>> tryGet() const {
        auto v = dataLock()->log_store.getRecv(member_);
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

    Log &clear() {
        dataLock()->log_store.setRecv(
            member_, std::make_shared<std::vector<std::shared_ptr<LogLine>>>());
        return *this;
    }
};
} // namespace WebCFace