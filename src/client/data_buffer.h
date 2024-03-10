#pragma once
#include <webcface/common/def.h>
#include <webcface/view.h>
#include <sstream>

namespace WEBCFACE_NS::Internal{
/*!
 * \brief View,Canvasなどで送信用にaddされたデータを管理する
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

    ~DataSetBuffer() {
        if (!target_.data_w.expired() && target_.isSelf()) {
            sync();
        }
    }

    /*!
     * \brief データを処理しtargetにsetする
     *
     * 実装は型ごと
     *
     */
    void sync();

    void init() {
        components_.clear();
        modified_ = true;
    }
    void add(const Component &cp) {
        components_.push_back(cp);
        modified_ = true;
    }
    void add(Component &&cp) {
        components_.push_back(std::move(cp));
        modified_ = true;
    }
    void set(const std::vector<Component> &cv) {
        for (const auto &cp : cv) {
            components_.push_back(cp);
        }
        modified_ = true;
    }
    void set(std::initializer_list<Component> cl) {
        for (const auto &cp : cl) {
            components_.push_back(cp);
        }
        modified_ = true;
    }
    const std::vector<Component> &components() const { return components_; }
};

template <>
void DataSetBuffer<ViewComponent>::sync();

/*!
 * \brief Viewの送信用データを保持する
 *
 */
class ViewBuf : public std::stringbuf, public DataSetBuffer<ViewComponent> {
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
    ~ViewBuf();
};

}
