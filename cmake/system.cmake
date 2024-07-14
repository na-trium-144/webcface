# project() の後に読み込む
# osごとの違いを分岐する変数を設定する。

set(WEBCFACE_SYSTEM_VERSION_RC 0) # use version.rc
set(WEBCFACE_SYSTEM_VISIBILITY 0) # use visibility=hidden
set(WEBCFACE_SYSTEM_DLLEXPORT 0) # use __declspec(dllexport)
set(WEBCFACE_SYSTEM_EXCLUDE_LIBS 0) # use --exclude-libs,ALL
set(WEBCFACE_SYSTEM_HIDDEN_L 0) # use --hidden-l
set(WEBCFACE_SYSTEM_ADD_VERSION 0) # add soversion to filename
set(WEBCFACE_SYSTEM_ADD_DEBUG 0) # add debug postfix
set(WEBCFACE_SYSTEM_WIN32API 0) # use win32api, wchar_t=utf16
set(WEBCFACE_SYSTEM_WINNT 0) # _WINNT is defined by default
set(WEBCFACE_SYSTEM_PATH_WINDOWS 0) # use windows style path

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(WEBCFACE_SYSTEM_VISIBILITY 1)
    set(WEBCFACE_SYSTEM_EXCLUDE_LIBS 1)

elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(WEBCFACE_SYSTEM_VISIBILITY 1)
    set(WEBCFACE_SYSTEM_HIDDEN_L 1)

elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(WEBCFACE_SYSTEM_DLLEXPORT 1)
    set(WEBCFACE_SYSTEM_VERSION_RC 1)
    set(WEBCFACE_SYSTEM_ADD_VERSION 1)
    set(WEBCFACE_SYSTEM_WIN32API 1)
    set(WEBCFACE_SYSTEM_PATH_WINDOWS 1)
    set(WEBCFACE_SYSTEM_WINNT 1)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        set(WEBCFACE_SYSTEM_ADD_DEBUG 1)
    endif()

elseif(CMAKE_SYSTEM_NAME STREQUAL "CYGWIN" OR CMAKE_SYSTEM_NAME STREQUAL "MSYS")
    message(WARNING "System name \"${CMAKE_SYSTEM_NAME}\" is not supported yet.")
    set(WEBCFACE_SYSTEM_DLLEXPORT 1)
    set(WEBCFACE_SYSTEM_ADD_VERSION 1)
    set(WEBCFACE_SYSTEM_WIN32API 1)

else()
    message(WARNING "System name \"${CMAKE_SYSTEM_NAME}\" is not supported yet.")

endif()
