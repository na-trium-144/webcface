#pragma once
#include <vector>
#include <sstream>
#include <ostream>
#include <memory>
#include <utility>
#include <stdexcept>
#include <concepts>
#include "func.h"
#include "event_target.h"
#include "common/def.h"
#include "common/canvas3d.h"
#include "common/robot_model.h"
#include "robot_model.h"
#include "canvas_data.h"

namespace WEBCFACE_NS {

/*!
 * \brief Canvas3Dの送受信データを表すクラス
 *
 * コンストラクタではなく Member::canvas3D() を使って取得してください
 *
 */
class Canvas3D : protected Field, public EventTarget<Canvas3D> {
    std::shared_ptr<std::vector<Canvas3DComponentBase>> components;
    std::shared_ptr<bool> modified;

    WEBCFACE_DLL void onAppend() const override;

    /*!
     * \brief 値をセットし、EventTargetを発動する
     *
     */
    WEBCFACE_DLL Canvas3D &set(std::vector<Canvas3DComponentBase> &v);

    WEBCFACE_DLL void onDestroy();

  public:
    WEBCFACE_DLL Canvas3D();
    WEBCFACE_DLL Canvas3D(const Field &base);
    Canvas3D(const Field &base, const std::string &field)
        : Canvas3D(Field{base, field}) {}

    /*!
     * \brief デストラクタで sync() を呼ぶ。
     *
     * Canvas3Dをコピーした場合は、すべてのコピーが破棄されたときにのみ sync()
     * が呼ばれる。
     * \sa sync()
     *
     */
    ~Canvas3D() override { onDestroy(); }

    using Field::member;
    using Field::name;

    /*!
     * \brief 子フィールドを返す
     *
     * \return「(thisのフィールド名).(子フィールド名)」をフィールド名とするCanvas3D
     *
     */
    Canvas3D child(const std::string &field) const {
        return Canvas3D{*this, this->field_ + "." + field};
    }
    /*!
     * \brief canvasの内容をリクエストする
     * \since ver1.7
     *
     */
    WEBCFACE_DLL void request() const;
    /*!
     * \brief Canvasの内容を取得する
     *
     */
    WEBCFACE_DLL std::optional<std::vector<Canvas3DComponent>> tryGet() const;
    /*!
     * \brief Canvasの内容を取得する
     *
     */
    std::vector<Canvas3DComponent> get() const {
        return tryGet().value_or(std::vector<Canvas3DComponent>{});
    }
    /*!
     * \brief syncの時刻を返す
     * \deprecated 1.7でMember::syncTime()に変更
     */
    [[deprecated]] WEBCFACE_DLL std::chrono::system_clock::time_point
    time() const;

    /*!
     * \brief 値やリクエスト状態をクリア
     *
     */
    WEBCFACE_DLL Canvas3D &free();

    /*!
     * \brief このCanvas3Dに追加した内容を初期化する
     *
     * このCanvas3Dオブジェクトに追加された内容をクリアし、
     * 内容を変更済みとしてマークする
     * (init() 後に sync() をすると空のCanvas3Dが送信される)
     *
     */
    WEBCFACE_DLL Canvas3D &init();

    /*!
     * \brief Componentを追加
     *
     */
    WEBCFACE_DLL Canvas3D &add(const Canvas3DComponentBase &cc);

    /*!
     * \brief Geometryを追加
     * \since 1.8
     */
    Canvas3D &add(CanvasCommonComponent &&cc) {
        add(static_cast<Canvas3DComponent &&>(std::move(cc)));
        return *this;
    }
    /*!
     * \brief Geometryを追加
     * \since 1.8
     */
    Canvas3D &add(CanvasCommonComponent &cc) {
        add(static_cast<Canvas3DComponent &>(cc));
        return *this;
    }
    /*!
     * \brief Geometryを追加
     * \param geometry 表示する図形
     * \param origin geometryを移動する
     * \param color 表示色 (省略時のinheritはWebUI上ではgrayと同じ)
     * \deprecated 1.8〜
     * CanvasCommonComponent に直接プロパティを設定できるようにしたため、
     * add時の引数での設定は不要
     *
     */
    [[deprecated]] Canvas3D &add(const Geometry &geometry,
                                 const Transform &origin,
                                 const ViewColor &color = ViewColor::inherit) {
        add({Canvas3DComponentType::geometry,
             origin,
             color,
             geometry,
             std::nullopt,
             {}});
        return *this;
    }
    /*!
     * \brief Geometryを追加
     *
     * originを省略した場合 identity() になる
     * \deprecated 1.8〜
     * CanvasCommonComponent に直接プロパティを設定できるようにしたため、
     * add時の引数での設定は不要
     *
     */
    [[deprecated]] Canvas3D &add(const Geometry &geometry,
                                 const ViewColor &color = ViewColor::inherit) {
        add({Canvas3DComponentType::geometry,
             identity(),
             color,
             geometry,
             std::nullopt,
             {}});
        return *this;
    }
    /*!
     * \brief RobotModelを追加
     *
     * jointのangleを変更できる。
     * それ以外のパラメータは元のモデルのまま。
     *
     */
    WEBCFACE_DLL Canvas3D &add(const RobotModel &model_field,
                               const Transform &origin,
                               std::unordered_map<std::string, double> angles) {
        std::unordered_map<std::size_t, double> angles_i;
        auto model = model_field.get();
        for (std::size_t ji = 0; ji < model.size(); ji++) {
            const auto &j = model[ji].joint;
            if (angles.count(j.name)) {
                angles_i[ji] = angles[j.name];
            }
        }
        add({Canvas3DComponentType::robot_model, origin, ViewColor::inherit,
             std::nullopt, static_cast<FieldBase>(model_field), angles_i});
        return *this;
    }

    /*!
     * \brief Viewの内容をclientに反映し送信可能にする
     *
     * このCanvas3Dオブジェクトの内容が変更されていなければ
     * (init()も追加もされていなければ) 何もしない。
     *
     */
    WEBCFACE_DLL Canvas3D &sync();
};
} // namespace WEBCFACE_NS
