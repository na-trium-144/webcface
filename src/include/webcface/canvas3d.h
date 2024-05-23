#pragma once
#include <vector>
#include <sstream>
#include <ostream>
#include <memory>
#include <utility>
#include <stdexcept>
#include <concepts>
#include "event_target.h"
#include <webcface/common/def.h>
#include "common/canvas3d.h"
#include "common/robot_model.h"
#include "robot_model.h"
#include "canvas_data.h"

WEBCFACE_NS_BEGIN
namespace Internal {
template <typename Component>
class DataSetBuffer;
}

class Canvas3D;
extern template class WEBCFACE_IMPORT EventTarget<Canvas3D>;

/*!
 * \brief Canvas3Dの送受信データを表すクラス
 *
 * コンストラクタではなく Member::canvas3D() を使って取得してください
 *
 */
class WEBCFACE_DLL Canvas3D : protected Field, public EventTarget<Canvas3D> {
    std::shared_ptr<Internal::DataSetBuffer<Canvas3DComponent>> sb;

    void onAppend() const override final;

  public:
    Canvas3D();
    Canvas3D(const Field &base);
    Canvas3D(const Field &base, std::u8string_view &field)
        : Canvas3D(Field{base, field}) {}

    friend Internal::DataSetBuffer<Canvas3DComponent>;
    using Field::lastName;
    using Field::member;
    using Field::name;
    /*!
     * \brief 「(thisの名前).(追加の名前)」を新しい名前とするField
     *
     */
    Canvas3D child(std::string_view field) const {
        return this->Field::child(field);
    }
    /*!
     * \brief 「(thisの名前).(追加の名前)」を新しい名前とするField (wstring)
     * \since ver1.12
     */
    Canvas3D child(std::wstring_view field) const {
        return this->Field::child(field);
    }
    /*!
     * \since ver1.11
     */
    Canvas3D child(int index) const { return this->Field::child(index); }
    /*!
     * child()と同じ
     * \since ver1.11
     */
    Canvas3D operator[](std::string_view field) const { return child(field); }
    /*!
     * child()と同じ
     * \since ver1.12
     */
    Canvas3D operator[](std::wstring_view field) const { return child(field); }
    /*!
     * operator[](long, const char *)と解釈されるのを防ぐための定義
     * \since ver1.11
     */
    Canvas3D operator[](const char *field) const { return child(field); }
    /*!
     * \since ver1.12
     */
    Canvas3D operator[](const wchar_t *field) const { return child(field); }
    /*!
     * child()と同じ
     * \since ver1.11
     */
    Canvas3D operator[](int index) const { return child(index); }
    /*!
     * \brief nameの最後のピリオドの前までを新しい名前とするField
     * \since ver1.11
     */
    Canvas3D parent() const { return this->Field::parent(); }

    /*!
     * \brief canvasの内容をリクエストする
     * \since ver1.7
     *
     */
    void request() const;
    /*!
     * \brief Canvasの内容を取得する
     *
     */
    std::optional<std::vector<Canvas3DComponent>> tryGet() const;
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
    [[deprecated]] std::chrono::system_clock::time_point time() const;

    /*!
     * \brief 値やリクエスト状態をクリア
     *
     */
    Canvas3D &free();

    /*!
     * \brief このCanvas3Dに追加した内容を初期化する
     *
     * このCanvas3Dオブジェクトに追加された内容をクリアし、
     * 内容を変更済みとしてマークする
     * (init() 後に sync() をすると空のCanvas3Dが送信される)
     *
     */
    Canvas3D &init();

    /*!
     * \brief Componentを追加
     * \since ver1.9
     */
    Canvas3D &operator<<(const Canvas3DComponent &cc);
    /*!
     * \brief Componentを追加
     * \since ver1.9
     */
    Canvas3D &operator<<(Canvas3DComponent &&cc);

    /*!
     * \brief コンポーネントなどを追加
     *
     * Tの型に応じた operator<< が呼ばれる
     *
     * \since ver1.9〜
     *
     */
    template <typename T>
    Canvas3D &add(T &&cc) {
        *this << std::forward<T>(cc);
        return *this;
    }

    /*!
     * \brief Geometryを追加
     * \since ver1.9
     */
    template <bool V, bool C2>
    Canvas3D &operator<<(TemporalComponent<V, C2, true> &&cc) {
        *this << std::move(cc.to3());
        return *this;
    }
    /*!
     * \brief Geometryを追加
     * \since ver1.9
     */
    template <bool V, bool C2>
    Canvas3D &operator<<(TemporalComponent<V, C2, true> &cc) {
        *this << cc.to3();
        return *this;
    }
    /*!
     * \brief Geometryを追加
     * \param geometry 表示する図形
     * \param origin geometryを移動する
     * \param color 表示色 (省略時のinheritはWebUI上ではgrayと同じ)
     * \deprecated 1.9〜
     * TemporalComponent に直接プロパティを設定できるようにしたため、
     * add時の引数での設定は不要
     *
     */
    [[deprecated]] Canvas3D &add(const Geometry &geometry,
                                 const Transform &origin,
                                 const ViewColor &color = ViewColor::inherit) {
        add(Canvas3DComponent{{Canvas3DComponentType::geometry,
                               origin,
                               color,
                               geometry,
                               std::nullopt,
                               {}}});
        return *this;
    }
    /*!
     * \brief Geometryを追加
     *
     * originを省略した場合 identity() になる
     * \deprecated 1.9〜
     * TemporalComponent に直接プロパティを設定できるようにしたため、
     * add時の引数での設定は不要
     *
     */
    [[deprecated]] Canvas3D &add(const Geometry &geometry,
                                 const ViewColor &color = ViewColor::inherit) {
        add(Canvas3DComponent{{Canvas3DComponentType::geometry,
                               identity(),
                               color,
                               geometry,
                               std::nullopt,
                               {}}});
        return *this;
    }
    /*!
     * \brief RobotModelを追加
     *
     * jointのangleを変更できる。
     * それ以外のパラメータは元のモデルのまま。
     * \deprecated 1.9〜
     * RobotModel に直接プロパティを設定できるようにしたため、
     * add時の引数での設定は不要
     *
     */
    [[deprecated]] Canvas3D &
    add(const RobotModel &model_field, const Transform &origin,
        std::unordered_map<std::string, double> angles) {
        std::unordered_map<std::size_t, double> angles_i;
        auto model = model_field.get();
        for (std::size_t ji = 0; ji < model.size(); ji++) {
            const auto &j = model[ji].joint;
            auto j_name_s = Encoding::decode(j.name);
            if (angles.count(j_name_s)) {
                angles_i[ji] = angles[j_name_s];
            }
        }
        add(Canvas3DComponent{{Canvas3DComponentType::robot_model, origin,
                               ViewColor::inherit, std::nullopt,
                               static_cast<FieldBase>(model_field), angles_i}});
        return *this;
    }

    /*!
     * \brief Viewの内容をclientに反映し送信可能にする
     *
     * このCanvas3Dオブジェクトの内容が変更されていなければ
     * (init()も追加もされていなければ) 何もしない。
     *
     */
    Canvas3D &sync();

    /*!
     * \brief Canvas3Dの参照先を比較
     * \since ver1.11
     *
     */
    template <typename T>
        requires std::same_as<T, Canvas3D>
    bool operator==(const T &other) const {
        return static_cast<Field>(*this) == static_cast<Field>(other);
    }
};
WEBCFACE_NS_END
