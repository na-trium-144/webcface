#pragma once
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif
#include "webcface/common/encoding.h"
#include "webcface/complete.h"

WEBCFACE_NS_BEGIN

namespace internal {
struct ClientData;
}

class Member;
class Value;
class Text;
class View;
class Image;
class Func;
class FuncListener;
class RobotModel;
class Plot;
class Canvas2D;
class Canvas3D;
class Log;

constexpr char field_separator = '.';

/*!
 * \brief メンバ名とデータ名を持つクラス
 *
 */
struct WEBCFACE_DLL FieldBase {
    /*!
     * \brief メンバー名
     *
     */
    SharedString member_;

    /*!
     * \brief フィールド名
     *
     * Memberなどフィールド名が不要なクラスでは使用しない
     *
     */
    SharedString field_;

    FieldBase() = default;
    explicit FieldBase(const SharedString &member)
        : member_(member), field_() {}
    FieldBase(const SharedString &member, const SharedString &field)
        : member_(member), field_(field) {}
    FieldBase(const FieldBase &base, const SharedString &field)
        : member_(base.member_), field_(field) {}

    bool operator==(const FieldBase &rhs) const {
        return this->member_ == rhs.member_ && this->field_ == rhs.field_;
    }
    bool operator!=(const FieldBase &rhs) const { return !(*this == rhs); }
};

/*!
 * \brief ClientDataの参照とメンバ名とデータ名を持つクラス
 *
 */
struct WEBCFACE_DLL Field : public FieldBase {
    /*!
     * \brief ClientDataの参照
     *
     * ClientData内に保持するクラスもあるので循環参照を避けるためweak_ptr
     */
    std::weak_ptr<internal::ClientData> data_w;

    Field() = default;
    Field(const std::weak_ptr<internal::ClientData> &data_w,
          const SharedString &member)
        : FieldBase(member), data_w(data_w) {}
    Field(const std::weak_ptr<internal::ClientData> &data_w,
          const SharedString &member, const SharedString &field)
        : FieldBase(member, field), data_w(data_w) {}
    Field(const Field &base, const SharedString &field)
        : FieldBase(base, field), data_w(base.data_w) {}

    /*!
     * \brief data_wをlockし、失敗したらruntime_errorを投げる
     *
     */
    std::shared_ptr<internal::ClientData> dataLock() const;
    /*!
     * \brief data_wをlockし、memberがselfではなければinvalid_argumentを投げる
     *
     */
    std::shared_ptr<internal::ClientData> setCheck() const;

    bool expired() const;

    /*!
     * \brief Memberを返す
     *
     */
    template <WEBCFACE_COMPLETE(Member)>
    Member_ member() const {
        return *this;
    }
    /*!
     * \brief field名を返す
     *
     */
    const std::string &name() const { return field_.decode(); }
    /*!
     * \brief field名を返す (wstring)
     * \since ver2.0
     */
    const std::wstring &nameW() const { return field_.decodeW(); }

  protected:
    SharedString lastName8() const;

  public:
    Field child(const SharedString &field) const;

    /*!
     * \brief nameのうちピリオドで区切られた最後の部分を取り出す
     * \since ver1.11
     */
    std::string lastName() const { return lastName8().decode(); }
    /*!
     * \brief nameのうちピリオドで区切られた最後の部分を取り出す (wstring)
     * \since ver2.0
     */
    std::wstring lastNameW() const { return lastName8().decodeW(); }
    /*!
     * \brief nameの最後のピリオドの前までを新しい名前とするField
     * \since ver1.11
     */
    Field parent() const;
    /*!
     * \brief 「(thisの名前).(追加の名前)」を新しい名前とするField
     * \since ver1.11
     */
    Field child(std::string_view field) const {
        return child(SharedString::encode(field));
    }
    /*!
     * \brief 「(thisの名前).(追加の名前)」を新しい名前とするField (wstring)
     * \since ver2.0
     */
    Field child(std::wstring_view field) const {
        return child(SharedString::encode(field));
    }
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
     * \brief 「(thisの名前).(追加の名前)」を新しい名前とするField (wstring)
     * \since ver2.0
     */
    Field operator[](std::wstring_view field) const { return child(field); }
    /*!
     * \brief 「(thisの名前).(index)」を新しい名前とするField
     * \since ver1.11
     */
    Field operator[](int index) const { return child(index); }

