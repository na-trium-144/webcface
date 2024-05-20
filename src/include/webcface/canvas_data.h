#pragma once
#include "common/canvas2d.h"
#include "common/canvas3d.h"
#include "func.h"
#include "text.h"
#include "webcface/common/view.h"
#include <memory>

WEBCFACE_NS_BEGIN
namespace Internal {
struct ClientData;
}
class RobotModel;

template <typename TypeEnum>
class IdBase {
  protected:
    int idx_for_type_ = 0;
    void initIdx(std::unordered_map<int, int> *idx_next, TypeEnum type) {
        if (idx_next) {
            idx_for_type_ = (*idx_next)[static_cast<int>(type)]++;
        }
    }

  public:
    IdBase() = default;
    virtual ~IdBase() = default;
    virtual TypeEnum type() const = 0;
    /*!
     * \brief そのview(またはcanvas)内で一意のid
     * \since ver1.10
     *
     * 要素が増減したり順序が変わったりしなければ、
     * 同じ要素には常に同じidが振られる。
     *
     */
    std::string id() const {
        return ".." + std::to_string(static_cast<int>(type())) + "." +
               std::to_string(idx_for_type_);
    }
};

/*!
 * \brief Viewに表示する要素です
 *
 */
class WEBCFACE_DLL ViewComponent : protected Common::ViewComponentBase,
                                   public IdBase<ViewComponentType> {
    std::weak_ptr<Internal::ClientData> data_w;

    std::shared_ptr<AnonymousFunc> on_click_func_tmp;
    std::optional<InputRef> text_ref_tmp;
    std::optional<ValAdaptor> init_;

  public:
    ViewComponent() = default;
    ViewComponent(const Common::ViewComponentBase &vc,
                  const std::weak_ptr<Internal::ClientData> &data_w,
                  std::unordered_map<int, int> *idx_next)
        : Common::ViewComponentBase(vc), IdBase<ViewComponentType>(),
          data_w(data_w) {
        initIdx(idx_next, type_);
    }
    explicit ViewComponent(ViewComponentType type)
        : Common::ViewComponentBase(), IdBase<ViewComponentType>() {
        type_ = type;
    }

    /*!
     * \brief AnonymousFuncとInputRefの名前を確定
     *
     */
    ViewComponentBase &
    lockTmp(const std::weak_ptr<Internal::ClientData> &data_w,
            const std::string &view_name,
            std::unordered_map<int, int> *idx_next = nullptr);

    wcfViewComponent cData() const;

    /*!
     * \brief 要素の比較
     * \since ver1.11
     *
     * 要素のプロパティが完全一致すればtrue
     *
     * 例外としてbindの中身は別のTextオブジェクトで管理されるので、
     * ここでは比較の対象ではない
     *
     */
    bool operator==(const ViewComponent &other) const {
        return id() == other.id() && static_cast<ViewComponentBase>(*this) ==
                                         static_cast<ViewComponentBase>(other);
    }
    /*!
     * \since ver1.11
     */
    bool operator!=(const ViewComponent &other) const {
        return !(*this == other);
    }

    /*!
     * \brief 要素の種類
     *
     */
    ViewComponentType type() const override { return type_; }
    /*!
     * \brief 表示する文字列を取得
     *
     */
    const std::string &text() const { return text_; }
    /*!
     * \brief 表示する文字列を設定
     *
     */
    ViewComponent &text(const std::string &text) {
        text_ = text;
        return *this;
    }
    /*!
     * \brief クリック時に実行される関数を取得
     *
     */
    std::optional<Func> onClick() const;
    /*!
     * \brief クリック時に実行される関数を設定
     * \param func 実行する関数を指すFuncオブジェクト
     *
     */
    ViewComponent &onClick(const Func &func);
    /*!
     * \brief クリック時に実行される関数を設定
     * \param func 実行する任意の関数(std::functionにキャスト可能ならなんでもok)
     *
     */
    template <typename T>
    ViewComponent &onClick(T func) {
        on_click_func_tmp = std::make_shared<AnonymousFunc>(func);
        return *this;
    }
    /*!
     * \brief クリック時に実行される関数を設定
     * \param func Client::func() で得られるAnonymousFuncオブジェクト
     * (ver1.9からコピー不可なので、一時オブジェクトでない場合はmoveすること)
     *
     */
    ViewComponent &onClick(AnonymousFunc &&func) {
        on_click_func_tmp = std::make_shared<AnonymousFunc>(std::move(func));
        return *this;
    }
    /*!
     * \brief inputの値の変化時に実行される関数を取得
     * \since ver1.10
     *
     * onChange()に新しい値を渡して呼び出すことで値を変更させる
     * (onChange()->runAsync(value) など)
     *
     * 内部データはonClickと共通
     *
     */
    std::optional<Func> onChange() const { return onClick(); }
    /*!
     * \brief 変更した値を格納するInputRefを設定
     * \since ver1.10
     *
     * refの値を変更する処理が自動的にonChangeに登録される
     *
     */
    ViewComponent &bind(const InputRef &ref) {
        on_click_func_tmp = std::make_shared<AnonymousFunc>(
            [ref](const ValAdaptor &val) { ref.lockedField().set(val); });
        text_ref_tmp = ref;
        return *this;
    }
    /*!
     * \brief inputの現在の値を取得
     * \since ver1.10
     *
     *     * 値の変更はonChange()に新しい値を渡して呼び出す
     * (onChange()->runAsync(value) など)
     *
     */
    std::optional<Text> bind() const;
    /*!
     * \brief 値が変化した時に実行される関数を設定
     * \since ver1.10
     *
     * 現在値を保持するInputRefは自動で生成されbindされる
     * \param func
     * 引数を1つ取る任意の関数(std::functionにキャスト可能ならなんでもok)
     *
     */
    template <typename T>
    ViewComponent &onChange(T func) {
        InputRef ref;
        on_click_func_tmp =
            std::make_shared<AnonymousFunc>([ref, func](ValAdaptor val) {
                ref.lockedField().set(val);
                return func(val);
            });
        text_ref_tmp = ref;
        return *this;
    }
    // /*!
    //  * \brief 値が変化した時に実行される関数を設定
    //  * \since ver1.10
    //  *
    //  */
    // ViewComponent &onChange(AnonymousFunc &&func) {
    //     InputRef ref;
    //     auto func_impl = func.getImpl();
    //     func.replaceImpl([ref, func_impl](const std::vector<ValAdaptor>
    //     &args) {
    //         if (args.size() >= 1) {
    //             ref.lockedField().set(args[0]);
    //         }
    //         return func_impl(args);
    //     });
    //     on_click_func_tmp = std::make_shared<AnonymousFunc>(std::move(func));
    //     text_ref_tmp = ref;
    //     return *this;
    // }
    /*!
     * \brief 文字色を取得
     *
     */
    ViewColor textColor() const { return text_color_; }
    /*!
     * \brief 文字色を設定
     *
     */
    ViewComponent &textColor(ViewColor c) {
        text_color_ = c;
        return *this;
    }
    /*!
     * \brief 背景色を取得
     *
     */
    ViewColor bgColor() const { return bg_color_; }
    /*!
     * \brief 背景色を設定
     *
     */
    ViewComponent &bgColor(ViewColor c) {
        bg_color_ = c;
        return *this;
    }
    /*!
     * \brief デフォルト値を設定する。
     * \since ver1.10
     *
     * デフォルト値はviewのメッセージには含まれるのではなく、
     * bindしたInputRefの初期化に使われる
     * (そのため component.init() では取得できない)
     *
     */
    template <typename T>
        requires std::constructible_from<ValAdaptor, T>
    ViewComponent &init(const T &init) {
        init_.emplace(init);
        return *this;
    }
    /*!
     * \brief 最小値を取得する。
     * \since ver1.10
     *
     */
    std::optional<double> min() const { return min_; }
    /*!
     * \brief 最小値を設定する。
     * \since ver1.10
     *
     * * 文字列入力の場合最小の文字数を表す。
     * * 文字列・数値入力でない場合効果がない。
     * * option() はクリアされる。
     *
     */
    ViewComponent &min(double min) {
        min_ = min;
        option_.clear();
        return *this;
    }
    /*!
     * \brief 最大値を取得する。
     * \since ver1.10
     *
     */
    std::optional<double> max() const { return max_; }
    /*!
     * \brief 最大値を設定する。
     * \since ver1.10
     *
     * * 文字列入力の場合最大の文字数を表す。
     * * 文字列・数値入力でない場合効果がない。
     * * option() はクリアされる。
     *
     */
    ViewComponent &max(double max) {
        max_ = max;
        option_.clear();
        return *this;
    }
    /*!
     * \brief 数値の刻み幅を取得する。
     * \since ver1.10
     *
     */
    std::optional<double> step() const { return step_; }
    /*!
     * \brief 数値の刻み幅を設定する。
     * \since ver1.10
     *
     * * 整数入力、スライダーなど以外効果がない。
     *
     */
    ViewComponent &step(double step) {
        step_ = step;
        return *this;
    }
    /*!
     * \brief 引数の選択肢を取得する。
     * \since ver1.10
     *
     */
    std::vector<ValAdaptor> option() const { return option_; }
    ViewComponent &option(const std::vector<ValAdaptor> &option) {
        option_ = option;
        min_ = max_ = std::nullopt;
        return *this;
    }
    /*!
     * \brief 引数の選択肢を設定する。
     * \since ver1.10
     *
     * * min(), max() はクリアされる。
     *
     */
    template <typename T>
        requires std::constructible_from<ValAdaptor, T>
    ViewComponent &option(std::initializer_list<T> option) {
        return this->option(
            std::vector<ValAdaptor>(option.begin(), option.end()));
    }
};

