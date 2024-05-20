#pragma once
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include "common/field_base.h"
#include <webcface/common/def.h>

WEBCFACE_NS_BEGIN

namespace Internal {
struct ClientData;
}

class Member;
class Value;
class Text;
class View;
class Image;
class Func;
class RobotModel;
class Canvas2D;
class Canvas3D;

constexpr char field_separator = '.';

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

    /*!
     * \brief Memberを返す
     *
     */
    Member member() const;
    /*!
     * \brief field名を返す
     *
     */
    std::string name() const { return field_; }

    /*!
     * \brief nameのうちピリオドで区切られた最後の部分を取り出す
     * \since ver1.11
     */
    std::string_view lastName() const;
    /*!
     * \brief nameの最後のピリオドの前までを新しい名前とするField
     * \since ver1.11
     */
    Field parent() const;
    /*!
     * \brief 「(thisの名前).(追加の名前)」を新しい名前とするField
     * \since ver1.11
     */
    Field child(std::string_view field) const;
    /*!
     * \brief 「(thisの名前).(index)」を新しい名前とするField
     * \since ver1.11
     */
    Field child(int index) const { return child(std::to_string(index)); }
    /*!
     * \brief 「(thisの名前).(追加の名前)」を新しい名前とするField
     * \since ver1.11
     */
    Field operator[](std::string_view field) const { return child(field); }
    /*!
     * \brief 「(thisの名前).(index)」を新しい名前とするField
     * \since ver1.11
     */
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