    template <WEBCFACE_COMPLETE(Value)>
    Value_ value(std::string_view field = "") const {
        return child(field);
    }
    template <WEBCFACE_COMPLETE(Value)>
    Value_ value(std::wstring_view field) const {
        return child(field);
    }
    template <WEBCFACE_COMPLETE(Text)>
    Text_ text(std::string_view field = "") const {
        return child(field);
    }
    template <WEBCFACE_COMPLETE(Text)>
    Text_ text(std::wstring_view field) const {
        return child(field);
    }
    template <WEBCFACE_COMPLETE(RobotModel)>
    RobotModel_ robotModel(std::string_view field = "") const {
        return child(field);
    }
    template <WEBCFACE_COMPLETE(RobotModel)>
    RobotModel_ robotModel(std::wstring_view field) const {
        return child(field);
    }
    template <WEBCFACE_COMPLETE(Plot)>
    Plot_ image(std::string_view field = "") const {
        return child(field);
    }
    template <WEBCFACE_COMPLETE(Plot)>
    Plot_ image(std::wstring_view field) const {
        return child(field);
    }
    template <WEBCFACE_COMPLETE(Image)>
    Image_ image(std::string_view field = "") const {
        return child(field);
    }
    template <WEBCFACE_COMPLETE(Image)>
    Image_ image(std::wstring_view field) const {
        return child(field);
    }
    template <WEBCFACE_COMPLETE(Func)>
    Func_ func(std::string_view field = "") const {
        return child(field);
    }
    template <WEBCFACE_COMPLETE(Func)>
    Func_ func(std::wstring_view field) const {
        return child(field);
    }
    template <WEBCFACE_COMPLETE(FuncListener)>
    FuncListener_ funcListener(std::string_view field = "") const {
        return child(field);
    }
    template <WEBCFACE_COMPLETE(FuncListener)>
    FuncListener_ funcListener(std::wstring_view field) const {
        return child(field);
    }
    template <WEBCFACE_COMPLETE(View)>
    View_ view(std::string_view field = "") const {
        return child(field);
    }
    template <WEBCFACE_COMPLETE(View)>
    View_ view(std::wstring_view field) const {
        return child(field);
    }
    template <WEBCFACE_COMPLETE(Canvas3D)>
    Canvas3D_ canvas3D(std::string_view field = "") const {
        return child(field);
    }
    template <WEBCFACE_COMPLETE(Canvas3D)>
    Canvas3D_ canvas3D(std::wstring_view field) const {
        return child(field);
    }
    template <WEBCFACE_COMPLETE(Canvas2D)>
    Canvas2D_ canvas2D(std::string_view field = "") const {
        return child(field);
    }
    template <WEBCFACE_COMPLETE(Canvas2D)>
    Canvas2D_ canvas2D(std::wstring_view field) const {
        return child(field);
    }
    /*!
     * \since ver2.4
     */
    template <WEBCFACE_COMPLETE(Log)>
    Log_ log(std::string_view field = "") const {
        return child(field);
    }
    /*!
     * \since ver2.4
     */
    template <WEBCFACE_COMPLETE(Log)>
    Log_ log(std::wstring_view field) const {
        return child(field);
    }


    /*!
     * \brief 「(thisの名前).(追加の名前)」で公開されているデータのリスト
     * \since ver2.6
     *
     * * データ型を問わずすべてのデータを列挙する。
     * * childrenRecurse() と異なり、
     * 名前にさらにピリオドが含まれる場合はその前までの名前を返す。
     * * 同名で複数のデータが存在する場合も1回のみカウントする。
     *
     * \sa childrenRecurse(), hasChildren()
     */
    std::vector<Field> children() const;
    /*!
     * \brief 「(thisの名前).(追加の名前)」で公開されているデータのリスト(再帰)
     * \since ver2.6
     *
     * * データ型を問わずすべてのデータを列挙する。
     * * 同名で複数のデータが存在する場合も1回のみカウントする。
     *
     * \sa children(), hasChildren()
     */
    std::vector<Field> childrenRecurse() const;
    /*!
     * \brief
     * 「(thisの名前).(追加の名前)」で公開されているデータが存在するかどうかを返す
     * \since ver2.6
     * \sa children(), childrenRecurse()
     */
    bool hasChildren() const;

