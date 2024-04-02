#pragma once
#include "common/def.h"
#include "common/robot_model.h"
#include "field.h"
#include "event_target.h"
#include "canvas_data.h"

WEBCFACE_NS_BEGIN
namespace Internal {
struct ClientData;
template <typename Component>
class DataSetBuffer;
} // namespace Internal
class Member;

class RobotModel;
extern template class WEBCFACE_IMPORT EventTarget<RobotModel>;

/*!
 * \brief RobotModelの送受信データを表すクラス
 *
 * コンストラクタではなく Member::robotModel() を使って取得してください
 *
 * (1.9〜) Canvas3DComponentを継承しており origin(), color()
 * などを指定してCanvas3Dに追加することができる
 *
 */
class WEBCFACE_DLL RobotModel : protected Field,
                                public EventTarget<RobotModel>,
                                public Canvas3DComponent {
    std::shared_ptr<Internal::DataSetBuffer<RobotLink>> sb;

    void onAppend() const override;

  public:
    RobotModel();
    RobotModel(const Field &base);
    RobotModel(const Field &base, const std::string &field)
        : RobotModel(Field{base, field}) {}

    friend class Canvas3D;
    using Field::member;
    using Field::name;
    friend Internal::DataSetBuffer<RobotLink>;

    /*!
     * \brief モデルを初期化
     * \since ver1.9
     */
    RobotModel &init();
    /*!
     * \brief モデルにlinkを追加
     * \since ver1.9
     */
    RobotModel &operator<<(const RobotLink &rl);
    /*!
     * \brief モデルにlinkを追加
     * \since ver1.9
     */
    RobotModel &operator<<(RobotLink &&rl);
    /*!
     * \brief モデルにlinkを追加
     * \since ver1.9
     */
    template <typename T>
    RobotModel &add(T &&rl) {
        *this << std::forward<T>(rl);
        return *this;
    }
    /*!
     * \brief addで追加したモデルをセットする
     * \since ver1.9
     */
    RobotModel &sync();

    /*!
     * \brief モデルをセットする
     *
     */
    RobotModel &set(const std::vector<RobotLink> &v);
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
    void request() const;
    /*!
     * \brief モデルを返す
     *
     */
    std::optional<std::vector<RobotLink>> tryGet() const;
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
    [[deprecated]] std::chrono::system_clock::time_point time() const;

    /*!
     * \brief 値やリクエスト状態をクリア
     *
     */
    RobotModel &free();
};

WEBCFACE_NS_END
