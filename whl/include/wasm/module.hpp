// Copyright 2023 - Corentin Godeau
// SPDX-License-Identifier: MIT

#pragma once

#include <wasm3.h>

#include <string>

namespace wasm {

class Environment;
class Runtime;

class Module {
public:
  Module(Module const &other) = delete;
  Module(Module &&other);
  Module &operator=(Module const &other) = delete;
  Module &operator=(Module &&other);
  ~Module();

  IM3Module native_handle() const;

  static void swap(Module &a, Module &b);

private:
  friend Environment;
  friend Runtime;

  Module();
  Module(IM3Module module, std::string module_data);

  std::string m_module_data{};
  IM3Module m_module{nullptr};
};

} // namespace wasm