    /*!
     * \brief 「(thisの名前).(追加の名前)」で公開されているvalueのリストを返す。
     * \since ver1.6
     * \sa childrenRecurse()
     */
    template <WEBCFACE_COMPLETE(Value)>
    std::vector<Value_> valueEntries() const;
    /*!
     * \brief 「(thisの名前).(追加の名前)」で公開されているtextのリストを返す。
     * \since ver1.6
     * \sa childrenRecurse()
     */
    template <WEBCFACE_COMPLETE(Text)>
    std::vector<Text_> textEntries() const;
    /*!
     * \brief
     * 「(thisの名前).(追加の名前)」で公開されているrobotModelのリストを返す。
     * \since ver1.6
     * \sa childrenRecurse()
     */
    template <WEBCFACE_COMPLETE(RobotModel)>
    std::vector<RobotModel_> robotModelEntries() const;
    /*!
     * \brief 「(thisの名前).(追加の名前)」で公開されているplotのリストを返す。
     * \since ver2.8
     * \sa childrenRecurse()
     */
    template <WEBCFACE_COMPLETE(Plot)>
    std::vector<Plot_> funcEntries() const;
    /*!
     * \brief 「(thisの名前).(追加の名前)」で公開されているfuncのリストを返す。
     * \since ver1.6
     * \sa childrenRecurse()
     */
    template <WEBCFACE_COMPLETE(Func)>
    std::vector<Func_> funcEntries() const;
    /*!
     * \brief 「(thisの名前).(追加の名前)」で公開されているviewのリストを返す。
     * \since ver1.6
     * \sa childrenRecurse()
     */
    template <WEBCFACE_COMPLETE(View)>
    std::vector<View_> viewEntries() const;
    /*!
     * \brief
     * 「(thisの名前).(追加の名前)」で公開されているcanvas2Dのリストを返す。
     * \since ver1.6
     * \sa childrenRecurse()
     */
    template <WEBCFACE_COMPLETE(Canvas2D)>
    std::vector<Canvas2D_> canvas2DEntries() const;
    /*!
     * \brief
     * 「(thisの名前).(追加の名前)」で公開されているcanvas3Dのリストを返す。
     * \since ver1.6
     * \sa childrenRecurse()
     */
    template <WEBCFACE_COMPLETE(Canvas3D)>
    std::vector<Canvas3D_> canvas3DEntries() const;
    /*!
     * \brief 「(thisの名前).(追加の名前)」で公開されているimageのリストを返す。
     * \since ver1.6
     * \sa childrenRecurse()
     */
    template <WEBCFACE_COMPLETE(Image)>
    std::vector<Image_> imageEntries() const;
    /*!
     * \brief 「(thisの名前).(追加の名前)」で公開されているlogのリストを返す。
     * \since ver2.4
     * \sa childrenRecurse()
     */
    template <WEBCFACE_COMPLETE(Log)>
    std::vector<Log_> logEntries() const;

    /*!
     * \brief memberがselfならtrue
     *
     * data_wがlockできなければruntime_errorを投げる
     *
     */
    bool isSelf() const;

    bool operator==(const Field &other) const;
    bool operator!=(const Field &other) const { return !(*this == other); }
};

extern template std::vector<Value> Field::valueEntries<Value, true>() const;
extern template std::vector<Text> Field::textEntries<Text, true>() const;
extern template std::vector<RobotModel>
Field::robotModelEntries<RobotModel, true>() const;
extern template std::vector<Func> Field::funcEntries<Func, true>() const;
extern template std::vector<View> Field::viewEntries<View, true>() const;
extern template std::vector<Canvas2D>
Field::canvas2DEntries<Canvas2D, true>() const;
extern template std::vector<Canvas3D>
Field::canvas3DEntries<Canvas3D, true>() const;
extern template std::vector<Image> Field::imageEntries<Image, true>() const;
extern template std::vector<Log> Field::logEntries<Log, true>() const;

WEBCFACE_NS_END
