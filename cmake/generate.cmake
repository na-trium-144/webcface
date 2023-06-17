function(webcface_generate)
    set(options )
    set(oneValueArgs TARGET NAMESPACE)
    set(multiValueArgs SOURCES)
    cmake_parse_arguments(WEBCFACE_GENERATE "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    # webcface::generate ターゲットをつくる
    # ソースはpythonで生成
    # WEBCFACE_SOURCES内の関数を宣言だけして、WebCFaceに登録するaddGenerateFunctions()関数を定義
    # WEBCFACE_SOURCES: cppファイル
    # WEBCFACE_NAMESPACE: 対象のnamespace
    if("${WEBCFACE_GENERATOR_COMMAND}" STREQUAL "")
        set(WEBCFACE_GENERATOR_COMMAND ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../generator/main.py)
    endif()
    set(WEBCFACE_GENERATED_CPP ${CMAKE_CURRENT_BINARY_DIR}/${WEBCFACE_GENERATE_TARGET}.cpp)
    list(APPEND WEBCFACE_GENERATOR_OPTION
        --namespace ${WEBCFACE_GENERATE_NAMESPACE}
        --file ${WEBCFACE_GENERATE_SOURCES}
        --output ${WEBCFACE_GENERATED_CPP}
    )
    if(NOT "${WEBCFACE_LIBCLANG_PATH}" STREQUAL "")
        list(APPEND WEBCFACE_GENERATOR_OPTION
            --libclang_path ${WEBCFACE_LIBCLANG_PATH}
        )
    endif()
    message("${WEBCFACE_GENERATE_TARGET}")
    message("  namespace: ${WEBCFACE_GENERATE_NAMESPACE}")
    message("  input: ${WEBCFACE_GENERATE_SOURCES}")
    message("  output: ${WEBCFACE_GENERATED_CPP}")
    message("  command: python3 ${WEBCFACE_GENERATOR_COMMAND} ${WEBCFACE_GENERATOR_OPTION}")
    add_custom_command(OUTPUT ${WEBCFACE_GENERATED_CPP}
        COMMAND [ -e `dirname ${WEBCFACE_GENERATED_CPP}` ] || mkdir -p `dirname ${WEBCFACE_GENERATED_CPP}`
        COMMAND python3 ${WEBCFACE_GENERATOR_COMMAND} ${WEBCFACE_GENERATOR_OPTION}
        DEPENDS ${WEBCFACE_GENERATE_SOURCES}
        COMMENT "Generating functions for WebCFace to ${WEBCFACE_GENERATED_CPP}"
    )
    add_library(${WEBCFACE_GENERATE_TARGET} STATIC ${WEBCFACE_GENERATED_CPP})
    target_link_libraries(${WEBCFACE_GENERATE_TARGET} PUBLIC webcface::webcface)
endfunction()
