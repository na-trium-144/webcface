include(FetchContent)

# external/名前 ディレクトリがあればそれを使用し、
# なければFetchContentするマクロ

# FetchContentでは名前はlower caseになるので、それで統一
# (必ず名前もディレクトリ名もlower caseにすること)

# ルートのCMakeListsからincludeすること

macro(fetch_cmake DEP_NAME URL TAG)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/external/${DEP_NAME}/CMakeLists.txt)
        message(STATUS "Using Source at external/${DEP_NAME}")
        set(${DEP_NAME}_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/${DEP_NAME})
        set(${DEP_NAME}_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/_deps/${DEP_NAME}-build)
        add_subdirectory(external/${DEP_NAME} _deps/${DEP_NAME}-build)
    else()
        message(STATUS "Fetching Source from ${URL}...")
        FetchContent_Declare(${DEP_NAME} GIT_REPOSITORY ${URL} GIT_TAG ${TAG})
        FetchContent_MakeAvailable(${DEP_NAME})
    endif()
endmacro()
macro(fetch_only DEP_NAME URL TAG CHECK_FILENAME)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/external/${DEP_NAME}/${CHECK_FILENAME})
        message(STATUS "Using Source at external/${DEP_NAME}")
        set(${DEP_NAME}_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/${DEP_NAME})
        set(${DEP_NAME}_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/_deps/${DEP_NAME}-build)
    else()
        message(STATUS "Fetching Source from ${URL}...")
        FetchContent_Declare(${DEP_NAME} GIT_REPOSITORY ${URL} GIT_TAG ${TAG})
        FetchContent_Populate(${DEP_NAME})
    endif()
endmacro()
