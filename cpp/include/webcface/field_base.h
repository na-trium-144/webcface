#pragma once
#include <memory>
#include <string>

namespace WebCFace {

class ClientData;

//! ClientDataの参照とメンバ名とデータ名を持つクラス
struct FieldBase {
    //! ClientDataの参照
    //! ClientData内に保持するクラスもあるので循環参照を避けるためweak_ptr
    std::weak_ptr<ClientData> data_w;

    //! メンバー名
    std::string member_;
    //! フィールド名
    //! Member などフィールド名が不要なクラスでは使用しない
    std::string field_;

    FieldBase() = default;
    FieldBase(const std::weak_ptr<ClientData> &data_w,
              const std::string &member, const std::string &field = "")
        : data_w(data_w), member_(member), field_(field) {}
    FieldBase(const FieldBase &base, const std::string &field)
        : FieldBase(base.data_w, base.member_, field) {}

    //! data_wをlockする
    //! 失敗したらruntime_errorを投げる
    //! (clientが死なない限り失敗することはないはず)
    std::shared_ptr<ClientData> dataLock() const;
    void setCheck() const;
};
} // namespace WebCFace