/*!
 * \brief Canvas3Dに表示する要素
 *
 */
class WEBCFACE_DLL Canvas3DComponent : protected Common::Canvas3DComponentBase {
    std::weak_ptr<Internal::ClientData> data_w;

  public:
    Canvas3DComponent() = default;
    Canvas3DComponent(const Common::Canvas3DComponentBase &vc,
                      const std::weak_ptr<Internal::ClientData> &data_w)
        : Common::Canvas3DComponentBase(vc), data_w(data_w) {}
    explicit Canvas3DComponent(const Common::Canvas3DComponentBase &vc)
        : Common::Canvas3DComponentBase(vc), data_w() {}
    Canvas3DComponent(Canvas3DComponentType type,
                      const std::weak_ptr<Internal::ClientData> &data_w)
        : Common::Canvas3DComponentBase(), data_w(data_w) {
        type_ = type;
    }
    explicit Canvas3DComponent(Canvas3DComponentType type)
        : Common::Canvas3DComponentBase(), data_w() {
        type_ = type;
    }

    /*!
     * \brief AnonymousFuncをFuncオブジェクトにlock
     *
     * 現状Funcをセットする要素無いのでなにもしない
     *
     */
    Canvas3DComponentBase &
    lockTmp(const std::weak_ptr<Internal::ClientData> & /*data_w*/,
            const std::string & /*field_id*/) {
        return *this;
    }

