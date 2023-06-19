# Set build platform for WAMR
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set (WAMR_BUILD_PLATFORM "linux")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Android")
    set (WAMR_BUILD_PLATFORM "android")
else()
    message(FATAL_ERROR "Unknown system: ${CMAKE_SYSTEM_NAME}")
endif()

# Set build target architecture for WAMR
if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64")
    set (WAMR_BUILD_TARGET "X86_64")
elseif (${CMAKE_SYSTEM_PROCESSOR} MATCHES "i686")
    set (WAMR_BUILD_TARGET "X86_32")
elseif (${CMAKE_SYSTEM_PROCESSOR} MATCHES "armv7-a")
    set (WAMR_BUILD_TARGET "ARMV7")
elseif (${CMAKE_SYSTEM_PROCESSOR} MATCHES "aarch64")
    set (WAMR_BUILD_TARGET "AARCH64V8")
else()
    message(FATAL_ERROR "Unknown arch: ${CMAKE_SYSTEM_PROCESSOR}")
endif()

# Various WAMR options
set (WAMR_BUILD_INTERP 1)
set (WAMR_BUILD_FAST_INTERP 1)
set (WAMR_BUILD_AOT 1)
set (WAMR_BUILD_LIBC_BUILTIN 1)
set (WAMR_BUILD_LIBC_WASI 1)
set (WAMR_BUILD_SIMD 1)
set (WAMR_ROOT_DIR "${CMAKE_SOURCE_DIR}/deps/wasm-micro-runtime")
set (WAMR_DISABLE_STACK_HW_BOUND_CHECK 1)

# Declare WAMR as a static library
include (${WAMR_ROOT_DIR}/build-scripts/runtime_lib.cmake)
add_library(wamr_vmlib STATIC ${WAMR_RUNTIME_LIB_SOURCE})
add_library(WAMR::vmlib ALIAS wamr_vmlib)

# On Android, liblog is required
if (${CMAKE_SYSTEM_NAME} MATCHES "Android")
    target_link_libraries(wamr_vmlib PUBLIC log)
endif()
