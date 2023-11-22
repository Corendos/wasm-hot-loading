// Copyright 2023 - Corentin Godeau
// SPDX-License-Identifier: MIT

#pragma once

#include <wasm/module.hpp>
#include <wasm/runtime.hpp>

#include <wasm3.h>

#include <optional>
#include <string>

namespace wasm {

class Environment {
public:
  Environment(Environment const &other) = delete;
  Environment(Environment &&other);
  Environment &operator=(Environment const &other) = delete;
  Environment &operator=(Environment &&other);
  ~Environment();

  std::optional<Runtime> create_runtime(uint32_t stack_size);
  std::optional<Module> parse_module(std::string module_data);

  IM3Environment native_handle() const;

  static std::optional<Environment> create();

  static void swap(Environment &a, Environment &b);

private:
  Environment(IM3Environment environment);

  IM3Environment m_environment{nullptr};
};

} // namespace wasm
