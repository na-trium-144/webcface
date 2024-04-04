#pragma once
#include <webcface/common/def.h>
#include <webcface/view.h>
#include <webcface/common/robot_model.h>
#include <webcface/canvas_data.h>
#include <sstream>
#include <memory>

WEBCFACE_NS_BEGIN
namespace Internal {
/*!
 * \brief View,Canvasなどで送信用にaddされたデータを管理する
 *
 * Field側ではこれをshared_ptrで構築し適宜 init(), add(), sync() する
 *
 */
template <typename Component>
class DataSetBuffer {
    Field target_;
    std::vector<Component> components_;
    bool modified_;

  public:
    DataSetBuffer() : target_(), components_(), modified_(false) {}
    explicit DataSetBuffer(const Field &base)
        : target_(base), components_(), modified_(false) {}
    DataSetBuffer(const DataSetBuffer &) = delete;
    DataSetBuffer(DataSetBuffer &&) = delete;
    DataSetBuffer &operator=(const DataSetBuffer &) = delete;
    DataSetBuffer &operator=(DataSetBuffer &&) = delete;

    virtual ~DataSetBuffer() { onDestroy(); }
    void onDestroy() {
        if (!target_.data_w.expired() && target_.isSelf()) {
            sync();
        }
    }

    void sync() {
        if (modified_) {
            modified_ = false;
            onSync();
        }
    }
    /*!
     * \brief データを処理しtargetにsetする
     *
     * 実装は型ごと
     *
     */
    void onSync();

    void init() {
        components_.clear();
        modified_ = true;
    }

    /*!
     * \brief add時のチェック
     *
     */
    virtual void onAdd() {}
    void add(const Component &cp) {
        onAdd();
        components_.push_back(cp);
        modified_ = true;
    }
    void add(Component &&cp) {
        onAdd();
        components_.push_back(std::move(cp));
        modified_ = true;
    }
    /*!
     * \brief まとめてセット
     *
     * initしてcomponentsを置き換えてsyncする
     *
     */
    void set(const std::vector<Component> &cv) {
        onAdd();
        components_ = cv;
        modified_ = true;
        sync();
    }
    void set(std::initializer_list<Component> cl) {
        onAdd();
        components_ = std::vector<Component>(cl.begin(), cl.end());
        modified_ = true;
        sync();
    }
    const std::vector<Component> &components() const { return components_; }
};

template <>
void DataSetBuffer<ViewComponent>::onSync();
template <>
void DataSetBuffer<RobotLink>::onSync();
template <>
void DataSetBuffer<Canvas2DComponent>::onSync();
template <>
void DataSetBuffer<Canvas3DComponent>::onSync();

/*!
 * \brief Viewの送信用データを保持する
 *
 */
class ViewBuf final : public std::stringbuf,
                      public DataSetBuffer<ViewComponent> {
    /*!
     * こっちはstreambufのsync
     *
     */
    int sync() override;

  public:
    /*!
     * \brief componentsに追加
     *
     * textは改行で分割する
     *
     */
    void addVC(const ViewComponent &vc);
    void addVC(ViewComponent &&vc);
    void addText(const ViewComponent &vc);
    void syncSetBuf() { this->DataSetBuffer<ViewComponent>::sync(); }

    explicit ViewBuf();
    explicit ViewBuf(const Field &base);
    ~ViewBuf() override;
};

class Canvas2DDataBuf final : public DataSetBuffer<Canvas2DComponent> {
    double width_ = 0, height_ = 0;

  public:
    friend DataSetBuffer<Canvas2DComponent>;
    Canvas2DDataBuf() = default;
    Canvas2DDataBuf(const Field &base)
        : DataSetBuffer<Canvas2DComponent>(base) {}
    void onAdd() override { checkSize(); }
    void checkSize() const {
        if (width_ <= 0 && height_ <= 0) {
            throw std::invalid_argument("Canvas2D size is invalid (" +
                                        std::to_string(width_) + ", " +
                                        std::to_string(height_) + ")");
        }
    }
    void init(double width, double height) {
        width_ = width;
        height_ = height;
        this->DataSetBuffer<Canvas2DComponent>::init();
    }
    /*!
     * ~DataSetBuffer() の時点ではすでにCanvas2DDataBufが破棄されているので、
     * その前にonDestroyを呼ぶ
     */
    ~Canvas2DDataBuf() override { onDestroy(); }
};

} // namespace Internal
WEBCFACE_NS_END
