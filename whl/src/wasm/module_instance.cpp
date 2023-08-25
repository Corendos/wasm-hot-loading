// Copyright 2023 - Corentin Godeau
// SPDX-License-Identifier: MIT

#include <wasm/module_instance.hpp>

namespace wasm {

ModuleInstance::ModuleInstance(wasm_module_inst_t instance, uint32_t stack_size,
                               uint32_t heap_size)
    : m_instance{instance}, m_stack_size{stack_size}, m_heap_size{heap_size} {}

ModuleInstance::ModuleInstance(ModuleInstance &&other) : ModuleInstance() {
  swap(*this, other);
}

ModuleInstance &ModuleInstance::operator=(ModuleInstance &&other) {
  swap(*this, other);

  return *this;
}

wasm_module_inst_t ModuleInstance::native_handle() const { return m_instance; }

uint32_t ModuleInstance::stack_size() const { return m_stack_size; }

uint32_t ModuleInstance::heap_size() const { return m_heap_size; }

void ModuleInstance::swap(ModuleInstance &a, ModuleInstance &b) {
  using std::swap;

  swap(a.m_instance, b.m_instance);
  swap(a.m_stack_size, b.m_stack_size);
  swap(a.m_heap_size, b.m_heap_size);
}

} // namespace wasm