    /*!
     * \since ver1.11
     */
    bool operator==(const Canvas3DComponent &other) const {
        return /*id() == other.id() && */ static_cast<Canvas3DComponentBase>(
                   *this) == static_cast<Canvas3DComponentBase>(other);
    }
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
    Canvas3DComponentType type() const { return type_; }
    /*!
     * \brief 要素の移動
     *
     */
    Transform origin() const { return origin_; }
    /*!
     * \brief 要素の移動
     *
     */
    Canvas3DComponent &origin(const Transform &origin) {
        origin_ = origin;
        return *this;
    }
    /*!
     * \brief 色
     *
     */
    ViewColor color() const { return color_; }
    /*!
     * \brief 色
     *
     */
    Canvas3DComponent &color(ViewColor color) {
        color_ = color;
        return *this;
    }
    /*!
     * \brief geometryを取得
     *
     */
    const std::optional<Geometry> &geometry() const { return geometry_; };
    /*!
     * \brief geometryをセット
     *
     */
    Canvas3DComponent &geometry(const Geometry &g) {
        geometry_.emplace(g);
        return *this;
    };
    /*!
     * \brief RobotModelを取得
     *
     */
    std::optional<RobotModel> robotModel() const;
    Canvas3DComponent &robotModel(const RobotModel &field);
    /*!
     * \brief RobotModelの関節をまとめて設定
     * \param angles RobotJointの名前と角度のリスト
     *
     */
    Canvas3DComponent &
    angles(const std::unordered_map<std::string, double> &angles);
    /*!
     * \brief RobotModelの関節を設定
     * \param joint_name RobotJointの名前
     * \param angle 角度
     *
     */
    Canvas3DComponent &angle(const std::string &joint_name, double angle);
};

