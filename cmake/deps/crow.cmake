include(cmake/fetch.cmake)
option(WEBCFACE_FIND_CROW "try find_package(Crow)" ${WEBCFACE_FIND_LIBS})

# target = crow-linker (header only)

if(WEBCFACE_FIND_CROW)
    find_package(Crow QUIET)
endif()
if(Crow_FOUND)
    include(CheckCXXSourceCompiles)
    set(CMAKE_REQUIRED_LIBRARIES Crow::Crow)
    check_cxx_source_compiles("
#include <crow.h>
int main(){
    crow::SimpleApp app;
    CROW_ROUTE(app, \"/\")([](){
        return \"Hello world\";
    });
    app.unix_path(\"/tmp/test.sock\").multithreaded().run();
}" CROW_HAS_UNIX_SUPPORT)
    unset(CMAKE_REQUIRED_LIBRARIES)
    if(NOT CROW_HAS_UNIX_SUPPORT)
        list(APPEND WEBCFACE_SUMMARY "(crow found but it has no unix socket support.)")
        unset(Crow_FOUND)
    else()
        list(APPEND WEBCFACE_SUMMARY "crow: ${Crow_VERSION} found at ${Crow_DIR}")
        add_library(crow-linker INTERFACE)
        target_link_libraries(crow-linker INTERFACE Crow::Crow)
    endif()
endif()
if(NOT Crow_FOUND)
    include(cmake/deps/asio.cmake)

    # asioのオプションを変更するため
    # add_subdirectoryではなくinterfaceライブラリを定義する
    fetch_only(crow
        # https://github.com/CrowCpp/Crow.git
        # 921ce6f
        https://github.com/na-trium-144/Crow.git
        5f5372ed80860dfcef788972bb0fd3972f715842
        CMakeLists.txt
    )
    add_library(crow-linker INTERFACE)
    target_include_directories(crow-linker INTERFACE $<BUILD_INTERFACE:${crow_SOURCE_DIR}/include>)
    target_link_libraries(crow-linker INTERFACE asio)

    if(WEBCFACE_INSTALL)
        install(FILES
            ${crow_SOURCE_DIR}/LICENSE
            ${crow_SOURCE_DIR}/README.md
            # 3rdpartyのライセンスがREADMEにある
            DESTINATION share/webcface/3rd_party/crow
        )
    endif()
endif()

