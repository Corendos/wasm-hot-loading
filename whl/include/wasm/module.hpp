#pragma once

#include <wasm_export.h>

#include <wasm/module_instance.hpp>

#include <optional>
#include <string_view>
#include <vector>

namespace wasm {

/// Represents a WAMR Wasm module.
class Module {
public:
  /// Constructor / Destructor
  Module() = default;
  Module(const Module &other) = delete;
  Module(Module &&other);
  Module &operator=(const Module &other) = delete;
  Module &operator=(Module &&other);

  ~Module();

  /// Instantiate an instance of this module.
  std::optional<ModuleInstance> instantiate(uint32_t stack_size,
                                            uint32_t heap_size);

  /// Try to load a Wasm module using the given buffer and its size. Returns
  /// std::nullopt on error.
  static std::optional<Module> load_from_memory(const uint8_t *buffer,
                                                size_t size);
  /// Try to load a Wasm module using the given filename. Returns std::nullopt
  /// on error.
  static std::optional<Module> load_from_file(std::string_view path);

  static void swap(Module &a, Module &b);

private:
  /// Creates a Module given the underlying WAMR handle and the loaded data.
  Module(wasm_module_t module, std::vector<uint8_t> &&data);

  /// Underlying WAMR handle.
  wasm_module_t m_module{nullptr};
  /// The binary data of the Module.
  std::vector<uint8_t> m_module_data{};
};

} // namespace wasm
