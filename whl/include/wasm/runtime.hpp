#pragma once

#include <wasm_export.h>

#include <forward_list>
#include <vector>

namespace wasm {

/// Registers given symbols to WAMR.
bool register_native_functions(std::vector<NativeSymbol> symbols);

/// Stores already registered symbols.
extern std::forward_list<std::vector<NativeSymbol>> m_registered_symbols;

} // namespace wasm
