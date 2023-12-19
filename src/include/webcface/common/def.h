#pragma once
#define WEBCFACE_STR(v) #v
#define WEBCFACE_DEFAULT_PORT 7530
#define WEBCFACE_DEFAULT_PORT_SI(port) WEBCFACE_STR(port)
#define WEBCFACE_DEFAULT_PORT_S WEBCFACE_DEFAULT_PORT_SI(WEBCFACE_DEFAULT_PORT)
#define WEBCFACE_SERVER_NAME "webcface"
#define WEBCFACE_VERSION_MAJOR 1
#define WEBCFACE_VERSION_MINOR 2
#define WEBCFACE_VERSION_REVISION 2
// CMakeを使えばWEBCFACE_VERSIONが与えられるが、もしなければ生成
#ifndef WEBCFACE_VERSION
#define WEBCFACE_VERSION_I(maj, min, rev)                                      \
    WEBCFACE_STR(maj) "." WEBCFACE_STR(min) "." WEBCFACE_STR(rev)
#define WEBCFACE_VERSION                                                       \
    WEBCFACE_VERSION_I(WEBCFACE_VERSION_MAJOR, WEBCFACE_VERSION_MINOR,         \
                       WEBCFACE_VERSION_REVISION)
#endif

#ifdef _MSC_VER
#ifdef webcface_EXPORTS
#define WEBCFACE_DLL __declspec(dllexport)
#else
#define WEBCFACE_DLL __declspec(dllimport)
#endif
#else
#define WEBCFACE_DLL
#endif
