// Copyright 2023 - Corentin Godeau
// SPDX-License-Identifier: MIT

#include <wasm/module.hpp>

#include <utility>

namespace wasm {

Module::Module(Module &&other) { swap(*this, other); }

Module &Module::operator=(Module &&other) {
  swap(*this, other);
  return *this;
}

Module::~Module() {
  if (m_module) {
    m3_FreeModule(m_module);
  }
}

IM3Module Module::native_handle() const { return m_module; }

void Module::swap(Module &a, Module &b) {
  using std::swap;

  swap(a.m_module, b.m_module);
  swap(a.m_module_data, b.m_module_data);
}

Module::Module() {}

Module::Module(IM3Module module, std::string module_data)
    : m_module_data{std::move(module_data)}, m_module{module} {}

} // namespace wasm
