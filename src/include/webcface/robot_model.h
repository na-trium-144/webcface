#pragma once
#include "common/def.h"
#include "common/robot_model.h"
#include "field.h"
#include "event_target.h"

namespace WEBCFACE_NS {
namespace Internal {
struct ClientData;
}
class Member;

/*!
 * \brief RobotModelの送受信データを表すクラス
 *
 * コンストラクタではなく Member::robotModel() を使って取得してください
 *
 */
class RobotModel : protected Field, public EventTarget<RobotModel> {
    WEBCFACE_DLL void onAppend() const override;

  public:
    RobotModel() = default;
    WEBCFACE_DLL RobotModel(const Field &base);
    WEBCFACE_DLL RobotModel(const Field &base, const std::string &field)
        : RobotModel(Field{base, field}) {}

    friend class Canvas3D;
    using Field::member;
    using Field::name;

    /*!
     * \brief モデルをセットする
     *
     */
    WEBCFACE_DLL RobotModel &set(const std::vector<RobotLink> &v);
    /*!
     * \brief モデルをセットする
     *
     */
    RobotModel &operator=(const std::vector<RobotLink> &v) {
        this->set(v);
        return *this;
    }

    /*!
     * \brief モデルを返す
     *
     */
    WEBCFACE_DLL std::optional<std::vector<RobotLink>> tryGet() const;
    /*!
     * \brief モデルを返す
     *
     */
    std::vector<RobotLink> get() const {
        return tryGet().value_or(std::vector<RobotLink>{});
    }
    operator std::vector<RobotLink>() const { return get(); }
    /*!
     * \brief syncの時刻を返す
     *
     */
    WEBCFACE_DLL std::chrono::system_clock::time_point time() const;

    /*!
     * \brief 値やリクエスト状態をクリア
     *
     */
    WEBCFACE_DLL RobotModel &free();
};

} // namespace WEBCFACE_NS