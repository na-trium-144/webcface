include(cmake/fetch.cmake)
option(WEBCFACE_FIND_CURL "try find_package(CURL)" ${WEBCFACE_FIND_LIBS})

# target = webcface-libcurl-linker

if(WEBCFACE_FIND_CURL)
    find_package(CURL QUIET)
endif()
if(CURL_FOUND)
    include(CheckCXXSourceRuns)
    set(CMAKE_REQUIRED_LIBRARIES CURL::libcurl)
    check_cxx_source_runs("
#include <curl/curl.h>
#include <cassert>
#include <cstddef>
int main() {
    CURL *curl = curl_easy_init();
    std::size_t s;
    assert(curl_ws_send(curl, nullptr, 0, &s, 0, 0) != CURLE_NOT_BUILT_IN);
}" CURL_HAS_WS_SUPPORT)
    unset(CMAKE_REQUIRED_LIBRARIES)
    if(NOT CURL_HAS_WS_SUPPORT)
        list(APPEND WEBCFACE_SUMMARY "(curl found but it has no websockets support.)")
        unset(CURL_FOUND)
    endif()
endif()
if(CURL_FOUND)
    list(APPEND WEBCFACE_SUMMARY "curl: ${CURL_VERSION_STRING} found at ${PC_CURL_LIBDIR}")
    set(libcurl CURL::libcurl)
    add_library(webcface-libcurl-linker INTERFACE)
    target_link_libraries(webcface-libcurl-linker INTERFACE CURL::libcurl)

    if(WEBCFACE_INSTALL)
        list(APPEND WEBCFACE_EXPORTS webcface-libcurl-linker)
        set(CURL_INSTALLED 1)
    endif()

else()
    set(CURL_ENABLE_SSL off CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_ALTSVC on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_SRP on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_BASIC_AUTH on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_BEARER_AUTH on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_DIGEST_AUTH on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_KERBEROS_AUTH on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_NEGOTIATE_AUTH on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_AWS on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_DICT on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_DOH on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_FILE on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_FORM_API on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_FTP on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_GETOPTIONS on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_GOPHER on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_HSTS on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_IMAP on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_LDAP on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_LDAPS on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_LIBCURL_OPTION on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_MIME on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_MQTT on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_BINDLOCAL on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_NETRC on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_NTLM on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_PARSEDATE on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_POP3 on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_PROGRESS_METER on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_RTSP on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_SMB on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_SMTP on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_SOCKETPAIR on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_TELNET on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_TFTP on CACHE INTERNAL "" FORCE)
    set(CURL_ZLIB off CACHE INTERNAL "" FORCE)
    set(CURL_USE_LIBSSH2 off CACHE INTERNAL "" FORCE)
    set(CURL_USE_LIBSSH off CACHE INTERNAL "" FORCE)
    set(BUILD_CURL_EXE on CACHE INTERNAL "" FORCE)
    set(BUILD_SHARED_LIBS off CACHE INTERNAL "" FORCE)
    set(BUILD_STATIC_LIBS on CACHE INTERNAL "" FORCE)
    set(SHARE_LIB_OBJECT ${WEBCFACE_PIC} CACHE INTERNAL "" FORCE)
    set(ENABLE_WEBSOCKETS on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_INSTALL on CACHE INTERNAL "" FORCE)
    set(CURL_ENABLE_EXPORT_TARGET off CACHE INTERNAL "" FORCE)
    fetch_cmake(curl
        https://github.com/curl/curl.git
        curl-8_5_0
    )

    include(cmake/linker.cmake)
    add_library(webcface-libcurl-linker INTERFACE)
    target_include_directories(webcface-libcurl-linker INTERFACE
        $<BUILD_INTERFACE:$<TARGET_PROPERTY:libcurl_static,INTERFACE_INCLUDE_DIRECTORIES>>
    )
    target_compile_definitions(webcface-libcurl-linker INTERFACE
        $<BUILD_INTERFACE:$<TARGET_PROPERTY:libcurl_static,INTERFACE_COMPILE_DEFINITIONS>>
    )
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_static_link(webcface-libcurl-linker
            BUILD_LIBRARY_DIRS $<TARGET_LINKER_FILE_DIR:libcurl_static>
            INSTALL_LIBRARY_DIRS $<TARGET_LINKER_FILE_DIR:webcface::libcurl_static>
            DEBUG_LIBRARIES libcurl-d.lib
            RELEASE_LIBRARIES libcurl.lib
        )
    else()
        target_static_link(webcface-libcurl-linker
            BUILD_LIBRARY_DIRS $<TARGET_LINKER_FILE_DIR:libcurl_static>
            INSTALL_LIBRARY_DIRS $<TARGET_LINKER_FILE_DIR:webcface::libcurl_static>
            DEBUG_LIBRARIES curl-d
            RELEASE_LIBRARIES curl
        )
    endif()
    get_target_property(libcurl_link_libraries libcurl_static INTERFACE_LINK_LIBRARIES)
    if(NOT libcurl_link_libraries STREQUAL "libcurl_link_libraries-NOTFOUND")
        if(WEBCFACE_SHARED)
            target_link_libraries(webcface-libcurl-linker INTERFACE $<BUILD_INTERFACE:${libcurl_link_libraries}>)
        else()
            target_link_libraries(webcface-libcurl-linker INTERFACE ${libcurl_link_libraries})
        endif()
    endif()
    if(WEBCFACE_SYSTEM_WIN32SOCKET)
        target_link_libraries(webcface-libcurl-linker INTERFACE ws2_32 wsock32)
    endif()
    add_dependencies(webcface-libcurl-linker libcurl_static)

    if(WEBCFACE_INSTALL)
        list(APPEND WEBCFACE_EXPORTS webcface-libcurl-linker)
        set(CURL_INSTALLED 0)
        # licenseファイルはリポジトリ内にないしバイナリ配布に必須ではないのでスキップ
        if(NOT WEBCFACE_SHARED)
            list(APPEND WEBCFACE_EXPORTS libcurl_static)
            if(CMAKE_BUILD_TYPE STREQUAL "Debug")
                list(APPEND libcurl_PKGCONFIG_LIBS -lcurl-d)
            else()
                list(APPEND libcurl_PKGCONFIG_LIBS -lcurl)
            endif()
            get_directory_property(CURL_LIBS
                DIRECTORY ${curl_SOURCE_DIR}
                DEFINITION CURL_LIBS
            )
            # curlのCMakeListsからコピペ
            foreach(_lib IN LISTS CURL_LIBS)
                if(TARGET "${_lib}")
                    set(_libname "${_lib}")
                    get_target_property(_imported "${_libname}" IMPORTED)
                    if(NOT _imported)
                        # Reading the LOCATION property on non-imported target will error out.
                        # Assume the user won't need this information in the .pc file.
                        continue()
                    endif()
                    get_target_property(_lib "${_libname}" LOCATION)
                    if(NOT _lib)
                        # message(WARNING "Bad lib in library list: ${_libname}")
                        continue()
                    endif()
                endif()
                if(_lib MATCHES ".*/.*" OR _lib MATCHES "^-")
                    list(APPEND libcurl_PKGCONFIG_LIBS ${_lib})
                else()
                    list(APPEND libcurl_PKGCONFIG_LIBS -l${_lib})
                endif()
            endforeach()
            list(INSERT WEBCFACE_PKGCONFIG_LIBS 0 ${libcurl_PKGCONFIG_LIBS})
        endif()
    endif()
endif()
