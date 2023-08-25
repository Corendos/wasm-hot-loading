// Copyright 2023 - Corentin Godeau
// SPDX-License-Identifier: MIT

#pragma once

#include <wasm/function_inst.hpp>

#include <wasm_export.h>

#include <iostream>
#include <optional>

namespace wasm {

class Module;

/// Represents a WAMR Module instance.
class ModuleInstance {
public:
  /// Constructor / Destructor
  ModuleInstance() = default;
  ~ModuleInstance() = default;
  ModuleInstance(const ModuleInstance &other) = delete;
  ModuleInstance(ModuleInstance &&other);
  ModuleInstance &operator=(const ModuleInstance &other) = delete;
  ModuleInstance &operator=(ModuleInstance &&other);

  /// Returns the underlying WAMR handle.
  wasm_module_inst_t native_handle() const;
  /// Returns the stack size of this instance.
  uint32_t stack_size() const;
  /// Returms the heap size of this instance.
  uint32_t heap_size() const;

  /// Try to get a handle on a function exported by the associated Wasm Module
  /// using it's name and optionnally its signature.
  template <typename R, typename... Args>
  std::optional<FunctionInst<R, Args...>>
  lookup_function(const char *name, const char *signature = nullptr) {
    wasm_function_inst_t function =
        wasm_runtime_lookup_function(m_instance, name, signature);
    if (!function) {
      whl::logger::error("Failed to retrieve \"{}\" function from module",
                         name);
      return std::nullopt;
    }

    return FunctionInst<R, Args...>{function};
  }

  static void swap(ModuleInstance &a, ModuleInstance &b);

  friend Module;

private:
  /// Creates a ModuleInstance from the underlying WAMR handle and required
  /// parameter.
  ModuleInstance(wasm_module_inst_t instance, uint32_t stack_size,
                 uint32_t heap_size);

  /// Unerlying WAMR handle.
  wasm_module_inst_t m_instance{nullptr};
  /// The instance stack size.
  uint32_t m_stack_size{};
  /// The instance heap size.
  uint32_t m_heap_size{};
};

} // namespace wasm
