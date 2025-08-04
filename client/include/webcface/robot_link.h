#pragma once
#include "webcface/component_view.h"
#include "webcface/common/encoding.h"
#include "webcface/geometry.h"
#include "webcface/transform.h"
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif

WEBCFACE_NS_BEGIN
namespace internal {
struct TemporalRobotJointData;
struct RobotLinkData;
} // namespace internal

enum class RobotJointType {
    fixed_absolute = 0,
    fixed = 1,
    rotational = 2,
    prismatic = 3,
};
class RobotLink;
/*!
 * \brief modelの関節1つを表すクラス
 *
 * ver2.0からメンバ変数ではなくgetter関数に変更
 * 取得時にはjointの情報をshared_ptrで持っているので、
 * コピーしても同じjointを指す
 */
class WEBCFACE_DLL RobotJoint {
    /*!
     * Jointを構築するときの一時データ
     *
     */
    std::unique_ptr<internal::TemporalRobotJointData> temp_data;
    /*!
     * modelからjointのデータを読むときの参照
     *
     */
    std::shared_ptr<internal::RobotLinkData> msg_data;

  public:
    friend class RobotLink;

    RobotJoint();
    explicit RobotJoint(
        const std::shared_ptr<internal::RobotLinkData> &msg_data);
    RobotJoint(const SharedString &name, const SharedString &parent_name,
               RobotJointType type, const Transform &origin, double angle);
    ~RobotJoint();
    RobotJoint(const RobotJoint &other);
    RobotJoint &operator=(const RobotJoint &other);

    /*!
     * \brief jointの名前を取得
     *
     */
    const std::string &name() const;
    /*!
     * \brief jointの名前を取得 (wstring)
     * \since ver2.0
     */
    const std::wstring &nameW() const;
    /*!
     * \brief 親リンクを取得
     * \since ver2.0
     */
    std::optional<RobotLink> parent() const;
    /*!
     * \brief jointの種類
     *
     */
    RobotJointType type() const;
    /*!
     * \brief 親リンクの座標系で子リンクの原点
     *
     */
    Transform origin() const;
    /*!
     * \brief 初期状態の回転角
     *
     */
    double angle() const;
};

inline namespace robot_joints {
/*!
 * \brief 親リンクをもたず座標を定義する
 *
 * ベースのリンクなどに使う
 * \param origin 子リンクの絶対座標
 *
 */
inline RobotJoint fixedAbsolute(const Transform &origin) {
    return RobotJoint{nullptr, nullptr, RobotJointType::fixed_absolute, origin,
                      0};
}
/*!
 * \brief 親リンクをもたず座標を定義する
 *
 * ベースのリンクなどに使う
 * \param origin 子リンクの絶対座標
 *
 */
inline RobotJoint fixedAbsolute(const Point &origin) {
    return fixedAbsolute(Transform{origin, {}});
}
/*!
 * \brief 固定された関節
 * \param parent_name 親リンクの名前
 * \param origin 親リンクの座標系で子リンクの原点
 *
 * ver2.0〜 wstring対応, ver2.10〜 String 型で置き換え
 * 
 */
inline RobotJoint fixedJoint(const String &parent_name,
                             const Transform &origin) {
    return RobotJoint{nullptr, parent_name,
                      RobotJointType::fixed, origin, 0};
}
/*!
 * \brief 固定された関節
 * \param parent_name 親リンクの名前
 * \param origin 親リンクの座標系で子リンクの原点
 * 
 * ver2.0〜 wstring対応, ver2.10〜 String 型で置き換え
 * 
 */
inline RobotJoint fixedJoint(const String &parent_name,
                             const Point &origin) {
    return fixedJoint(parent_name, Transform{origin, {}});
}
/*!
 * \brief 回転関節
 *
 * originのz軸を中心に回転する関節。
 * \param name 関節の名前
 * \param parent_name 親リンクの名前
 * \param origin 親リンクの座標系で子リンクの原点
 * \param angle 初期状態の回転角
 *
 * ver2.0〜 wstring対応, ver2.10〜 String 型で置き換え
 * 
 */
inline RobotJoint rotationalJoint(const String &name,
                                  const String &parent_name,
                                  const Transform &origin, double angle = 0) {
    return RobotJoint{name,
                      parent_name,
                      RobotJointType::rotational, origin, angle};
}
/*!
 * \brief 直動関節
 *
 * originのz軸方向に直線運動する関節。
 * \param name 関節の名前
 * \param parent_name 親リンクの名前
 * \param origin 親リンクの座標系で子リンクの原点
 * \param angle 初期状態の回転角(移動距離)
 *
 * ver2.0〜 wstring対応, ver2.10〜 String 型で置き換え
 * 
 */
inline RobotJoint prismaticJoint(const String &name,
                                 const String &parent_name,
                                 const Transform &origin, double angle = 0) {
    return RobotJoint{name,
                      parent_name,
                      RobotJointType::prismatic, origin, angle};
}
} // namespace robot_joints

namespace RobotJoints = robot_joints; // 〜ver1.11

/*!
 * \brief リンク1つを表すクラス
 *
 * ver2.0〜 メンバ変数ではなくgetter関数に変更
 * リンクの情報をshared_ptrで持っているので、
 * コピーしても同じリンクを指す
 *
 */
class WEBCFACE_DLL RobotLink {
    std::shared_ptr<internal::RobotLinkData> msg_data;

  public:
    RobotLink();
    explicit RobotLink(
        const std::shared_ptr<internal::RobotLinkData> &msg_data);
    ~RobotLink();
    RobotLink(const SharedString &name, const RobotJoint &joint,
              const Geometry &geometry, ViewColor color);

    /*!
     * jointのparentを名前指定からid指定に変換
     * (message送信用)
     *
     */
    std::shared_ptr<internal::RobotLinkData>
    lockJoints(const std::vector<SharedString> &link_names) const;

    /*!
     * \param name リンクの名前
     * \param joint 親リンクとの接続方法
     * \param geometry リンクの形状 (表示用)
     * \param color 色 (表示用)
     *
     * ver2.0〜 wstring対応, ver2.10〜 String 型で置き換え
     * 
     */
    RobotLink(const String &name, const RobotJoint &joint,
              const Geometry &geometry, ViewColor color = ViewColor::inherit)
        : RobotLink(static_cast<const SharedString &>(name), joint, geometry, color) {}
    /*!
     * ベースのリンクではjointを省略可能
     * (fixedAbsolute({0, 0, 0})になる)
     * \param name リンクの名前
     * \param geometry リンクの形状 (表示用)
     * \param color 色 (表示用)
     *
     * ver2.0〜 wstring対応, ver2.10〜 String 型で置き換え
     * 
     */
    RobotLink(const String &name, const Geometry &geometry,
              ViewColor color = ViewColor::inherit)
        : RobotLink(static_cast<const SharedString &>(name), fixedAbsolute({0, 0, 0}), geometry, color) {}

    /*!
     * \brief 名前を取得
     *
     */
    const std::string &name() const;
    /*!
     * \brief 名前を取得 (wstring)
     * \since ver2.0
     */
    const std::wstring &nameW() const;
    /*!
     * \brief jointを取得
     *
     */
    RobotJoint joint() const;
    /*!
     * \brief geometryを取得
     *
     */
    std::optional<Geometry> geometry() const;
    /*!
     * \brief 色を取得
     *
     */
    ViewColor color() const;
};
WEBCFACE_NS_END
