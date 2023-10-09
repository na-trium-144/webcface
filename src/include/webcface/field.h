#pragma once
#include <memory>
#include <string>
#include "common/field_base.h"
#include "common/def.h"

namespace WebCFace {

class ClientData;
class Member;

//! ClientDataの参照とメンバ名とデータ名を持つクラス
struct WEBCFACE_DLL Field : public Common::FieldBase {
    //! ClientDataの参照
    //! ClientData内に保持するクラスもあるので循環参照を避けるためweak_ptr
    std::weak_ptr<ClientData> data_w;

    Field() = default;
    Field(const std::weak_ptr<ClientData> &data_w, const std::string &member,
          const std::string &field = "")
        : Common::FieldBase(member, field), data_w(data_w) {}
    Field(const Field &base, const std::string &field)
        : Field(base.data_w, base.member_, field) {}

    //! data_wをlockする
    //! 失敗したらruntime_errorを投げる
    //! (clientが死なない限り失敗することはないはず)
    std::shared_ptr<ClientData> dataLock() const;
    void setCheck() const;

    //! Memberを返す
    Member member() const;
    //! field名を返す
    std::string name() const { return field_; }
};
} // namespace WebCFace