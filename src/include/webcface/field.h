#pragma once
#include <memory>
#include <string_view>
#include <string>
#include <string_view>
#include <vector>
#include "common/field_base.h"
#include "common/def.h"
#include "encoding.h"

WEBCFACE_NS_BEGIN

namespace Internal {
struct ClientData;
} // namespace Internal

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

constexpr char8_t field_separator = u8'.';

using MemberNameRef = Encoding::NameRef;
using FieldNameRef = Encoding::NameRef;

/*!
 * \brief メンバ名とデータ名を持つクラス
 *
 * (ver1.11〜)メンバ名とデータ名は別の場所で保持した文字列(char8_t配列)へのポインタとして持つ。
 *
 * fieldを使用しない場合nullptr
 *
 */
class WEBCFACE_DLL Field {
  protected:
    /*!
     * \brief ClientDataの参照
     *
     * ClientData内に保持するクラスもあるので循環参照を避けるためweak_ptr
     *
     */
    std::weak_ptr<Internal::ClientData> data_w;

    MemberNameRef member_;
    FieldNameRef field_;

    std::u8string_view memberRef() const;
    std::u8string_view nameRef() const;
    std::u8string_view lastNameRef() const;
    Field child(std::u8string_view field) const;

    static MemberNameRef
    getMemberRef(std::weak_ptr<Internal::ClientData> data_w,
                 std::u8string_view member);
    static FieldNameRef getFieldRef(std::weak_ptr<Internal::ClientData> data_w,
                                    std::u8string_view field);
    static std::shared_ptr<Internal::ClientData>
    dataLock(std::weak_ptr<Internal::ClientData> data_w);

  public:
    Field() : data_w(), member_(nullptr), field_(nullptr) {}
    Field(std::weak_ptr<Internal::ClientData> data_w, MemberNameRef member,
          FieldNameRef field = nullptr)
        : data_w(data_w), member_(member), field_(field) {}
    Field(std::weak_ptr<Internal::ClientData> data_w, std::u8string_view member)
        : Field(data_w, getMemberRef(data_w, member)) {}
    Field(std::weak_ptr<Internal::ClientData> data_w, std::u8string_view member,
          std::u8string_view field)
        : Field(data_w, getMemberRef(data_w, member),
                getFieldRef(data_w, field)) {}
    Field(const Field &base, std::u8string_view field)
        : Field(base.data_w, base.member_, getFieldRef(data_w, field)) {}
    Field(const Field &base, FieldNameRef field)
        : Field(base.data_w, base.member_, field) {}

    FieldBase toBase() const {
        return FieldBase{std::u8string(Encoding::getNameU8(member_)),
                         std::u8string(Encoding::getNameU8(field_))};
    }

    /*!
     * data_wをlockし、失敗したらruntime_errorを投げる
     *
     */
    std::shared_ptr<Internal::ClientData> dataLock() const {
        return dataLock(data_w);
    }
    /*!
     * data_wをlockし、memberがselfではなければinvalid_argumentを投げる
     *
     */
    std::shared_ptr<Internal::ClientData> setCheck() const;

    bool expired() const;

    /*!
     * \brief Memberを返す
     *
     */
    Member member() const;
    
    MemberNameRef memberPtr() const { return member_; }
    FieldNameRef namePtr() const { return field_; }

    /*!
     * \brief field名を返す
     *
     */
    std::string name() const { return Encoding::getName(nameRef()); }
    /*!
     * \brief field名を返す (wstring)
     * \since ver1.11
     */
    std::wstring nameW() const { return Encoding::getNameW(nameRef()); }

    /*!
     * \brief nameのうちピリオドで区切られた最後の部分を取り出す
     * \since ver1.11
     */
    std::string lastName() const { return Encoding::getName(lastNameRef()); }
    /*!
     * \brief nameのうちピリオドで区切られた最後の部分を取り出す
     * \since ver1.11
     */
    std::wstring lastNameW() const { return Encoding::getNameW(lastNameRef()); }
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
        return child(Encoding::initName(field));
    }
    /*!
     * \brief 「(thisの名前).(追加の名前)」を新しい名前とするField
     * \since ver1.11
     */
    Field child(std::wstring_view field) const {
        return child(Encoding::initNameW(field));
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
     * \brief 「(thisの名前).(追加の名前)」を新しい名前とするField
     * \since ver1.11
     */
    Field operator[](std::wstring_view field) const { return child(field); }
    /*!
     * \brief 「(thisの名前).(index)」を新しい名前とするField
     * \since ver1.11
     */
    Field operator[](int index) const { return child(index); }

    // 以下、クラスが定義されてないのでヘッダに書けない

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

struct FieldComparable : public Field {
    FieldComparable() = default;
    FieldComparable(const Field &base) : Field(base) {}
    FieldComparable(MemberNameRef member, FieldNameRef field) : Field() {
        member_ = member;
        field_ = field;
    }

    bool operator==(const FieldComparable &rhs) const {
        return this->member_ == rhs.member_ && this->field_ == rhs.field_;
    }
    bool operator<(const FieldComparable &rhs) const {
        return this->member_ < rhs.member_ ||
               (this->member_ == rhs.member_ && this->field_ < rhs.field_);
    }
};

WEBCFACE_NS_END
