#pragma once
#include "common/def.h"
#include "common/robot_model.h"
#include "field.h"
#include "event_target.h"
#include "canvas_data.h"

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
 * (1.9〜) Canvas3DComponentを継承しており origin(), color()
 * などを指定してCanvas3Dに追加することができる
 *
 */
class RobotModel : protected Field,
                   public EventTarget<RobotModel>,
                   public Canvas3DComponent {
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
     * \brief モデルをリクエストする
     * \since ver1.7
     *
     */
    WEBCFACE_DLL void request() const;
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
     * \deprecated 1.7でMember::syncTime() に変更
     */
    [[deprecated]] WEBCFACE_DLL std::chrono::system_clock::time_point
    time() const;

    /*!
     * \brief 値やリクエスト状態をクリア
     *
     */
    WEBCFACE_DLL RobotModel &free();
};

} // namespace WEBCFACE_NS