#pragma once
#define WEBCFACE_STR(v) #v
#define WEBCFACE_DEFAULT_PORT 7530
#define WEBCFACE_DEFAULT_PORT_S WEBCFACE_STR(WEBCFACE_DEFAULT_PORT)
#define WEBCFACE_SERVER_NAME "webcface"
#define WEBCFACE_VERSION_MAJOR 1
#define WEBCFACE_VERSION_MINOR 0
#define WEBCFACE_VERSION_REVISION 0
#define WEBCFACE_VERSION                                                       \
    WEBCFACE_STR(WEBCFACE_VERSION_MAJOR)                                       \
    "." WEBCFACE_STR(WEBCFACE_VERSION_MINOR) "." WEBCFACE_STR(                 \
        WEBCFACE_VERSION_REVISION)
