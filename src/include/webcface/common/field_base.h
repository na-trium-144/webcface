#pragma once
#include <string_view>
#include <stdexcept>
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
        : FieldBase(member, field) {}

  public:
    FieldBase() : FieldBase(nullptr, nullptr) {}

    bool operator==(const FieldBase &other) const {
        return this->member_ptr() == rhs.member_ptr() &&
               this->field_ptr() == rhs.field_ptr();
    }

    MemberNameRef member_ptr() const { return member_; }
    FieldNameRef field_ptr() const { return field_; }

    std::string_view member_sv() const {
        if (member_) {
            return std::string_view(static_cast<const char *>(member_));
        }
        throw std::runtime_error("member name is null");
    }
    std::string_view field_sv() const {
        if (field_) {
            return std::string_view(static_cast<const char *>(field_));
        }
        throw std::runtime_error("field name is null");
    }
};

struct FieldBaseComparable : public FieldBase {
    FieldBaseComparable() = default;
    FieldBaseComparable(const FieldBase &base) : FieldBase(base) {}

    bool operator==(const FieldBaseComparable &rhs) const {
        return this->member_ptr() == rhs.member_ptr() &&
               this->field_ptr() == rhs.field_ptr();
    }
    bool operator<(const FieldBaseComparable &rhs) const {
        return this->member_ptr() < rhs.member_ptr() ||
               (this->member_ptr() == rhs.member_ptr() &&
                this->field_ptr() < rhs.field_ptr());
    }
};
} // namespace Common
WEBCFACE_NS_END
