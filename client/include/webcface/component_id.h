#pragma once
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/config.h"
#endif
#include <unordered_map>
#include <string>

WEBCFACE_NS_BEGIN

template <typename TypeEnum>
class WEBCFACE_DLL_TEMPLATE IdBase {
  protected:
    int idx_for_type_ = 0;
    void initIdx(std::unordered_map<int, int> *idx_next, TypeEnum type);

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
    std::string id() const;
};

enum class ViewComponentType {
    text = 0,
    new_line = 1,
    button = 2,
    text_input = 3,
    decimal_input = 4,
    number_input = 5,
    toggle_input = 6,
    select_input = 7,
    slider_input = 8,
    check_input = 9,
};

enum class Canvas2DComponentType {
    geometry = 0,
    text = 3,
};

#if WEBCFACE_SYSTEM_DLLEXPORT
extern template class WEBCFACE_DLL_INSTANCE_DECL IdBase<ViewComponentType>;
extern template class WEBCFACE_DLL_INSTANCE_DECL IdBase<Canvas2DComponentType>;
#endif

WEBCFACE_NS_END
