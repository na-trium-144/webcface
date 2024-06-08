#pragma once
// clang-format off
/*
WEBCFACE_DLL: 関数の宣言 (msvc, mingw, unix)
WEBCFACE_DLL_TEMPLATE: テンプレートクラスの定義 (unix)
WEBCFACE_DLL_INSTANCE_DECL: 明示的実体化の宣言 (msvc_import, mingw) (unixでは消す:gcc-10でエラー)
WEBCFACE_DLL_INSTANCE_DEF: 明示的実体化の定義 (msvc_export)
*/
// clang-format on

#if WEBCFACE_SHARED
#ifdef _WIN32
#ifdef webcface_EXPORTS
#ifdef _MSC_VER
#define WEBCFACE_DLL __declspec(dllexport)
#define WEBCFACE_DLL_TEMPLATE
#define WEBCFACE_DLL_INSTANCE_DECL
#define WEBCFACE_DLL_INSTANCE_DEF WEBCFACE_DLL
#else // !_MSC_VER => on MinGW
#define WEBCFACE_DLL __declspec(dllexport)
#define WEBCFACE_DLL_TEMPLATE
#define WEBCFACE_DLL_INSTANCE_DECL WEBCFACE_DLL
#define WEBCFACE_DLL_INSTANCE_DEF
#endif // _MSC_VER
#else  // !webcface_EXPORTS
#define WEBCFACE_DLL __declspec(dllimport)
#define WEBCFACE_DLL_TEMPLATE
#define WEBCFACE_DLL_INSTANCE_DECL WEBCFACE_DLL
#define WEBCFACE_DLL_INSTANCE_DEF
#endif // webcface_EXPORTS
#else  // !_WIN32
#ifdef webcface_EXPORTS
#define WEBCFACE_DLL __attribute__((visibility("default")))
#define WEBCFACE_DLL_TEMPLATE WEBCFACE_DLL
#undef WEBCFACE_DLL_INSTANCE_DECL
#define WEBCFACE_DLL_INSTANCE_DEF
#else // !webcface_EXPORTS
#define WEBCFACE_DLL
#define WEBCFACE_DLL_TEMPLATE
#undef WEBCFACE_DLL_INSTANCE_DECL
#define WEBCFACE_DLL_INSTANCE_DEF
#endif // webcface_EXPORTS
#endif // _WIN32
#else  // !WEBCFACE_SHARED
#define WEBCFACE_DLL
#define WEBCFACE_DLL_TEMPLATE
#define WEBCFACE_DLL_INSTANCE_DECL
#define WEBCFACE_DLL_INSTANCE_DEF
#endif // WEBCFACE_SHARED

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

#ifndef WEBCFACE_NS_BEGIN
#define WEBCFACE_NS_BEGIN namespace webcface {
#define WEBCFACE_NS_END }
#endif
