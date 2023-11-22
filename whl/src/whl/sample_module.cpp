// Copyright 2023 - Corentin Godeau
// SPDX-License-Identifier: MIT

#include "wasm3.h"
#include <whl/logger.hpp>
#include <whl/messages_fmt.hpp>
#include <whl/sample_module.hpp>

namespace whl {
SampleModule::SampleModule(
    wasm::Runtime &runtime, SampleModuleConfiguration configuration,
    task::Scheduler<SampleMessage, task::MessageListener<SampleMessage>>
        *scheduler,
    task::Sender<GlobalMessage> *global_sender)
    : m_configuration{std::move(configuration)}, m_scheduler{scheduler},
      m_global_sender{global_sender}, m_runtime{runtime} {
  // NOTE(Corentin): This object must be pinned to memory due to that.
  m_scheduler->set_listener(this);

  runtime.link("env", "print", this, SampleModule::print);
  runtime.link("env", "send_message", this, SampleModule::send_message);
  // module.link("env", "repeat_task", SampleModule::repeat_task);
  // module.link("env", "cancel_task", SampleModule::cancel_task);

  m_init_function = runtime.find_function<int32_t>("init").value();
  m_deinit_function = runtime.find_function<void, int32_t>("deinit").value();
  m_allocate_function =
      runtime.find_function<int32_t, int32_t>("allocate").value();
  m_free_function =
      runtime.find_function<void, int32_t, int32_t>("free").value();
  m_on_message_function =
      runtime.find_function<void, int32_t, int32_t>("on_message").value();
  m_register_scheduler_function =
      runtime
          .find_function<void, int32_t, int32_t, int32_t, int32_t>(
              "register_scheduler")
          .value();

  m_module_context = m_init_function.call();

  register_scheduler(1, "global");
}

SampleModule::~SampleModule() { m_deinit_function.call(m_module_context); }

void SampleModule::on_message(SampleMessage message) {
  int32_t wasm_address = m_allocate_function.call(sizeof(WasmSampleMessage));
  WasmSampleMessage *wasm_message = reinterpret_cast<WasmSampleMessage *>(
      m_runtime.offset_to_ptr(wasm_address));
  *wasm_message =
      WasmSampleMessage::from(m_allocate_function, m_runtime, message);

  m_on_message_function.call(m_module_context, wasm_address);

  WasmSampleMessage::release(m_free_function, *wasm_message);

  m_free_function.call(wasm_address, sizeof(WasmSampleMessage));

  // NOTE(Corentin): We need to free it as this was allocated when received.
  switch (message.type) {
  case SampleMessageType::UPDATE:
    UpdatePayload::free(message.payload.update);
    break;
  default:
    break;
  }
}

void SampleModule::register_scheduler(int32_t scheduler_id,
                                      std::string_view scheduler_name) {
  int32_t wasm_address = m_allocate_function.call(scheduler_name.size());
  char *ptr = reinterpret_cast<char *>(m_runtime.offset_to_ptr(wasm_address));

  std::copy(scheduler_name.begin(), scheduler_name.end(), ptr);

  m_register_scheduler_function.call(m_module_context, scheduler_id,
                                     wasm_address, scheduler_name.size());

  m_free_function.call(wasm_address, scheduler_name.size());
}

int32_t SampleModule::repeat_task(void *userdata, int32_t function_index,
                                  int32_t context, int32_t delay_us) {
  SampleModule *self = reinterpret_cast<SampleModule *>(userdata);

  size_t task_id = self->m_scheduler->schedule_repeatedly(
      std::chrono::microseconds(delay_us), 1,
      [=] {
        // auto local_env = wasm_runtime_get_exec_env_singleton(
        //     m_module_instance.native_handle());
        // uint32_t args[] = {module->m_module_context, userdata};
        // wasm_runtime_call_indirect(local_env, function_index, 2, args);
      },
      0, std::nullopt);

  return static_cast<uint32_t>(task_id);
}

void SampleModule::cancel_task(void *userdata, int32_t task_id) {
  SampleModule *self = reinterpret_cast<SampleModule *>(userdata);
  self->m_scheduler->cancel(static_cast<size_t>(task_id));
}

void SampleModule::send_message(void *userdata, int32_t scheduler_id,
                                void *message_ptr) {
  SampleModule *self = reinterpret_cast<SampleModule *>(userdata);
  if (scheduler_id == 1) {
    if (self->m_global_sender) {
      GlobalMessage global_message = WasmGlobalMessage::to(
          self->m_runtime, *reinterpret_cast<WasmGlobalMessage *>(message_ptr));
      self->m_global_sender->send(global_message);
    }
  } else {
    logger::warning("No sender with id={}", scheduler_id);
  }
}

void SampleModule::print(void *userdata, void *str, int32_t len) {
  logger::info("{}",
               std::string_view(reinterpret_cast<const char *>(str), len));
}

} // namespace whl
