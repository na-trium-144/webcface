#include "webcface/common/internal/message/pack.h"
#include "webcface/common/internal/message/plot.h"
#include "webcface/plot.h"
#include "webcface/value.h"
#include "webcface/internal/data_buffer.h"
#include "webcface/internal/client_internal.h"

WEBCFACE_NS_BEGIN
PlotSeries::PlotSeries(const std::vector<Value> &values)
    : msg_data(std::make_shared<message::PlotSeriesData>()), data_w() {
    this->msg_data->value_member.reserve(values.size());
    this->msg_data->value_field.reserve(values.size());
    for (const auto &v : values) {
        this->msg_data->value_member.push_back(v.member_);
        this->msg_data->value_field.push_back(v.field_);
        auto data = v.data_w.lock();
        if (data) {
            this->data_w = data;
        }
    }
}
PlotSeries::PlotSeries(const Value &value_x, const Value &value_y)
    : PlotSeries(std::vector<Value>{value_x, value_y}) {}


Plot::Plot()
    : Field(), sb(std::make_shared<internal::DataSetBuffer<PlotSeries>>()) {}
Plot::Plot(const Field &base)
    : Field(base),
      sb(std::make_shared<internal::DataSetBuffer<PlotSeries>>(base)) {}
Plot::~Plot() {}

Plot &Plot::operator=(const Plot &rhs) {
    if (this == &rhs) {
        return *this;
    }
    this->Field::operator=(rhs);
    this->sb = rhs.sb;
    return *this;
}
Plot &Plot::operator=(Plot &&rhs) noexcept {
    if (this == &rhs) {
        return *this;
    }
    this->Field::operator=(std::move(static_cast<Field &>(rhs)));
    this->sb = std::move(rhs.sb);
    return *this;
}


const Plot &Plot::init() const {
    sb->init();
    return *this;
}
const Plot &Plot::sync() const {
    sb->sync();
    return *this;
}
const Plot &Plot::operator<<(PlotSeries vc) const {
    sb->add(std::move(vc));
    return *this;
}

template <>
void internal::DataSetBuffer<PlotSeries>::onSync() {
    auto data = target_.setCheck();
    std::vector<std::shared_ptr<message::PlotSeriesData>> series_data;
    series_data.reserve(components_.size());
    for (std::size_t i = 0; i < components_.size(); i++) {
        series_data.push_back(components_[i].msg_data);
    }
    data->plot_store.setSend(target_, series_data);

    auto change_event =
        internal::findFromMap2(data->plot_change_event.shared_lock().get(),
                               target_.member_, target_.field_);
    if (change_event && *change_event) {
        change_event->operator()(target_);
    }
}
const Plot &Plot::onChange(std::function<void(Plot)> callback) const {
    this->request();
    auto data = dataLock();
    data->plot_change_event.lock().get()[this->member_][this->field_] =
        std::make_shared<std::function<void(Plot)>>(std::move(callback));
    return *this;
}

const Plot &Plot::request() const {
    auto data = dataLock();
    auto req = data->plot_store.addReq(member_, field_);
    if (req) {
        data->messagePushReq(
            message::Req<message::Plot>{{}, member_, field_, req});
    }
    return *this;
}
std::optional<std::vector<PlotSeries>> Plot::tryGet() const {
    auto vb = dataLock()->plot_store.getRecv(*this);
    request();
    if (vb) {
        std::vector<PlotSeries> v;
        v.reserve((*vb).size());
        for (const auto &s : *vb) {
            v.emplace_back(s, data_w);
        }
        return v;
    } else {
        return std::nullopt;
    }
}
const Plot &Plot::free() const {
    auto req = dataLock()->plot_store.unsetRecv(*this);
    if (req) {
        // todo: リクエスト解除
    }
    return *this;
}
bool Plot::exists() const {
    return dataLock()->plot_store.getEntry(member_).count(field_);
}

WEBCFACE_NS_END
