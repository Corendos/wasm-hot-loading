#include <wasm/module.hpp>
#include <whl/logger.hpp>
#include <whl/utils.hpp>

#include <iostream>

namespace wasm {

Module::Module(wasm_module_t module, std::vector<uint8_t> &&data)
    : m_module(module), m_module_data{std::move(data)} {}

Module::~Module() {
  if (m_module) {
    wasm_runtime_unload(m_module);
  }
}

Module::Module(Module &&other) : Module() { swap(*this, other); }

Module &Module::operator=(Module &&other) {
  swap(*this, other);

  return *this;
}
std::optional<ModuleInstance> Module::instantiate(uint32_t stack_size,
                                                  uint32_t heap_size) {
  char error_buffer[1024]{0};

  wasm_module_inst_t result = wasm_runtime_instantiate(
      m_module, stack_size, heap_size, error_buffer, sizeof(error_buffer));
  if (!result) {
    whl::logger::error("Failed to instantiate Wasm module. Reason: {}",
                       error_buffer);

    return std::nullopt;
  }

  return ModuleInstance{result, stack_size, heap_size};
}

void Module::swap(Module &a, Module &b) {
  using std::swap;

  swap(a.m_module, b.m_module);
  swap(a.m_module_data, b.m_module_data);
}

std::optional<Module> Module::load_from_memory(const uint8_t *buffer,
                                               size_t size) {
  const uint8_t *beg{reinterpret_cast<const uint8_t *>(buffer)};
  const uint8_t *end{beg + size};
  std::vector<uint8_t> data(beg, end);

  char error_buffer[1024]{0};

  wasm_module_t result =
      wasm_runtime_load(data.data(), static_cast<uint32_t>(data.size()),
                        error_buffer, sizeof(error_buffer));
  if (!result) {
    whl::logger::error("Failed to load Wasm module. Reason: {}", error_buffer);
    return std::nullopt;
  }

  return Module{result, std::move(data)};
}

std::optional<Module> Module::load_from_file(std::string_view path) {
  std::vector<uint8_t> content = whl::fs::read_file(path);
  return load_from_memory(content.data(), content.size());
}

} // namespace wasm
