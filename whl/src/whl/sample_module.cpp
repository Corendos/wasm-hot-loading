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

  // Link wasm-callable functions
  runtime.link("env", "print", this, SampleModule::print);
  runtime.link("env", "sendMessage", this, SampleModule::send_message);
  runtime.link("env", "repeatTask", this, SampleModule::repeat_task);
  runtime.link("env", "cancelTask", this, SampleModule::cancel_task);

  // Retrieve wasm-defined function
  m_init_function = runtime.find_function<int32_t>("init").value();
  m_deinit_function = runtime.find_function<void, int32_t>("deinit").value();
  m_allocate_function =
      runtime.find_function<int32_t, int32_t>("allocate").value();
  m_free_function =
      runtime.find_function<void, int32_t, int32_t>("free").value();
  m_on_message_function =
      runtime.find_function<void, int32_t, int32_t>("onMessage").value();
  m_register_scheduler_function =
      runtime
          .find_function<void, int32_t, int32_t, int32_t, int32_t>(
              "registerScheduler")
          .value();
  m_call_function =
      runtime.find_function<void, int32_t, int32_t, int32_t>("call").value();

  // Call the Wasm module "init" function.
  m_module_context = m_init_function.call();

  // Associate the global scheduler to the ID 1.
  register_scheduler(1, "global");
}

SampleModule::~SampleModule() {
  // Call the Wasm module "deinit" function.
  m_deinit_function.call(m_module_context);
}

void SampleModule::on_message(SampleMessage message) {
  // Conver the incoming message to a WASM-compatible message.
  WasmSampleMessage wasm_message =
      WasmSampleMessage::from(m_allocate_function, m_runtime, message);
  // Allocate space in the WASM memory for this message.
  int32_t wasm_address = m_allocate_function.call(sizeof(WasmSampleMessage));
  // Stores the message in the allocated slot.
  *reinterpret_cast<WasmSampleMessage *>(
      m_runtime.offset_to_ptr(wasm_address)) = wasm_message;

  // Call the "onMessage" function from the WASM module.
  m_on_message_function.call(m_module_context, wasm_address);

  // Release the allocated space for the message.
  WasmSampleMessage::release(m_free_function, wasm_message);
  m_free_function.call(wasm_address, sizeof(WasmSampleMessage));

  switch (message.type) {
  case SampleMessageType::UPDATE:
    // NOTE(Corentin): We need to free it as this was allocated when received.
    //                 See WasmHotLoadingImpl::update() method.
    UpdatePayload::free(message.payload.update);
    break;
  default:
    break;
  }
}

void SampleModule::register_scheduler(int32_t scheduler_id,
                                      std::string_view scheduler_name) {
  // Allocate space in WASM memory to temporarily store the scheduler name.
  int32_t wasm_address = m_allocate_function.call(scheduler_name.size());
  char *ptr = reinterpret_cast<char *>(m_runtime.offset_to_ptr(wasm_address));

  std::copy(scheduler_name.begin(), scheduler_name.end(), ptr);

  // Call the "registerScheduler" function from the WASM module.
  m_register_scheduler_function.call(m_module_context, scheduler_id,
                                     wasm_address, scheduler_name.size());

  // Release the allocated space.
  m_free_function.call(wasm_address, scheduler_name.size());
}

int32_t SampleModule::repeat_task(void *userdata, int32_t function_index,
                                  int32_t context, int32_t delay_us) {
  SampleModule *self = reinterpret_cast<SampleModule *>(userdata);

  // Register a task to be called repeatedly on our scheduler.
  size_t task_id = self->m_scheduler->schedule_repeatedly(
      std::chrono::microseconds(delay_us), 1,
      [=] {
        self->m_call_function.call(function_index, self->m_module_context,
                                   context);
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
  // GlobalScheduler is associate with ID 1.
  if (scheduler_id == 1) {
    if (self->m_global_sender) {
      // Convert the WASM-compatible message to a C++-compatible message.
      GlobalMessage global_message = WasmGlobalMessage::to(
          self->m_runtime, *reinterpret_cast<WasmGlobalMessage *>(message_ptr));
      // Send it to the GlobalScheduler
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
