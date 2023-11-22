// Copyright 2023 - Corentin Godeau
// SPDX-License-Identifier: MIT

#include "wasm3.h"
#include <wasm/environment.hpp>

namespace wasm {

Environment::Environment(IM3Environment environment)
    : m_environment{environment} {}

Environment::~Environment() {
  if (m_environment) {
    m3_FreeEnvironment(m_environment);
  }
}

Environment::Environment(Environment &&other) { swap(*this, other); }

Environment &Environment::operator=(Environment &&other) {
  swap(*this, other);
  return *this;
}

std::optional<Runtime> Environment::create_runtime(uint32_t stack_size) {
  IM3Runtime runtime = m3_NewRuntime(m_environment, stack_size, nullptr);
  if (runtime != nullptr) {
    return Runtime(runtime);
  }
  return std::nullopt;
}

std::optional<Module> Environment::parse_module(std::string module_data) {
  IM3Module module{nullptr};
  M3Result result =
      m3_ParseModule(m_environment, &module,
                     reinterpret_cast<const uint8_t *>(module_data.data()),
                     static_cast<uint32_t>(module_data.size()));
  if (result == m3Err_none) {
    return Module(module, std::move(module_data));
  }

  return std::nullopt;
}

IM3Environment Environment::native_handle() const { return m_environment; }

std::optional<Environment> Environment::create() {
  IM3Environment environment = m3_NewEnvironment();
  if (environment != nullptr) {
    return Environment(environment);
  }

  return std::nullopt;
}

void Environment::swap(Environment &a, Environment &b) {
  using std::swap;

  swap(a.m_environment, b.m_environment);
}

} // namespace wasm