/*!
 * \brief Canvas2Dの各要素を表すクラス。
 *
 * Geometryをセットするときは TemporalComponent を使うので、
 * 今のところこのクラスのオブジェクトのデータを変更する用途はない。
 *
 */
class WEBCFACE_DLL Canvas2DComponent : protected Common::Canvas2DComponentBase,
                                       public IdBase<Canvas2DComponentType> {
    std::weak_ptr<Internal::ClientData> data_w;
    std::shared_ptr<AnonymousFunc> on_click_func_tmp;

  public:
    Canvas2DComponent() = default;
    Canvas2DComponent(const Common::Canvas2DComponentBase &vc,
                      const std::weak_ptr<Internal::ClientData> &data_w,
                      std::unordered_map<int, int> *idx_next)
        : Common::Canvas2DComponentBase(vc), IdBase<Canvas2DComponentType>(),
          data_w(data_w), on_click_func_tmp(nullptr) {
        initIdx(idx_next, type_);
    }
    explicit Canvas2DComponent(const Common::Canvas2DComponentBase &vc)
        : Common::Canvas2DComponentBase(vc), IdBase<Canvas2DComponentType>(),
          data_w(), on_click_func_tmp(nullptr) {}
    Canvas2DComponent(Canvas2DComponentType type,
                      const std::weak_ptr<Internal::ClientData> &data_w)
        : Common::Canvas2DComponentBase(), IdBase<Canvas2DComponentType>(),
          data_w(data_w), on_click_func_tmp(nullptr) {
        type_ = type;
    }
    explicit Canvas2DComponent(Canvas2DComponentType type)
        : Common::Canvas2DComponentBase(), IdBase<Canvas2DComponentType>(),
          data_w(), on_click_func_tmp(nullptr) {
        type_ = type;
    }

    /*!
     * \brief AnonymousFuncの名前を確定
     *
     */
    Canvas2DComponentBase &
    lockTmp(const std::weak_ptr<Internal::ClientData> &data_w,
            const std::string &view_name,
            std::unordered_map<int, int> *idx_next = nullptr);

    /*!
     * \since ver1.11
     */
    bool operator==(const Canvas2DComponent &other) const {
        return id() == other.id() &&
               static_cast<Canvas2DComponentBase>(*this) ==
                   static_cast<Canvas2DComponentBase>(other);
    }
    /*!
     * \since ver1.11
     */
    bool operator!=(const Canvas2DComponent &other) const {
        return !(*this == other);
    }

    /*!
     * \brief 要素の種類
     *
     */
    Canvas2DComponentType type() const override { return type_; }
    /*!
     * \brief 要素の移動
     *
     * 要素を平行移動&回転します。
     *
     */
    Canvas2DComponent &origin(const Transform &origin) {
        origin_ = origin;
        return *this;
    }
    Transform origin() const { return origin_; }
    /*!
     * \brief 色
     *
     * 図形の輪郭の色を指定します。
     * デフォルト時のinheritはWebUI上ではblackとして表示されます
     *
     */
    Canvas2DComponent &color(const ViewColor &color) {
        color_ = color;
        return *this;
    }
    ViewColor color() const { return color_; }
    /*!
     * \brief 塗りつぶし色
     *
     * 図形の塗りつぶし色を指定します。
     * デフォルト時のinheritはWebUI上では透明になります
     *
     */
    ViewColor fillColor() const { return fill_; }
    Canvas2DComponent &fillColor(const ViewColor &color) {
        fill_ = color;
        return *this;
    }
    /*!
     * \brief 線の太さ
     *
     * 図形の輪郭の太さを指定します。
     * 太さ1はCanvas2Dの座標系で1の長さ分の太さになります(拡大縮小で太さが変わる)
     *
     * 指定しない場合0となり、WebUIではその場合Canvasの拡大に関係なく1ピクセルになります
     *
     */
    Canvas2DComponent &strokeWidth(double s) {
        stroke_width_ = s;
        return *this;
    }
    double strokeWidth() const { return stroke_width_; }
    /*!
     * \brief 文字の大きさ(高さ)
     * \since ver1.9
     *
     * 文字の大きさを指定します(Text要素の場合のみ)
     * 大きさ1は文字の高さがCanvas2Dの座標系で1の長さ分になります(拡大縮小で大きさが変わる)
     *
     * 内部のデータとしてはstrokeWidthのデータを使いまわしています
     *
     */
    Canvas2DComponent &textSize(double s) { return strokeWidth(s); }
    double textSize() const { return stroke_width_; }
    /*!
     * \brief 表示する文字列
     * \since ver1.9
     *
     */
    const std::string &text() const { return text_; }
    /*!
     * \brief 表示する文字列を設定
     * \since ver1.9
     *
     */
    Canvas2DComponent &text(const std::string &text) {
        text_ = text;
        return *this;
    }
    /*!
     * \brief geometryを取得
     *
     */
    const std::optional<Geometry> &geometry() const { return geometry_; };
    /*!
     * \brief geometryをセット
     *
     */
    Canvas2DComponent &geometry(const Geometry &g) {
        geometry_.emplace(g);
        return *this;
    };
    /*!
     * \brief クリック時に実行される関数を取得
     * \since ver1.9
     */
    std::optional<Func> onClick() const;
    /*!
     * \brief クリック時に実行される関数を設定
     * \since ver1.9
     * \param func 実行する関数を指すFuncオブジェクト
     *
     */
    Canvas2DComponent &onClick(const Func &func);
    /*!
     * \brief クリック時に実行される関数を設定
     * \since ver1.9
     * \param func 実行する任意の関数(std::functionにキャスト可能ならなんでもok)
     *
     */
    template <typename T>
    Canvas2DComponent &onClick(T func) {
        on_click_func_tmp = std::make_shared<AnonymousFunc>(func);
        return *this;
    }
    /*!
     * \brief クリック時に実行される関数を設定
     * \since ver1.9
     * \param func Client::func() で得られるAnonymousFuncオブジェクト
     * (コピー不可なので、一時オブジェクトでない場合はmoveすること)
     *
     */
    Canvas2DComponent &onClick(AnonymousFunc &&func) {
        on_click_func_tmp = std::make_shared<AnonymousFunc>(std::move(func));
        return *this;
    }
};

