// Copyright 2023 - Corentin Godeau
// SPDX-License-Identifier: MIT

#include "wasm3.h"
#include <wasm/runtime.hpp>

#include <utility>

namespace wasm {

Runtime::Runtime(Runtime &&other) { swap(*this, other); }

Runtime &Runtime::operator=(Runtime &&other) {
  swap(*this, other);
  return *this;
}

Runtime::~Runtime() {
  if (m_runtime) {
    m3_FreeRuntime(m_runtime);
  }
}

void Runtime::swap(Runtime &a, Runtime &b) {
  using std::swap;

  swap(a.m_runtime, b.m_runtime);
  swap(a.m_loaded_module_data, b.m_loaded_module_data);
  swap(a.m_loaded_module, b.m_loaded_module);
  swap(a.m_link_entries, b.m_link_entries);
}

bool Runtime::load_module(Module &&module) {
  // If a module is already loaded, abort.
  if (m_loaded_module) {
    return false;
  }

  M3Result result = m3_LoadModule(m_runtime, module.m_module);
  if (result != m3Err_none) {
    return false;
  }

  using std::swap;

  swap(m_loaded_module, module.m_module);
  swap(m_loaded_module_data, module.m_module_data);

  return true;
}

IM3Runtime Runtime::native_handle() const { return m_runtime; }

void *Runtime::offset_to_ptr(int32_t offset) {
  uint32_t memory_size{0};
  char *mem =
      reinterpret_cast<char *>(m3_GetMemory(m_runtime, &memory_size, 0));

  return mem + offset;
}

int32_t Runtime::ptr_to_offset(void *ptr) {
  uint32_t memory_size{0};
  char *mem =
      reinterpret_cast<char *>(m3_GetMemory(m_runtime, &memory_size, 0));
  return static_cast<int32_t>(reinterpret_cast<char *>(ptr) - mem);
}

Runtime::Runtime(IM3Runtime runtime) : m_runtime{runtime} {}

} // namespace wasm
