#pragma once
#include <unordered_map>
#include "webcface/component_view.h"
#include "webcface/geometry.h"
#include "webcface/transform.h"
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif

WEBCFACE_NS_BEGIN
namespace message {
struct Canvas3DComponentData;
}
namespace internal {
struct TemporalCanvas3DComponentData;
}

enum class Canvas3DComponentType {
    geometry = 0,
    robot_model = 1,
    // scatter = 2,
};

/*!
 * \brief Canvas3Dに表示する要素
 *
 * * ver2.0〜:
 * get専用(Canvas3DComponent)とset用(TemporalComponent)で分けている。
 *
 */
class WEBCFACE_DLL Canvas3DComponent {
    std::shared_ptr<const message::Canvas3DComponentData> msg_data;
    std::weak_ptr<internal::ClientData> data_w;
    SharedString id_;

    void checkData() const;

  public:
    /*!
     * msg_dataはnullptrになり、内容にアクセスしようとするとruntime_errorを投げる
     *
     */
    Canvas3DComponent();

    Canvas3DComponent(
        const std::shared_ptr<const message::Canvas3DComponentData> &msg_data,
        const std::weak_ptr<internal::ClientData> &data_w,
        const SharedString &id);

    /*!
     * \brief そのcanvas3d内で一意のid
     * \since ver2.0
     *
     * * 要素が増減したり順序が変わったりしなければ、
     * 同じ要素には常に同じidが振られる。
     * * (ver2.5〜) canvas3d作成側でidを指定した場合その値が返る。
     *
     */
    std::string id() const;
    /*!
     * \brief そのcanvas3d内で一意のid (wstring)
     * \since ver2.5
     *
     * * 要素が増減したり順序が変わったりしなければ、
     * 同じ要素には常に同じidが振られる。
     * * canvas3d作成側でidを指定した場合その値が返る。
     *
     */
    std::wstring idW() const;

    /*!
     * \since ver1.11
     */
    bool operator==(const Canvas3DComponent &other) const;
    /*!
     * \since ver1.11
     */
    bool operator!=(const Canvas3DComponent &other) const {
        return !(*this == other);
    }

    /*!
     * \brief 要素の種類
     *
     */
    Canvas3DComponentType type() const;
    /*!
     * \brief 要素の移動
     *
     */
    Transform origin() const;
    /*!
     * \brief 色
     *
     */
    ViewColor color() const;
    /*!
     * \brief geometryを取得
     *
     */
    std::optional<Geometry> geometry() const;
    /*!
     * \brief RobotModelを取得
     *
     */
    template <WEBCFACE_COMPLETE(RobotModel)>
    std::optional<RobotModel_> robotModel() const;
};
extern template std::optional<RobotModel>
Canvas3DComponent::robotModel<RobotModel, true>() const;

/*!
 * \brief Canvas3Dを構築するときに使う一時的なCanvas3DComponent
 * \since ver2.0
 *
 */
class WEBCFACE_DLL TemporalCanvas3DComponent {
    std::unique_ptr<internal::TemporalCanvas3DComponentData> msg_data;

  public:
    /*!
     * msg_dataはnullptrになる
     *
     */
    explicit TemporalCanvas3DComponent(std::nullptr_t = nullptr);
    /*!
     * msg_dataを初期化する
     *
     */
    explicit TemporalCanvas3DComponent(Canvas3DComponentType type);
    TemporalCanvas3DComponent(const TemporalCanvas3DComponent &other);
    TemporalCanvas3DComponent &
    operator=(const TemporalCanvas3DComponent &other);
    ~TemporalCanvas3DComponent() noexcept;

    /*!
     * \brief AnonymousFuncの名前を確定
     *
     * 現状Canvas3DにFuncを使う要素はないのでなにもしない
     *
     * \param data
     * \param view_name viewの名前
     * \param idx_next 種類ごとの要素数のmap
     * InputRefの名前に使うidを決定するのに使う
     *
     */
    std::unique_ptr<internal::TemporalCanvas3DComponentData>
    lockTmp(const std::shared_ptr<internal::ClientData> &data,
            const SharedString &view_name,
            std::unordered_map<Canvas3DComponentType, int> *idx_next = nullptr);

    /*!
     * \brief idを設定
     * \since ver2.5
     */
    TemporalCanvas3DComponent &id(std::string_view id);
    /*!
     * \brief idを設定 (wstring)
     * \since ver2.5
     */
    TemporalCanvas3DComponent &id(std::wstring_view id);
    /*!
     * \brief 要素の移動
     *
     */
    TemporalCanvas3DComponent &origin(const Transform &origin) &;
    /*!
     * \since ver2.5
     */
    TemporalCanvas3DComponent &&origin(const Transform &origin) && {
        this->origin(origin);
        return std::move(*this);
    }
    /*!
     * \brief 色
     *
     */
    TemporalCanvas3DComponent &color(ViewColor color) &;
    /*!
     * \since ver2.5
     */
    TemporalCanvas3DComponent &&color(ViewColor color) && {
        this->color(color);
        return std::move(*this);
    }
    /*!
     * \brief geometryをセット
     *
     */
    TemporalCanvas3DComponent &geometry(const Geometry &g) &;
    /*!
     * \since ver2.5
     */
    TemporalCanvas3DComponent &&geometry(const Geometry &g) && {
        this->geometry(g);
        return std::move(*this);
    }

    TemporalCanvas3DComponent &robotModel(const RobotModel &field) &;
    /*!
     * \since ver2.5
     */
    TemporalCanvas3DComponent &&robotModel(const RobotModel &field) && {
        this->robotModel(field);
        return std::move(*this);
    }
    /*!
     * \brief RobotModelの関節をまとめて設定
     * \param angles RobotJointの名前と角度のリスト
     *
     */
    TemporalCanvas3DComponent &
    angles(const std::unordered_map<std::string, double> &angles) &;
    /*!
     * \since ver2.5
     */
    TemporalCanvas3DComponent &&
    angles(const std::unordered_map<std::string, double> &angles) && {
        this->angles(angles);
        return std::move(*this);
    }

    /*!
     * \brief RobotModelの関節をまとめて設定 (wstring)
     * \since ver2.0
     * \param angles RobotJointの名前と角度のリスト
     *
     */
    TemporalCanvas3DComponent &
    angles(const std::unordered_map<std::wstring, double> &angles) &;
    /*!
     * \since ver2.5
     */
    TemporalCanvas3DComponent &&
    angles(const std::unordered_map<std::wstring, double> &angles) && {
        this->angles(angles);
        return std::move(*this);
    }

    /*!
     * \brief RobotModelの関節を設定
     * \param joint_name RobotJointの名前
     * \param angle 角度
     *
     */
    TemporalCanvas3DComponent &angle(const std::string &joint_name,
                                     double angle) &;
    /*!
     * \since ver2.5
     */
    TemporalCanvas3DComponent &&angle(const std::string &joint_name,
                                      double angle) && {
        this->angle(joint_name, angle);
        return std::move(*this);
    }
    /*!
     * \brief RobotModelの関節を設定 (wstring)
     * \since ver2.0
     * \param joint_name RobotJointの名前
     * \param angle 角度
     *
     */
    TemporalCanvas3DComponent &angle(const std::wstring &joint_name,
                                     double angle) &;
    /*!
     * \since ver2.5
     */
    TemporalCanvas3DComponent &&angle(const std::wstring &joint_name,
                                      double angle) && {
        this->angle(joint_name, angle);
        return std::move(*this);
    }
};

WEBCFACE_NS_END
