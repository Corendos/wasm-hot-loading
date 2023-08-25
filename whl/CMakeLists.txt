# Copyright 2023 - Corentin Godeau
# SPDX-License-Identifier: MIT

set(WHL_SOURCES
    src/whl/task/task_scheduler.cpp
    src/whl/network/http.cpp
    src/whl/utils.cpp
    src/whl/messages.cpp
    src/whl/sample_module.cpp
    src/whl/sample_module/messages.cpp
)

if (ANDROID)
    list(APPEND WHL_SOURCES src/whl/logger/android.cpp)
else()
    list(APPEND WHL_SOURCES src/whl/logger/default.cpp)
endif()

set(WASM_SOURCES
    src/wasm/runtime.cpp
    src/wasm/module.cpp
    src/wasm/module_instance.cpp
    src/wasm/exec_env.cpp
    src/wasm/function_inst.cpp
)

add_library(whlcore STATIC ${WHL_SOURCES} ${WASM_SOURCES})
target_include_directories(whlcore PUBLIC include)
target_link_libraries(whlcore PUBLIC WAMR::vmlib fmt::fmt-header-only libcurl crypto)
if(ANDROID)
target_link_libraries(whlcore PUBLIC log)
endif()

