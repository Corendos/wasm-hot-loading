#include <whl/sample_module/messages.hpp>

namespace whl {

WasmUpdatePayload WasmUpdatePayload::from(const wasm::ModuleInstance &instance,
                                          const UpdatePayload &payload) {
  WasmUpdatePayload wasm_payload{};
  wasm_payload.value = payload.value;

  char *wasm_name_{nullptr};
  uint32_t wasm_address = wasm_runtime_module_malloc(
      instance.native_handle(), payload.name_len, (void **)&wasm_name_);

  std::string_view name{payload.name_ptr, payload.name_len};
  std::copy(name.begin(), name.end(), wasm_name_);
  wasm_payload.name_ptr = wasm_address;
  wasm_payload.name_len = payload.name_len;

  return wasm_payload;
}

UpdatePayload WasmUpdatePayload::to(const wasm::ModuleInstance &instance,
                                    const WasmUpdatePayload &wasm_payload) {
  UpdatePayload payload{};

  payload.value = wasm_payload.value;
  payload.name_ptr = (const char *)wasm_runtime_addr_app_to_native(
      instance.native_handle(), wasm_payload.name_ptr);
  payload.name_len = wasm_payload.name_len;

  return payload;
}

void WasmUpdatePayload::release(const wasm::ModuleInstance &instance,
                                const WasmUpdatePayload &payload) {
  wasm_runtime_module_free(instance.native_handle(), payload.name_ptr);
}

WasmSampleMessage WasmSampleMessage::from(const wasm::ModuleInstance &instance,
                                          const SampleMessage &message) {
  WasmSampleMessage wasm_message{
      .type = message.type,
  };
  switch (message.type) {
  case SampleMessageType::UPDATE:
    wasm_message.payload.update =
        WasmUpdatePayload::from(instance, message.payload.update);
    break;
  default:
    break;
  }

  return wasm_message;
}

SampleMessage WasmSampleMessage::to(const wasm::ModuleInstance &instance,
                                    const WasmSampleMessage &wasm_message) {
  SampleMessage message{
      .type = wasm_message.type,
  };
  switch (wasm_message.type) {
  case SampleMessageType::UPDATE:
    message.payload.update =
        WasmUpdatePayload::to(instance, wasm_message.payload.update);
    break;
  default:
    break;
  }

  return message;
}

void WasmSampleMessage::release(const wasm::ModuleInstance &instance,
                                const WasmSampleMessage &wasm_message) {
  switch (wasm_message.type) {
  case SampleMessageType::UPDATE:
    WasmUpdatePayload::release(instance, wasm_message.payload.update);
    break;
  default:
    break;
  }
}

WasmGlobalMessage WasmGlobalMessage::from(const wasm::ModuleInstance &instance,
                                          const GlobalMessage &message) {
  WasmGlobalMessage wasm_message{
      .type = message.type,
  };
  switch (message.type) {
  case GlobalMessageType::STATE:
    wasm_message.payload.state =
        WasmStatePayload::from(instance, message.payload.state);
    break;
  default:
    break;
  }

  return wasm_message;
}

GlobalMessage WasmGlobalMessage::to(const wasm::ModuleInstance &instance,
                                    const WasmGlobalMessage &wasm_message) {
  GlobalMessage message{
      .type = wasm_message.type,
  };
  switch (wasm_message.type) {
  case GlobalMessageType::STATE:
    message.payload.state =
        WasmStatePayload::to(instance, wasm_message.payload.state);
    break;
  default:
    break;
  }

  return message;
}
void WasmGlobalMessage::release(const wasm::ModuleInstance &instance,
                                const WasmGlobalMessage &wasm_message) {
  switch (wasm_message.type) {
  case GlobalMessageType::STATE:
    WasmStatePayload::release(instance, wasm_message.payload.state);
    break;
  default:
    break;
  }
}

} // namespace whl
