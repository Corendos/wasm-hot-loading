// Copyright 2023 - Corentin Godeau
// SPDX-License-Identifier: MIT

#pragma once

#include <wasm/module_instance.hpp>
#include <whl/messages.hpp>

namespace whl {

/// Wasm-compatible equivalent of UpdatePayload.
struct WasmUpdatePayload {
  uint32_t name_ptr;
  uint32_t name_len;
  uint32_t value;

  /// Convert from a C++ UpdatePayload to a Wasm compatible struct.
  static WasmUpdatePayload from(const wasm::ModuleInstance &instance,
                                const UpdatePayload &payload);
  /// Convert from a WasmUpdatePayload to a C++ compatible struct.
  static UpdatePayload to(const wasm::ModuleInstance &instance,
                          const WasmUpdatePayload &wasm_payload);
  /// Release any allocated temporary memory.
  static void release(const wasm::ModuleInstance &instance,
                      const WasmUpdatePayload &payload);
};

/// Wasm-compatible equivalent of SampleMessage.
struct WasmSampleMessage {
  SampleMessageType type;
  union {
    /// void noop;
    WasmUpdatePayload update;
    /// void get_state;
  } payload;

  /// Convert from a C++ SampleMessage to a Wasm compatible struct.
  static WasmSampleMessage from(const wasm::ModuleInstance &instance,
                                const SampleMessage &message);
  /// Convert from a WasmSampleMessage to a C++ compatible struct.
  static SampleMessage to(const wasm::ModuleInstance &instance,
                          const WasmSampleMessage &wasm_message);
  /// Release any allocated temporary memory.
  static void release(const wasm::ModuleInstance &instance,
                      const WasmSampleMessage &wasm_message);
};

/// Wasm-compatible equivalent of StatePayload.
using WasmStatePayload = WasmUpdatePayload;

/// Wasm-compatible equivalent of GlobalMessage.
struct WasmGlobalMessage {
  GlobalMessageType type;
  union {
    /// void noop;
    WasmStatePayload state;
  } payload;

  /// Convert from a C++ GlobalMessage to a Wasm compatible struct.
  static WasmGlobalMessage from(const wasm::ModuleInstance &instance,
                                const GlobalMessage &message);
  /// Convert from a WasmGlobalMessage to a C++ compatible struct.
  static GlobalMessage to(const wasm::ModuleInstance &instance,
                          const WasmGlobalMessage &wasm_message);
  /// Release any allocated temporary memory.
  static void release(const wasm::ModuleInstance &instance,
                      const WasmGlobalMessage &wasm_message);
};

} // namespace whl
