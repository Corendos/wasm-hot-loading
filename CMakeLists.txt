# Copyright 2023 - Corentin Godeau
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.19)

project(wasm-hot-loading)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")


set(BUILD_SHARED_LIBS OFF)

set(CMAKE_CXX_FLAGS "-Wno-unused-result")

if (ANDROID)
    set(DJINNI_BUILD_TESTING OFF CACHE INTERNAL "")
    set(DJINNI_WITH_JNI ON CACHE INTERNAL "")
    set(JNI_CPP_THREAD_ATTACH ON CACHE INTERNAL "")
    add_subdirectory(deps/djinni-support-lib)
else()
endif()

add_subdirectory(deps/boringssl)
set(CURL_ENABLE_SSL OFF)
add_subdirectory(deps/curl)
add_subdirectory(deps/CLI11)
add_subdirectory(deps/fmt)

include(cmake/wamr_lib.cmake)

add_subdirectory(whl)
add_subdirectory(platform)