/*!
 * \brief Canvas2D, Canvas3D (, View) に要素をaddするときに使うインタフェース
 *
 * add時に各種Componentにキャストする
 *
 * 各オプションの詳細な説明は ViewComponent, Canvas2DComponent,
 * Canvas3DComponent を参照
 *
 */
template <bool V, bool C2, bool C3>
class TemporalComponent {
  protected:
    std::conditional_t<V, ViewComponent, int> component_v;
    std::conditional_t<C2, Canvas2DComponent, int> component_2d;
    std::conditional_t<C3, Canvas3DComponent, int> component_3d;

  public:
    TemporalComponent() = default;
    template <typename VT, typename C2T, typename C3T>
    TemporalComponent(VT v_type, C2T c2_type, C3T c3_type)
        : component_v(v_type), component_2d(c2_type), component_3d(c3_type) {}
    explicit TemporalComponent(const std::string &text)
        requires(V && C2 && !C3)
        : TemporalComponent(ViewComponentType::text,
                            Canvas2DComponentType::text, 0) {
        component_v.text(text);
        component_2d.text(text);
    }
    ViewComponent &toV()
        requires V
    {
        return component_v;
    }
    Canvas2DComponent &to2()
        requires C2
    {
        // component_2d->geometry(std::move(static_cast<Geometry &>(*this)));
        return component_2d;
    }
    Canvas3DComponent &to3()
        requires C3
    {
        // component_3d->geometry(std::move(static_cast<Geometry &>(*this)));
        return component_3d;
    }
    /*!
     * \brief クリック時に実行される関数を設定 (Viewまたは2D)
     *
     * 引数については ViewComponent::onClick(), Canvas2DComponent::onClick()
     * を参照
     *
     */
    template <typename T>
    TemporalComponent &onClick(T &&func)
        requires(V || C2)
    {
        if constexpr (V) {
            component_v.onClick(std::forward<T>(func));
        }
        if constexpr (C2) {
            component_2d.onClick(std::forward<T>(func));
        }
        return *this;
    }
    /*!
     * \brief 要素の移動 (2Dまたは3D)
     *
     */
    TemporalComponent &origin(const Transform &origin)
        requires(C2 || C3)
    {
        if constexpr (C2) {
            component_2d.origin(origin);
        }
        if constexpr (C3) {
            component_3d.origin(origin);
        }
        return *this;
    }
    /*!
     * \brief 色
     *
     * Viewの要素では textColor として設定される
     */
    TemporalComponent &color(ViewColor c)
        requires(V || C2 || C3)
    {
        if constexpr (V) {
            component_v.textColor(c);
        }
        if constexpr (C2) {
            component_2d.color(c);
        }
        if constexpr (C3) {
            component_3d.color(c);
        }
        return *this;
    }
    /*!
     * \brief 文字色 (Viewまたは2D)
     *
     * Canvas2DのTextでは fillColor が文字色の代わりに使われている
     */
    TemporalComponent &textColor(ViewColor c)
        requires(V || C2)
    {
        if constexpr (V) {
            component_v.textColor(c);
        }
        if constexpr (C2) {
            component_2d.fillColor(c);
        }
        return *this;
    }
    /*!
     * \brief 背景色 (Viewまたは2D)
     *
     */
    TemporalComponent &fillColor(ViewColor c)
        requires(V || C2)
    {
        if constexpr (V) {
            component_v.bgColor(c);
        }
        if constexpr (C2) {
            component_2d.fillColor(c);
        }
        return *this;
    }
    /*!
     * \brief 背景色 (Viewまたは2D)
     *
     */
    TemporalComponent &bgColor(ViewColor c)
        requires(V || C2)
    {
        return fillColor(c);
    }
    /*!
     * \brief 線の太さ (2Dのみ)
     *
     * 文字の太さではない
     */
    TemporalComponent &strokeWidth(double s)
        requires(C2)
    {
        if constexpr (C2) {
            component_2d.strokeWidth(s);
        }
        return *this;
    }
    /*!
     * \brief 文字の大きさ (2Dのみ)
     *
     */
    TemporalComponent &textSize(double s)
        requires(C2)
    {
        if constexpr (C2) {
            component_2d.textSize(s);
        }
        return *this;
    }
};
class TemporalGeometry : public TemporalComponent<false, true, true>,
                         public Geometry {
  public:
    TemporalGeometry(GeometryType type, std::vector<double> &&properties)
        : TemporalComponent(0, Canvas2DComponentType::geometry,
                            Canvas3DComponentType::geometry),
          Geometry(type, std::move(properties)) {
        this->component_2d.geometry(static_cast<Geometry &>(*this));
        this->component_3d.geometry(static_cast<Geometry &>(*this));
    }
};

