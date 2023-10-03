#pragma once
#define WEBCFACE_STR(v) #v
#define WEBCFACE_DEFAULT_PORT 7530
#define WEBCFACE_DEFAULT_PORT_SI(port) WEBCFACE_STR(port)
#define WEBCFACE_DEFAULT_PORT_S WEBCFACE_DEFAULT_PORT_SI(WEBCFACE_DEFAULT_PORT)
#define WEBCFACE_SERVER_NAME "webcface"
#define WEBCFACE_VERSION_MAJOR 1
#define WEBCFACE_VERSION_MINOR 0
#define WEBCFACE_VERSION_REVISION 1
#define WEBCFACE_VERSION_I(maj, min, rev)                                      \
    WEBCFACE_STR(maj) "." WEBCFACE_STR(min) "." WEBCFACE_STR(rev)
#define WEBCFACE_VERSION                                                       \
    WEBCFACE_VERSION_I(WEBCFACE_VERSION_MAJOR, WEBCFACE_VERSION_MINOR,         \
                       WEBCFACE_VERSION_REVISION)
