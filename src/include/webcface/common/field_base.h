#pragma once
#include <string>
#include "def.h"

WEBCFACE_NS_BEGIN
inline namespace Common {


/*!
 * \brief メンバ名とデータ名を持つクラス
 *
 * (ver1.11〜) Fieldと分離、FieldBaseは従来と同様stringで保持するが、
 * 型をu8stringに変更
 *
 * fieldを使用しない場合空文字列
 *
 */
class FieldBase {
  protected:
    std::u8string member_;
    std::u8string field_;

  public:
    FieldBase() = default;
    explicit FieldBase(std::u8string &&member)
        : member_(std::move(member)), field_() {}
    explicit FieldBase(std::u8string &&member, std::u8string &&field)
        : member_(std::move(member)), field_(std::move(field)) {}

    bool operator==(const FieldBase &other) const {
        return this->member_ == other.member_ && this->field_ == other.field_;
    }

    const std::u8string &memberName() const { return member_; }
    const std::u8string &fieldName() const { return field_; }
};

} // namespace Common
WEBCFACE_NS_END