inline namespace Components {
inline namespace Geometries {
struct Line {
    const Geometry &base;
    explicit Line(const Geometry &rg) : base(rg) {
        if (base.properties.size() != 6) {
            throw std::invalid_argument("number of properties does not match");
        }
    }
    Point begin() const {
        return Point{base.properties[0], base.properties[1],
                     base.properties[2]};
    }
    Point end() const {
        return Point{base.properties[3], base.properties[4],
                     base.properties[5]};
    }
};
inline TemporalGeometry line(const Point &begin, const Point &end) {
    return TemporalGeometry(GeometryType::line,
                            {begin.pos()[0], begin.pos()[1], begin.pos()[2],
                             end.pos()[0], end.pos()[1], end.pos()[2]});
}
struct Polygon {
    const Geometry &base;
    explicit Polygon(const Geometry &rg) : base(rg) {
        if (base.properties.size() % 3 != 0 || size() == 0) {
            throw std::invalid_argument("number of properties does not match");
        }
    }
    std::size_t size() const { return base.properties.size() / 3; }
    Point at(std::size_t i) const {
        if (i >= size()) {
            throw std::out_of_range("Polygon::at(" + std::to_string(i) +
                                    "), size = " + std::to_string(size()));
        }
        return Point{base.properties[i * 3 + 0], base.properties[i * 3 + 1],
                     base.properties[i * 3 + 2]};
    }
    Point operator[](std::size_t i) const { return at(i); }
};
inline TemporalGeometry polygon(const std::vector<Point> &points) {
    std::vector<double> properties;
    properties.reserve(points.size() * 3);
    for (const auto &p : points) {
        properties.push_back(p.pos(0));
        properties.push_back(p.pos(1));
        properties.push_back(p.pos(2));
    }
    return TemporalGeometry(GeometryType::polygon, std::move(properties));
}
struct Plane {
    const Geometry &base;
    explicit Plane(const Geometry &rg) : base(rg) {
        if (base.properties.size() != 8) {
            throw std::invalid_argument("number of properties does not match");
        }
    }
    Transform origin() const {
        return Transform{base.properties[0], base.properties[1],
                         base.properties[2], base.properties[3],
                         base.properties[4], base.properties[5]};
    }
    double width() const { return base.properties[6]; }
    double height() const { return base.properties[7]; }
    /*!
     * todo: 3次元のplaneの場合正しくない
     *
     */
    Point vertex1() const {
        return {base.properties[0] - width() / 2,
                base.properties[1] - height() / 2, base.properties[2]};
    }
    Point vertex2() const {
        return {base.properties[0] + width() / 2,
                base.properties[1] + height() / 2, base.properties[2]};
    }
};
inline TemporalGeometry plane(const Transform &origin, double width,
                              double height) {
    return TemporalGeometry(GeometryType::plane,
                            {origin.pos()[0], origin.pos()[1], origin.pos()[2],
                             origin.rot()[0], origin.rot()[1], origin.rot()[2],
                             width, height});
}
using Rect = Plane;
inline TemporalGeometry rect(const Point &origin, double width, double height) {
    return TemporalGeometry{GeometryType::plane,
                            {origin.pos()[0], origin.pos()[1], origin.pos()[2],
                             0, 0, 0, width, height}};
}
inline TemporalGeometry rect(const Point &p1, const Point &p2) {
    Transform origin = identity();
    for (int i = 0; i < 2; i++) {
        origin.pos(i) = (p1.pos(i) + p2.pos(i)) / 2;
    }
    double width = std::abs(p1.pos(0) - p2.pos(0));
    double height = std::abs(p1.pos(0) - p2.pos(0));
    return TemporalGeometry{GeometryType::plane,
                            {origin.pos(0), origin.pos(1), origin.pos(2),
                             origin.rot(0), origin.rot(1), origin.rot(2), width,
                             height}};
}

struct Box {
    const Geometry &base;
    explicit Box(const Geometry &rg) : base(rg) {
        if (base.properties.size() != 6) {
            throw std::invalid_argument("number of properties does not match");
        }
    }
    Point vertex1() const {
        return Point{base.properties[0], base.properties[1],
                     base.properties[2]};
    }
    Point vertex2() const {
        return Point{base.properties[3], base.properties[4],
                     base.properties[5]};
    }
};
inline TemporalGeometry box(const Point &vertex1, const Point &vertex2) {
    return TemporalGeometry{GeometryType::box,
                            {vertex1.pos()[0], vertex1.pos()[1],
                             vertex1.pos()[2], vertex2.pos()[0],
                             vertex2.pos()[1], vertex2.pos()[2]}};
}

struct Circle {
    const Geometry &base;
    explicit Circle(const Geometry &rg) : base(rg) {
        if (base.properties.size() != 7) {
            throw std::invalid_argument("number of properties does not match");
        }
    }
    Transform origin() const {
        return Transform{base.properties[0], base.properties[1],
                         base.properties[2], base.properties[3],
                         base.properties[4], base.properties[5]};
    }
    double radius() const { return base.properties[6]; }
};
inline TemporalGeometry circle(const Transform &origin, double radius) {
    return TemporalGeometry{GeometryType::circle,
                            {origin.pos()[0], origin.pos()[1], origin.pos()[2],
                             origin.rot()[0], origin.rot()[1], origin.rot()[2],
                             radius}};
}
inline TemporalGeometry circle(const Point &origin, double radius) {
    return TemporalGeometry{
        GeometryType::circle,
        {origin.pos()[0], origin.pos()[1], origin.pos()[2], 0, 0, 0, radius}};
}

struct Cylinder {
    const Geometry &base;
    explicit Cylinder(const Geometry &rg) : base(rg) {
        if (base.properties.size() != 8) {
            throw std::invalid_argument("number of properties does not match");
        }
    }
    Transform origin() const {
        return Transform{base.properties[0], base.properties[1],
                         base.properties[2], base.properties[3],
                         base.properties[4], base.properties[5]};
    }
    double radius() const { return base.properties[6]; }
    double length() const { return base.properties[7]; }
};
inline TemporalGeometry cylinder(const Transform &origin, double radius,
                                 double length) {
    return TemporalGeometry{GeometryType::cylinder,
                            {origin.pos()[0], origin.pos()[1], origin.pos()[2],
                             origin.rot()[0], origin.rot()[1], origin.rot()[2],
                             radius, length}};
}

struct Sphere {
    const Geometry &base;
    explicit Sphere(const Geometry &rg) : base(rg) {
        if (base.properties.size() != 4) {
            throw std::invalid_argument("number of properties does not match");
        }
    }
    Point origin() const {
        return Point{base.properties[0], base.properties[1],
                     base.properties[2]};
    }
    double radius() const { return base.properties[3]; }
};
inline TemporalGeometry sphere(const Point &origin, double radius) {
    return TemporalGeometry{
        GeometryType::sphere,
        {origin.pos()[0], origin.pos()[1], origin.pos()[2], radius}};
}


} // namespace Geometries

/*!
 * \brief textコンポーネント
 *
 */
inline TemporalComponent<true, true, false> text(const std::string &text) {
    return TemporalComponent<true, true, false>(text);
}
/*!
 * \brief newLineコンポーネント
 *
 */
inline ViewComponent newLine() {
    return ViewComponent(ViewComponentType::new_line);
}

/*!
 * \brief buttonコンポーネント
 *
 */
template <typename T>
inline ViewComponent button(const std::string &text, T &&func) {
    return ViewComponent(ViewComponentType::button)
        .text(text)
        .onClick(std::forward<T>(func));
}

inline ViewComponent textInput(const std::string &text = "") {
    return ViewComponent(ViewComponentType::text_input).text(text);
}
inline ViewComponent decimalInput(const std::string &text = "") {
    return ViewComponent(ViewComponentType::decimal_input).text(text).init(0);
}
inline ViewComponent numberInput(const std::string &text = "") {
    return ViewComponent(ViewComponentType::number_input).text(text).init(0);
}
inline ViewComponent toggleInput(const std::string &text = "") {
    return ViewComponent(ViewComponentType::toggle_input).text(text);
}
inline ViewComponent selectInput(const std::string &text = "") {
    return ViewComponent(ViewComponentType::select_input).text(text);
}
inline ViewComponent sliderInput(const std::string &text = "") {
    return ViewComponent(ViewComponentType::slider_input).text(text).init(0);
}
inline ViewComponent checkInput(const std::string &text = "") {
    return ViewComponent(ViewComponentType::check_input).text(text).init(false);
}
} // namespace Components
namespace ViewComponents = Components;

WEBCFACE_NS_END
