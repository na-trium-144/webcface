#pragma once
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include "common/field_base.h"
#include "common/def.h"

WEBCFACE_NS_BEGIN

namespace Internal {
struct ClientData;
}

class Member;
class Value;
class Text;
class Log;
class View;
class Image;
class Func;
class RobotModel;
class Canvas2D;
class Canvas3D;

//! ClientDataの参照とメンバ名とデータ名を持つクラス
struct WEBCFACE_DLL Field : public Common::FieldBase {
    //! ClientDataの参照
    //! ClientData内に保持するクラスもあるので循環参照を避けるためweak_ptr
    std::weak_ptr<Internal::ClientData> data_w;

    Field() = default;
    Field(const std::weak_ptr<Internal::ClientData> &data_w,
          std::string_view member, std::string_view field = "")
        : Common::FieldBase(member, field), data_w(data_w) {}
    Field(const Field &base, std::string_view field)
        : Field(base.data_w, base.member_, field) {}

    //! data_wをlockし、失敗したらruntime_errorを投げる
    std::shared_ptr<Internal::ClientData> dataLock() const;
    //! data_wをlockし、memberがselfではなければinvalid_argumentを投げる
    std::shared_ptr<Internal::ClientData> setCheck() const;

    bool expired() const;

    //! Memberを返す
    Member member() const;
    //! field名を返す
    std::string name() const { return field_; }

    Field child(std::string_view field = "") const {
        if (this->field_.empty()) {
            return Field{*this, field};
        } else if(field.empty()){
            return *this;
        } else {
            return Field{*this, this->field_ + "." + std::string(field)};
        }
    }
    Field child(int index) const { return child(std::to_string(index)); }
    Field operator[](std::string_view field) const { return child(field); }
    Field operator[](int index) const { return child(index); }

    Value value(std::string_view field = "") const;
    Text text(std::string_view field = "") const;
    RobotModel robotModel(std::string_view field = "") const;
    Image image(std::string_view field = "") const;
    Func func(std::string_view field = "") const;
    View view(std::string_view field = "") const;
    Canvas3D canvas3D(std::string_view field = "") const;
    Canvas2D canvas2D(std::string_view field = "") const;
    
    std::vector<Value> valueEntries() const;
    std::vector<Text> textEntries() const;
    std::vector<RobotModel> robotModelEntries() const;
    std::vector<Func> funcEntries() const;
    std::vector<View> viewEntries() const;
    std::vector<Canvas3D> canvas3DEntries() const;
    std::vector<Canvas2D> canvas2DEntries() const;
    std::vector<Image> imageEntries() const;

    /*!
     * \brief memberがselfならtrue
     *
     * data_wがlockできなければruntime_errorを投げる
     *
     */
    bool isSelf() const;

    bool operator==(const Field &other) const;
};
WEBCFACE_NS_END
