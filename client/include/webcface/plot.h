#pragma once
#include <vector>
#include <ostream>
#include <memory>
#include <utility>
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif
#include "components.h"

WEBCFACE_NS_BEGIN
namespace internal {
template <typename Component>
class DataSetBuffer;
} // namespace internal
namespace message {
struct PlotSeriesData;
}

class Value;

class PlotSeries {
    std::shared_ptr<message::PlotSeriesData> msg_data;
    std::weak_ptr<internal::ClientData> data_w;

    PlotSeries(const std::vector<Value> &values);

  public:
    PlotSeries() = default;
    PlotSeries(const Value &value_x, const Value &value_y);
    PlotSeries(const std::shared_ptr<message::PlotSeriesData> &msg_data,
               const std::weak_ptr<internal::ClientData> &data_w)
        : msg_data(msg_data), data_w(data_w) {}

    friend internal::DataSetBuffer<PlotSeries>;

    std::vector<Value> values() const;
    ViewColor color() const;
    PlotSeries &color(ViewColor color) &;
    PlotSeries &&color(ViewColor color) && {
        this->color(color);
        return std::move(*this);
    }
};

/*!
 * \brief Plotの送受信データを表すクラス
 * \since ver2.6
 *
 * コンストラクタではなく Member::plot() を使って取得してください
 *
 */
class WEBCFACE_DLL Plot : protected Field {
    std::shared_ptr<internal::DataSetBuffer<PlotSeries>> sb;

  public:
    Plot();
    Plot(const Field &base);
    Plot(const Field &base, const SharedString &field)
        : Plot(Field{base, field}) {}
    Plot(const Plot &rhs) : Plot() { *this = rhs; }
    Plot(Plot &&rhs) noexcept : Plot() { *this = std::move(rhs); }
    Plot &operator=(const Plot &rhs);
    Plot &operator=(Plot &&rhs) noexcept;
    ~Plot();

    friend internal::DataSetBuffer<PlotSeries>;

    using Field::lastName;
    using Field::member;
    using Field::name;
    using Field::nameW;
    /*!
     * \brief 「(thisの名前).(追加の名前)」を新しい名前とするField
     *
     */
    Plot child(std::string_view field) const {
        return this->Field::child(field);
    }
    /*!
     * \brief 「(thisの名前).(追加の名前)」を新しい名前とするField (wstring)
     *
     */
    Plot child(std::wstring_view field) const {
        return this->Field::child(field);
    }
    Plot child(int index) const { return this->Field::child(index); }
    /*!
     * child()と同じ
     *
     */
    Plot operator[](std::string_view field) const { return child(field); }
    /*!
     * child()と同じ
     *
     */
    Plot operator[](std::wstring_view field) const { return child(field); }
    /*!
     * operator[](long, const char *)と解釈されるのを防ぐための定義
     *
     */
    Plot operator[](const char *field) const { return child(field); }
    Plot operator[](const wchar_t *field) const { return child(field); }
    /*!
     * child()と同じ
     *
     */
    Plot operator[](int index) const { return child(index); }
    /*!
     * \brief nameの最後のピリオドの前までを新しい名前とするField
     *
     */
    Plot parent() const { return this->Field::parent(); }

    /*!
     * \brief 値が変化したときに呼び出されるコールバックを設定
     * \param callback Plot型の引数(thisが渡される)を1つ取る関数
     *
     */
    const Plot &
    onChange(std::function<void WEBCFACE_CALL_FP(Plot)> callback) const;
    /*!
     * \brief 値が変化したときに呼び出されるコールバックを設定
     * \param callback 引数をとらない関数
     *
     */
    template <typename F, typename std::enable_if_t<std::is_invocable_v<F>,
                                                    std::nullptr_t> = nullptr>
    const Plot &onChange(F callback) const {
        return onChange(
            [callback = std::move(callback)](const auto &) { callback(); });
    }

    /*!
     * \brief plotをリクエストする
     *
     */
    const Plot &request() const;
    /*!
     * \brief Plotの内容を取得する
     *
     */
    std::optional<std::vector<PlotSeries>> tryGet() const;
    /*!
     * \brief Plotの内容を取得する
     *
     */
    std::vector<PlotSeries> get() const {
        return tryGet().value_or(std::vector<PlotSeries>{});
    }
    /*!
     * \brief このフィールドにデータが存在すればtrue
     *
     * tryGet() などとは違って、実際のデータを受信しない。
     * リクエストも送信しない。
     *
     */
    bool exists() const;

    /*!
     * \brief 値やリクエスト状態をクリア
     *
     */
    const Plot &free() const;

    /*!
     * \brief このPlotの内容を初期化する
     *
     * このPlotオブジェクトに追加された内容をクリアし、内容を変更済みとしてマークする
     * (init() 後に sync() をするとPlotの内容が空になる)
     *
     */
    const Plot &init() const;
    /*!
     * \brief PlotSeriesを追加
     *
     */
    const Plot &operator<<(PlotSeries vc) const;

    /*!
     * \brief PlotSeriesを追加
     *
     */
    const Plot &add(PlotSeries rhs) const {
        *this << std::move(rhs);
        return *this;
    }

    /*!
     * \brief Plotの内容をclientに反映し送信可能にする
     *
     * * このPlotオブジェクトの内容が変更されていなければ
     * (init()も追加もされていなければ) 何もしない。
     *
     */
    const Plot &sync() const;

    /*!
     * \brief Plotの内容をまとめてセット
     *
     */
    const Plot &set(const std::vector<PlotSeries> &data) const {
        this->init();
        for (const auto &v : data) {
            *this << v;
        }
        this->sync();
        return *this;
    }

    /*!
     * \brief これをComponentに変換
     */
    TemporalComponent<false, true, true> toComponent() const;
    /*!
     * \brief これをCanvas2DComponentに変換
     */
    TemporalCanvas2DComponent toComponent2D() const;
    /*!
     * \brief これをCanvas3DComponentに変換
     */
    TemporalCanvas3DComponent toComponent3D() const;
    /*!
     * \brief Componentに変換 + 要素の移動
     *
     */
    TemporalComponent<false, true, true> origin(const Transform &origin) const {
        return toComponent().origin(origin);
    }
    /*!
     * \brief Componentに変換 + サイズを指定
     *
     */
    TemporalComponent<false, true, true> scale(double scale_x,
                                               double scale_y) const {
        return toComponent().scale(scale_x, scale_y);
    }

    /*!
     * \brief Plotの参照先を比較
     *
     */
    template <typename T, typename std::enable_if_t<std::is_same_v<T, Plot>,
                                                    std::nullptr_t> = nullptr>
    bool operator==(const T &other) const {
        return static_cast<Field>(*this) == static_cast<Field>(other);
    }
    template <typename T, typename std::enable_if_t<std::is_same_v<T, Plot>,
                                                    std::nullptr_t> = nullptr>
    bool operator!=(const T &other) const {
        return static_cast<Field>(*this) != static_cast<Field>(other);
    }
};
WEBCFACE_NS_END
