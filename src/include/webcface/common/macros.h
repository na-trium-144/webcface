#pragma once
#ifdef _MSC_VER

#ifdef webcface_EXPORTS
#define WEBCFACE_DLL __declspec(dllexport)
#define WEBCFACE_IMPORT
#else
#define WEBCFACE_DLL __declspec(dllimport)
#define WEBCFACE_IMPORT __declspec(dllimport)
#endif

#ifdef _DEBUG
#define WEBCFACE_NS_BEGIN                                                      \
    namespace webcface {                                                       \
    inline namespace debug {
#define WEBCFACE_NS_END                                                        \
    }                                                                          \
    }
#endif

#else

#define WEBCFACE_DLL
#define WEBCFACE_IMPORT

#endif

#ifndef WEBCFACE_NS_BEGIN
#define WEBCFACE_NS_BEGIN namespace webcface {
#define WEBCFACE_NS_END }
#endif
