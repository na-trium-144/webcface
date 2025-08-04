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
     * \brief data_wをlockし、失敗したらSanityErrorを投げる
     *
     */
    std::shared_ptr<internal::ClientData> dataLock() const;
    /*!
     * \brief data_wをlockし、memberがselfではなければIntrusionを投げる
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
     * ver2.10〜 std::stringの参照から StringView に変更
     *
     */
    StringView name() const { return field_.decode(); }
    /*!
     * \brief field名を返す (wstring)
     * \since ver2.0
     *
     * ver2.10〜 std::wstringの参照から WStringView に変更
     *
     */
    WStringView nameW() const { return field_.decodeW(); }

  protected:
    SharedString lastName8() const;

  public:
    Field child(const SharedString &field) const;

    /*!
     * \brief nameのうちピリオドで区切られた最後の部分を取り出す
     * \since ver1.11
     */
    std::string lastName() const { return std::string(lastName8().decode()); }
    /*!
     * \brief nameのうちピリオドで区切られた最後の部分を取り出す (wstring)
     * \since ver2.0
     */
    std::wstring lastNameW() const {
        return std::wstring(lastName8().decodeW());
    }
    /*!
     * \brief nameの最後のピリオドの前までを新しい名前とするField
     * \since ver1.11
     */
    Field parent() const;
    /*!
     * \brief 「(thisの名前).(追加の名前)」を新しい名前とするField
     * \since ver1.11
     *
     * ver2.0〜 wstring対応, ver2.10〜 String 型で置き換え
     *
     */
    Field child(String field) const {
        return child(static_cast<SharedString &>(field));
    }
    /*!
     * \brief 「(thisの名前).(index)」を新しい名前とするField
     * \since ver1.11
     * \deprecated ver2.8〜
     */
    [[deprecated]]
    Field child(int index) const {
        return child(std::to_string(index));
    }
    /*!
     * \brief 「(thisの名前).(追加の名前)」を新しい名前とするField
     * \since ver1.11
     *
     * ver2.0〜 wstring対応, ver2.10〜 String 型で置き換え
     *
     */
    Field operator[](String field) const {
        return child(static_cast<SharedString &>(field));
    }
    /*!
     * \brief 「(thisの名前).(index)」を新しい名前とするField
     * \since ver1.11
     * \deprecated ver2.8〜
     */
    [[deprecated]]
    Field
    operator[](int index) const {
        return child(std::to_string(index));
    }

    template <WEBCFACE_COMPLETE(Value)>
    Value_ value(String field = String()) const {
        return child(static_cast<SharedString &>(field));
    }
    template <WEBCFACE_COMPLETE(Text)>
    Text_ text(String field = String()) const {
        return child(static_cast<SharedString &>(field));
    }
    template <WEBCFACE_COMPLETE(RobotModel)>
    RobotModel_ robotModel(String field = String()) const {
        return child(static_cast<SharedString &>(field));
    }
    template <WEBCFACE_COMPLETE(Image)>
    Image_ image(String field = String()) const {
        return child(static_cast<SharedString &>(field));
    }
    template <WEBCFACE_COMPLETE(Func)>
    Func_ func(String field = String()) const {
        return child(static_cast<SharedString &>(field));
    }
    template <WEBCFACE_COMPLETE(FuncListener)>
    FuncListener_ funcListener(String field = String()) const {
        return child(static_cast<SharedString &>(field));
    }
    template <WEBCFACE_COMPLETE(View)>
    View_ view(String field = String()) const {
        return child(static_cast<SharedString &>(field));
    }
    template <WEBCFACE_COMPLETE(Canvas3D)>
    Canvas3D_ canvas3D(String field = String()) const {
        return child(static_cast<SharedString &>(field));
    }
    template <WEBCFACE_COMPLETE(Canvas2D)>
    Canvas2D_ canvas2D(String field = String()) const {
        return child(static_cast<SharedString &>(field));
    }
    /*!
     * \since ver2.4
     */
    template <WEBCFACE_COMPLETE(Log)>
    Log_ log(String field = String()) const {
        return child(static_cast<SharedString &>(field));
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
