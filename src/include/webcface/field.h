#pragma once
#include <memory>
#include <string>
#include "common/field_base.h"
#include "common/def.h"

namespace WEBCFACE_NS {

namespace Internal {
struct ClientData;
}

class Member;

//! ClientDataの参照とメンバ名とデータ名を持つクラス
struct Field : public Common::FieldBase {
    //! ClientDataの参照
    //! ClientData内に保持するクラスもあるので循環参照を避けるためweak_ptr
    std::weak_ptr<Internal::ClientData> data_w;

    Field() = default;
    Field(const std::weak_ptr<Internal::ClientData> &data_w,
          const std::string &member, const std::string &field = "")
        : Common::FieldBase(member, field), data_w(data_w) {}
    Field(const Field &base, const std::string &field)
        : Field(base.data_w, base.member_, field) {}

    //! data_wをlockし、失敗したらruntime_errorを投げる
    WEBCFACE_DLL std::shared_ptr<Internal::ClientData> dataLock() const;
    //! data_wをlockし、memberがselfではなければinvalid_argumentを投げる
    WEBCFACE_DLL std::shared_ptr<Internal::ClientData> setCheck() const;

    //! Memberを返す
    WEBCFACE_DLL Member member() const;
    //! field名を返す
    std::string name() const { return field_; }

    /*!
     * \brief memberがselfならtrue
     *
     * data_wがlockできなければruntime_errorを投げる
     *
     */
    WEBCFACE_DLL bool isSelf() const;
};
} // namespace WEBCFACE_NS
