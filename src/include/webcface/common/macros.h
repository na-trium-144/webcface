#pragma once
#ifdef _WIN32

#if WEBCFACE_SHARED
#ifdef webcface_EXPORTS
#define WEBCFACE_DLL __declspec(dllexport)
#define WEBCFACE_IMPORT
#else
#define WEBCFACE_DLL __declspec(dllimport)
#define WEBCFACE_IMPORT __declspec(dllimport)
#endif
#endif

#else

#if WEBCFACE_SHARED
#ifdef webcface_EXPORTS
#define WEBCFACE_DLL __attribute__((visibility("default")))
#define WEBCFACE_IMPORT
#endif
#endif

#endif

#ifdef _MSC_VER
#ifdef _DEBUG
#define WEBCFACE_NS_BEGIN                                                      \
    namespace webcface {                                                       \
    inline namespace debug {
#define WEBCFACE_NS_END                                                        \
    }                                                                          \
    }
#endif
#endif

#ifndef WEBCFACE_DLL
#define WEBCFACE_DLL
#define WEBCFACE_IMPORT
#endif

#ifndef WEBCFACE_NS_BEGIN
#define WEBCFACE_NS_BEGIN namespace webcface {
#define WEBCFACE_NS_END }
#endif
