// AUTOGENERATED FILE - DO NOT MODIFY!
// This file was generated by Djinni from whl.djinni

#pragma once

#include <cstdint>
#include <string>
#include <utility>

namespace whl {

/** Configuration to instantiate a wasm_hot_loading instance. */
struct Configuration final {
    /** The URL to retrieve the Wasm module to load. */
    std::string sample_module_url;
    /** The public key used to verify module's signature. */
    std::string public_key;
    /** The stack size to use for the Wasm execution environment. */
    int32_t wasm_stack_size;
    /** The heap size to use for the Wasm execution environment. */
    int32_t wasm_heap_size;

    Configuration(std::string sample_module_url_,
                  std::string public_key_,
                  int32_t wasm_stack_size_,
                  int32_t wasm_heap_size_)
    : sample_module_url(std::move(sample_module_url_))
    , public_key(std::move(public_key_))
    , wasm_stack_size(std::move(wasm_stack_size_))
    , wasm_heap_size(std::move(wasm_heap_size_))
    {}

    Configuration()
    : sample_module_url()
    , public_key()
    , wasm_stack_size()
    , wasm_heap_size()
    {}
};

}  // namespace whl