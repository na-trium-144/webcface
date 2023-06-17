if("${WEB_NAMESPACE}" STREQUAL "")
    set(WEB_NAMESPACE Shell)
endif()
if(NOT "${WEB_SOURCES}" STREQUAL "")
    # webcface::generate ターゲットをつくる
    # ソースはpythonで生成
    # WEB_SOURCES内の関数を宣言だけして、WebCFaceに登録するaddGenerateFunctions()関数を定義
    # WEB_SOURCES: cppファイル
    # WEB_NAMESPACE: 対象のnamespace
    set(WEB_GENERATED_CPP ${CMAKE_CURRENT_BINARY_DIR}/generated.cpp)
    list(APPEND WEBCFACE_GENERATOR_OPTION
        --namespace ${WEB_NAMESPACE}
        --file ${WEB_SOURCES}
        --output ${WEB_GENERATED_CPP}
    )
    message("webcface::generate")
    message("  namespace: ${WEB_NAMESPACE}")
    message("  input: ${WEB_SOURCES}")
    message("  output: ${WEB_GENERATED_CPP}")
    message("  command: python3 ${WEBCFACE_GENERATOR_COMMAND} ${WEBCFACE_GENERATOR_OPTION}")
    add_custom_command(OUTPUT ${WEB_GENERATED_CPP}
        COMMAND [ -e `dirname ${WEB_GENERATED_CPP}` ] || mkdir -p `dirname ${WEB_GENERATED_CPP}`
        COMMAND python3 ${CMAKE_CURRENT_LIST_DIR}/../../generator/main.py ${WEBCFACE_GENERATOR_OPTION}
        DEPENDS ${WEB_SOURCES}
        COMMENT "Generating functions for WebCFace to ${WEB_GENERATED_CPP}"
    )
    add_library(generate STATIC ${WEB_GENERATED_CPP})
    target_link_libraries(generate PRIVATE ${PROJECT_NAME})
    target_compile_definitions(${PROJECT_NAME}
    PUBLIC
    WEB_USE_GENERATE
    )
    add_library(${PROJECT_NAME}::generate ALIAS generate)
endif()
