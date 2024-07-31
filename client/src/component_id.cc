#include "webcface/component_id.h"
WEBCFACE_NS_BEGIN

template <typename TypeEnum>
std::string IdBase<TypeEnum>::id() const {
    return ".." + std::to_string(static_cast<int>(type())) + "." +
           std::to_string(idx_for_type_);
}
template <typename TypeEnum>
void IdBase<TypeEnum>::initIdx(std::unordered_map<int, int> *idx_next,
                               TypeEnum type) {
    if (idx_next) {
        idx_for_type_ = (*idx_next)[static_cast<int>(type)]++;
    }
}

template class WEBCFACE_DLL_INSTANCE_DEF IdBase<ViewComponentType>;
template class WEBCFACE_DLL_INSTANCE_DEF IdBase<Canvas2DComponentType>;

WEBCFACE_NS_END
