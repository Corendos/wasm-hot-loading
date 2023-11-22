// Copyright 2023 - Corentin Godeau
// SPDX-License-Identifier: MIT

#pragma once

#include <wasm/function.hpp>
#include <wasm/runtime.hpp>
#include <whl/messages.hpp>

namespace whl {

/// Wasm-compatible equivalent of UpdatePayload.
struct WasmUpdatePayload {
  uint32_t name_ptr;
  uint32_t name_len;
  uint32_t value;

  /// Convert from a C++ UpdatePayload to a Wasm compatible struct.
  static WasmUpdatePayload
  from(wasm::Function<int32_t, int32_t> &allocate_function,
       wasm::Runtime &runtime, const UpdatePayload &payload);
  /// Convert from a WasmUpdatePayload to a C++ compatible struct.
  static UpdatePayload to(wasm::Runtime &runtime,
                          const WasmUpdatePayload &wasm_payload);
  /// Release any allocated temporary memory.
  static void release(wasm::Function<void, int32_t, int32_t> &free_function,
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
  static WasmSampleMessage
  from(wasm::Function<int32_t, int32_t> &allocate_function,
       wasm::Runtime &runtime, const SampleMessage &message);
  /// Convert from a WasmSampleMessage to a C++ compatible struct.
  static SampleMessage to(wasm::Runtime &runtime,
                          const WasmSampleMessage &wasm_message);
  /// Release any allocated temporary memory.
  static void release(wasm::Function<void, int32_t, int32_t> &free_function,
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
  static WasmGlobalMessage
  from(wasm::Function<int32_t, int32_t> &allocate_function,
       wasm::Runtime &runtime, const GlobalMessage &message);
  /// Convert from a WasmGlobalMessage to a C++ compatible struct.
  static GlobalMessage to(wasm::Runtime &runtime,
                          const WasmGlobalMessage &wasm_message);
  /// Release any allocated temporary memory.
  static void release(wasm::Function<void, int32_t, int32_t> &free_function,
                      const WasmGlobalMessage &wasm_message);
};

} // namespace whl
