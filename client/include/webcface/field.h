#pragma once
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <webcface/common/def.h>
#include <webcface/encoding/encoding.h>

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
class FuncListener;
class RobotModel;
class Canvas2D;
class Canvas3D;

constexpr char8_t field_separator = '.';

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
    std::weak_ptr<Internal::ClientData> data_w;

    Field() = default;
    Field(const std::weak_ptr<Internal::ClientData> &data_w,
          const SharedString &member)
        : FieldBase(member), data_w(data_w) {}
    Field(const std::weak_ptr<Internal::ClientData> &data_w,
          const SharedString &member, const SharedString &field)
        : FieldBase(member, field), data_w(data_w) {}
    Field(const Field &base, const SharedString &field)
        : FieldBase(base, field), data_w(base.data_w) {}

    /*!
     * \brief data_wをlockし、失敗したらruntime_errorを投げる
     *
     */
    std::shared_ptr<Internal::ClientData> dataLock() const;
    /*!
     * \brief data_wをlockし、memberがselfではなければinvalid_argumentを投げる
     *
     */
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
    const std::string &name() const { return field_.decode(); }
    /*!
     * \brief field名を返す (wstring)
     * \since ver2.0
     */
    const std::wstring &nameW() const { return field_.decodeW(); }

  protected:
    std::u8string_view lastName8() const;

  public:
    Field child(const SharedString &field) const;

    /*!
     * \brief nameのうちピリオドで区切られた最後の部分を取り出す
     * \since ver1.11
     */
    std::string lastName() const { return Encoding::decode(lastName8()); }
    /*!
     * \brief nameのうちピリオドで区切られた最後の部分を取り出す (wstring)
     * \since ver2.0
     */
    std::wstring lastNameW() const { return Encoding::decodeW(lastName8()); }
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
        return child(SharedString(field));
    }
    /*!
     * \brief 「(thisの名前).(追加の名前)」を新しい名前とするField (wstring)
     * \since ver2.0
     */
    Field child(std::wstring_view field) const {
        return child(SharedString(field));
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

    Value value(std::string_view field = "") const;
    Value value(std::wstring_view field) const;
    Text text(std::string_view field = "") const;
    Text text(std::wstring_view field) const;
    RobotModel robotModel(std::string_view field = "") const;
    RobotModel robotModel(std::wstring_view field) const;
    Image image(std::string_view field = "") const;
    Image image(std::wstring_view field) const;
    Func func(std::string_view field = "") const;
    Func func(std::wstring_view field) const;
    FuncListener funcListener(std::string_view field) const;
    FuncListener funcListener(std::wstring_view field) const;
    View view(std::string_view field = "") const;
    View view(std::wstring_view field) const;
    Canvas3D canvas3D(std::string_view field = "") const;
    Canvas3D canvas3D(std::wstring_view field) const;
    Canvas2D canvas2D(std::string_view field = "") const;
    Canvas2D canvas2D(std::wstring_view field) const;


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
