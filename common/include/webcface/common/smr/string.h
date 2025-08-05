#pragma once
#include <string>
#include "allocator.h"
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif

WEBCFACE_NS_BEGIN
namespace smr {

template <typename CharT, typename Traits = std::char_traits<CharT>>
using basic_string = std::basic_string<CharT, Traits,
        SharedResourceAllocator<CharT>>;

using string = basic_string<char>;
using wstring = basic_string<wchar_t>;

}
WEBCFACE_NS_END
