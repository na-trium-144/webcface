#pragma once
#include "def.h"

WEBCFACE_NS_BEGIN
inline namespace Common {

using MemberNameRef = const void *;
using FieldNameRef = const void *;

/*!
 * \brief メンバ名とデータ名を持つクラス
 *
 * (ver1.11〜)メンバ名とデータ名は別の場所で保持した文字列(char配列)へのポインタとして持つ。
 *
 * fieldを使用しない場合nullptr
 *
 */
class FieldBase {
  protected:
    MemberNameRef member_;
    FieldNameRef field_;
    FieldBase(MemberNameRef member, FieldNameRef field)
        : member_(member), field_(field) {}

  public:
    FieldBase() : FieldBase(nullptr, nullptr) {}

    bool operator==(const FieldBase &other) const {
        return this->memberPtr() == other.memberPtr() &&
               this->fieldPtr() == other.fieldPtr();
    }
    bool memberValid() const { return memberPtr() != nullptr; }
    bool fieldValid() const {
        return memberPtr() != nullptr && fieldPtr() != nullptr;
    }

    MemberNameRef memberPtr() const { return member_; }
    FieldNameRef fieldPtr() const { return field_; }
};

struct FieldBaseComparable : public FieldBase {
    FieldBaseComparable() = default;
    FieldBaseComparable(const FieldBase &base) : FieldBase(base) {}

    bool operator==(const FieldBaseComparable &rhs) const {
        return this->memberPtr() == rhs.memberPtr() &&
               this->fieldPtr() == rhs.fieldPtr();
    }
    bool operator<(const FieldBaseComparable &rhs) const {
        return this->memberPtr() < rhs.memberPtr() ||
               (this->memberPtr() == rhs.memberPtr() &&
                this->fieldPtr() < rhs.fieldPtr());
    }
};
} // namespace Common
WEBCFACE_NS_END
