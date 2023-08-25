// Copyright 2023 - Corentin Godeau
// SPDX-License-Identifier: MIT

#include <wasm/runtime.hpp>

#include <cassert>

namespace wasm {

std::forward_list<std::vector<NativeSymbol>> m_registered_symbols{};

bool register_native_functions(std::vector<NativeSymbol> symbols) {
  m_registered_symbols.push_front(std::move(symbols));
  return wasm_runtime_register_natives(
      "env", m_registered_symbols.front().data(),
      static_cast<uint32_t>(m_registered_symbols.front().size()));
}

} // namespace wasm
