#pragma once
#include <webcface/common/def.h>
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

WEBCFACE_NS_END
