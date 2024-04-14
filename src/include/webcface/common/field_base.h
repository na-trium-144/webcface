#pragma once
#include <string>
#include "def.h"

WEBCFACE_NS_BEGIN
inline namespace Common {
/*!
 * \brief メンバ名とデータ名を持つクラス
 *
 */
struct FieldBase {
    std::string member_; //!< メンバー名

    /*!
     * \brief フィールド名
     *
     * Memberなどフィールド名が不要なクラスでは使用しない
     *
     */
    std::string field_;

    FieldBase() = default;
    FieldBase(const std::string &member, const std::string &field = "")
        : member_(member), field_(field) {}
    FieldBase(const FieldBase &base, const std::string &field)
        : FieldBase(base.member_, field) {}

    bool operator==(const FieldBase &rhs) const {
        return this->member_ == rhs.member_ && this->field_ == rhs.field_;
    }
    bool operator!=(const FieldBase &rhs) const { return !(*this == rhs); }
};

struct FieldBaseComparable : public FieldBase {
    FieldBaseComparable() = default;
    FieldBaseComparable(const FieldBase &base) : FieldBase(base) {}

    bool operator==(const FieldBaseComparable &rhs) const {
        return this->member_ == rhs.member_ && this->field_ == rhs.field_;
    }
    bool operator!=(const FieldBaseComparable &rhs) const {
        return !(*this == rhs);
    }
    bool operator<(const FieldBaseComparable &rhs) const {
        return this->member_ < rhs.member_ ||
               (this->member_ == rhs.member_ && this->field_ < rhs.field_);
    }
};
} // namespace Common
WEBCFACE_NS_END
