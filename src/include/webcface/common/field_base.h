#pragma once
#include <string>
#include <webcface/common/def.h>
#include <webcface/encoding.h>

WEBCFACE_NS_BEGIN
inline namespace Common {
/*!
 * \brief メンバ名とデータ名を持つクラス
 *
 */
struct FieldBase {
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

struct FieldBaseComparable : public FieldBase {
    FieldBaseComparable() = default;
    FieldBaseComparable(const FieldBase &base) : FieldBase(base) {}

    bool operator==(const FieldBaseComparable &rhs) const {
        return this->member_ == rhs.member_ && this->field_ == rhs.field_;
    }
    bool operator<(const FieldBaseComparable &rhs) const {
        return this->member_ < rhs.member_ ||
               (this->member_ == rhs.member_ && this->field_ < rhs.field_);
    }

    struct Hash {
        SharedString::Hash h1, h2;
        Hash() = default;
        auto operator()(const FieldBaseComparable &base) const {
            return h1(base.member_) + h2(base.field_);
        }
    };
};
} // namespace Common
WEBCFACE_NS_END
