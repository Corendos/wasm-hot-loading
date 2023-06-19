set(DJINNI_PLATFORM_SOURCES "")

if (ANDROID)
    list(APPEND DJINNI_PLATFORM_SOURCES
        jni/djinni_jni_main.cpp
        jni/configuration.cpp
        jni/wasm_hot_loading.cpp
        jni/state_listener.cpp
    )
    set(ADDITIONAL_LIBS "djinni-support-lib::djinni-support-lib")
endif()

add_library(whlstatic STATIC
    cpp/wasm_hot_loading.cpp
    ${DJINNI_PLATFORM_SOURCES}
)
target_link_libraries(whlstatic PUBLIC whlcore)
target_include_directories(whlstatic PUBLIC include)

if (ANDROID)
add_library(whl SHARED
    cpp/wasm_hot_loading.cpp
    ${DJINNI_PLATFORM_SOURCES}
)
target_link_libraries(whl PUBLIC whlcore ${ADDITIONAL_LIBS})
target_include_directories(whl PUBLIC include)
endif()
