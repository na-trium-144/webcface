#pragma once
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include "webcface/common/encoding.h"

WEBCFACE_NS_BEGIN
namespace internal {

/*!
 * \brief View,Canvasなどで使う差分要素を管理するクラス
 *
 * 各要素にidを振り、id→要素のデータ の対応を components が管理し、
 * idの並び順を data_ids が管理する
 *
 */
template <typename ComponentData>
struct DiffData {
    std::map<std::string, std::shared_ptr<ComponentData>> components;
    std::vector<SharedString> data_ids;

    DiffData() = default;
    DiffData(
        const std::map<std::string, std::shared_ptr<ComponentData>> &components,
        std::vector<SharedString> &&data_ids)
        : components(components), data_ids(std::move(data_ids)) {}
    static DiffData mergeDiff(
        const DiffData *prev,
        const std::map<std::string, std::shared_ptr<ComponentData>> &components,
        std::optional<std::vector<SharedString>> &&data_ids) {
        DiffData data;
        if (prev) {
            data.components = prev->components;
        }
        if (data_ids) {
            data.data_ids = std::move(*data_ids);
        } else if (prev) {
            data.data_ids = prev->data_ids;
        }
        for (auto &c : components) {
            if (c.second) {
                data.components[c.first] = c.second;
            }
        }
        return data;
    }

    void reserve(std::size_t size) { data_ids.reserve(size); }
    void emplace_back(const SharedString &id,
                      const std::shared_ptr<ComponentData> &data) {
        data_ids.emplace_back(id);
        components.emplace(id.u8String(), data);
    }
    template <typename F>
    void forEach(F f) const {
        for (const auto &id : data_ids) {
            if (!components.count(id.u8String()) ||
                !components.at(id.u8String())) {
                static const std::shared_ptr<ComponentData> empty =
                    std::make_shared<ComponentData>();
                f(id, empty);
            } else {
                f(id, components.at(id.u8String()));
            }
        }
    }
};

} // namespace internal
WEBCFACE_NS_END