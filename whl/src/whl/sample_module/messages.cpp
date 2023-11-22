// Copyright 2023 - Corentin Godeau
// SPDX-License-Identifier: MIT

#include <whl/sample_module/messages.hpp>

namespace whl {

WasmUpdatePayload
WasmUpdatePayload::from(wasm::Function<int32_t, int32_t> &allocate_function,
                        wasm::Runtime &runtime, const UpdatePayload &payload) {
  WasmUpdatePayload wasm_payload{};
  wasm_payload.value = payload.value;

  int32_t wasm_address = allocate_function.call(payload.name_len);
  char *wasm_name =
      reinterpret_cast<char *>(runtime.offset_to_ptr(wasm_address));

  std::string_view name{payload.name_ptr, payload.name_len};
  std::copy(name.begin(), name.end(), wasm_name);
  wasm_payload.name_ptr = wasm_address;
  wasm_payload.name_len = payload.name_len;

  return wasm_payload;
}

UpdatePayload WasmUpdatePayload::to(wasm::Runtime &runtime,
                                    const WasmUpdatePayload &wasm_payload) {
  UpdatePayload payload{};

  payload.value = wasm_payload.value;
  payload.name_ptr = reinterpret_cast<const char *>(
      runtime.offset_to_ptr(wasm_payload.name_ptr));
  payload.name_len = wasm_payload.name_len;

  return payload;
}

void WasmUpdatePayload::release(
    wasm::Function<void, int32_t, int32_t> &free_function,
    const WasmUpdatePayload &payload) {
  free_function.call(payload.name_ptr, payload.name_len);
}

WasmSampleMessage
WasmSampleMessage::from(wasm::Function<int32_t, int32_t> &allocate_function,
                        wasm::Runtime &runtime, const SampleMessage &message) {
  WasmSampleMessage wasm_message{
      .type = message.type,
  };
  switch (message.type) {
  case SampleMessageType::UPDATE:
    wasm_message.payload.update = WasmUpdatePayload::from(
        allocate_function, runtime, message.payload.update);
    break;
  default:
    break;
  }

  return wasm_message;
}

SampleMessage WasmSampleMessage::to(wasm::Runtime &runtime,
                                    const WasmSampleMessage &wasm_message) {
  SampleMessage message{
      .type = wasm_message.type,
  };
  switch (wasm_message.type) {
  case SampleMessageType::UPDATE:
    message.payload.update =
        WasmUpdatePayload::to(runtime, wasm_message.payload.update);
    break;
  default:
    break;
  }

  return message;
}

void WasmSampleMessage::release(
    wasm::Function<void, int32_t, int32_t> &free_function,
    const WasmSampleMessage &wasm_message) {
  switch (wasm_message.type) {
  case SampleMessageType::UPDATE:
    WasmUpdatePayload::release(free_function, wasm_message.payload.update);
    break;
  default:
    break;
  }
}

WasmGlobalMessage
WasmGlobalMessage::from(wasm::Function<int32_t, int32_t> &allocate_function,
                        wasm::Runtime &runtime, const GlobalMessage &message) {
  WasmGlobalMessage wasm_message{
      .type = message.type,
  };
  switch (message.type) {
  case GlobalMessageType::STATE:
    wasm_message.payload.state = WasmStatePayload::from(
        allocate_function, runtime, message.payload.state);
    break;
  default:
    break;
  }

  return wasm_message;
}

GlobalMessage WasmGlobalMessage::to(wasm::Runtime &runtime,
                                    const WasmGlobalMessage &wasm_message) {
  GlobalMessage message{
      .type = wasm_message.type,
  };
  switch (wasm_message.type) {
  case GlobalMessageType::STATE:
    message.payload.state =
        WasmStatePayload::to(runtime, wasm_message.payload.state);
    break;
  default:
    break;
  }

  return message;
}
void WasmGlobalMessage::release(
    wasm::Function<void, int32_t, int32_t> &free_function,
    const WasmGlobalMessage &wasm_message) {
  switch (wasm_message.type) {
  case GlobalMessageType::STATE:
    WasmStatePayload::release(free_function, wasm_message.payload.state);
    break;
  default:
    break;
  }
}

} // namespace whl
