#include "webcface/common/internal/message/pack.h"
#include "webcface/common/internal/message/plot.h"
#include "webcface/plot.h"
#include "webcface/value.h"
#include "webcface/internal/data_buffer.h"
#include "webcface/internal/client_internal.h"

WEBCFACE_NS_BEGIN
PlotSeries::PlotSeries(PlotSeriesType type, const std::vector<Value> &values)
    : msg_data(std::make_shared<message::PlotSeriesData>()), data_w() {
    this->msg_data->type = static_cast<int>(type);
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

void PlotSeries::checkData() const {
    if (!this->msg_data) {
        throw std::runtime_error("Accessed empty PlotSeries");
    }
}

PlotSeriesType PlotSeries::type() const {
    checkData();
    return static_cast<PlotSeriesType>(this->msg_data->type);
}

template <typename T, bool>
std::vector<T> PlotSeries::values() const {
    checkData();
    std::vector<Value> v;
    auto data = this->data_w.lock();
    for (std::size_t i = 0; i < this->msg_data->value_member.size() &&
                            i < this->msg_data->value_field.size();
         i++) {
        v.push_back(Value{Field{data, this->msg_data->value_member[i],
                                this->msg_data->value_field[i]}});
    }
    return v;
}
template WEBCFACE_DLL std::vector<Value>
PlotSeries::values<Value, true>() const;

ViewColor PlotSeries::color() const {
    checkData();
    return static_cast<ViewColor>(this->msg_data->color);
}
PlotSeries &PlotSeries::color(ViewColor color) & {
    checkData();
    this->msg_data->color = static_cast<int>(color);
    return *this;
}

double PlotSeries::xMin() const {
    checkData();
    return this->msg_data->range[0];
}
double PlotSeries::xMax() const {
    checkData();
    return this->msg_data->range[1];
}
double PlotSeries::yMin() const {
    checkData();
    return this->msg_data->range[2];
}
double PlotSeries::yMax() const {
    checkData();
    return this->msg_data->range[3];
}
PlotSeries &PlotSeries::xRange(double x_min, double x_max) & {
    checkData();
    this->msg_data->range[0] = x_min;
    this->msg_data->range[1] = x_max;
    return *this;
}
PlotSeries &PlotSeries::yRange(double y_min, double y_max) & {
    checkData();
    this->msg_data->range[2] = y_min;
    this->msg_data->range[3] = y_max;
    return *this;
}

PlotSeries plot_series::trace1(webcface::Value &y) {
    return PlotSeries(PlotSeriesType::trace1, {y});
}
PlotSeries plot_series::trace2(webcface::Value &x, webcface::Value &y) {
    return PlotSeries(PlotSeriesType::trace2, {x, y});
}
PlotSeries plot_series::scatter2(webcface::Value &x, webcface::Value &y) {
    return PlotSeries(PlotSeriesType::scatter2, {x, y});
}
PlotSeries plot_series::line2(webcface::Value &x, webcface::Value &y) {
    return PlotSeries(PlotSeriesType::line2, {x, y});
}


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
