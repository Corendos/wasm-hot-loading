// Copyright 2023 - Corentin Godeau
// SPDX-License-Identifier: MIT

#include <whl/logger.hpp>
#include <whl/messages_fmt.hpp>
#include <whl/sample_module.hpp>
#include <whl/sample_module/messages.hpp>

namespace whl {
SampleModule::SampleModule(
    wasm::Module &module, SampleModuleConfiguration configuration,
    task::Scheduler<SampleMessage, task::MessageListener<SampleMessage>>
        *scheduler,
    task::Sender<GlobalMessage> *global_sender)
    : m_configuration{std::move(configuration)}, m_scheduler{scheduler},
      m_global_sender{global_sender} {
  // NOTE(Corentin): This object must be pinned to memory due to that.
  m_scheduler->set_listener(this);

  m_module_instance = module
                          .instantiate(m_configuration.wasm_stack_size,
                                       m_configuration.wasm_heap_size)
                          .value();
  m_exec_env = wasm::ExecEnv::from_instance(m_module_instance).value();

  wasm_runtime_set_custom_data(m_module_instance.native_handle(), this);

  m_init_function = m_module_instance.lookup_function<uint32_t>("init").value();
  m_deinit_function =
      m_module_instance.lookup_function<void, uint32_t>("deinit").value();
  m_on_message_function =
      m_module_instance.lookup_function<void, uint32_t, uint32_t>("on_message")
          .value();
  m_register_scheduler_function =
      m_module_instance
          .lookup_function<void, uint32_t, uint32_t, uint32_t, uint32_t>(
              "register_scheduler")
          .value();

  m_module_context = m_init_function(m_exec_env);
  register_scheduler(1, "global");
}

SampleModule::~SampleModule() {
  m_deinit_function(m_exec_env, m_module_context);
}

void SampleModule::on_message(SampleMessage message) {
  WasmSampleMessage *wasm_message{nullptr};
  uint32_t wasm_address =
      wasm_runtime_module_malloc(m_module_instance.native_handle(),
                                 sizeof(SampleMessage), (void **)&wasm_message);
  *wasm_message = WasmSampleMessage::from(m_module_instance, message);

  m_on_message_function(m_exec_env, m_module_context, wasm_address);

  WasmSampleMessage::release(m_module_instance, *wasm_message);

  wasm_runtime_module_free(m_module_instance.native_handle(), wasm_address);

  // NOTE(Corentin): We need to free it as this was allocated when received.
  switch (message.type) {
  case SampleMessageType::UPDATE:
    UpdatePayload::free(message.payload.update);
    break;
  default:
    break;
  }
}

void SampleModule::register_scheduler(uint32_t scheduler_id,
                                      std::string_view scheduler_name) {
  char *wasm_scheduler_name{nullptr};
  uint32_t wasm_address = wasm_runtime_module_malloc(
      m_module_instance.native_handle(), scheduler_name.size(),
      (void **)&wasm_scheduler_name);
  std::copy(scheduler_name.begin(), scheduler_name.end(), wasm_scheduler_name);

  m_register_scheduler_function(m_exec_env, m_module_context, scheduler_id,
                                wasm_address, scheduler_name.size());

  wasm_runtime_module_free(m_module_instance.native_handle(), wasm_address);
}

bool SampleModule::register_native_exports() {
  // Ensure one time registration.
  static bool registered{false};

  bool result{true};
  if (!registered) {
    result = result && wasm::register_native_functions({
                           EXPORT_WASM_API_WITH_SIG(repeat_task, "(iii)i"),
                           EXPORT_WASM_API_WITH_SIG(cancel_task, "(i)"),
                           EXPORT_WASM_API_WITH_SIG(send_message, "(ii)"),
                           EXPORT_WASM_API_WITH_SIG(print, "(ii)"),
                       });
    if (result) {
      registered = true;
    }
  }

  return result;
}

uint32_t SampleModule::repeat_task(wasm_exec_env_t env, uint32_t function_index,
                                   uint32_t userdata, uint32_t delay_us) {
  wasm_module_inst_t instance = wasm_runtime_get_module_inst(env);
  SampleModule *module =
      reinterpret_cast<SampleModule *>(wasm_runtime_get_custom_data(instance));

  size_t task_id = module->m_scheduler->schedule_repeatedly(
      std::chrono::microseconds(delay_us), 1,
      [=] {
        auto local_env = wasm_runtime_get_exec_env_singleton(
            module->m_module_instance.native_handle());
        uint32_t args[] = {module->m_module_context, userdata};
        wasm_runtime_call_indirect(local_env, function_index, 2, args);
      },
      0, std::nullopt);

  return static_cast<uint32_t>(task_id);
}

void SampleModule::cancel_task(wasm_exec_env_t env, uint32_t task_id) {
  wasm_module_inst_t instance = wasm_runtime_get_module_inst(env);
  SampleModule *module =
      reinterpret_cast<SampleModule *>(wasm_runtime_get_custom_data(instance));

  module->m_scheduler->cancel(static_cast<size_t>(task_id));
}

void SampleModule::send_message(wasm_exec_env_t env, uint32_t scheduler_id,
                                uint32_t message_ptr) {
  wasm_module_inst_t instance = wasm_runtime_get_module_inst(env);
  SampleModule *module =
      reinterpret_cast<SampleModule *>(wasm_runtime_get_custom_data(instance));
  if (scheduler_id == 1) {
    WasmGlobalMessage *raw_global_message =
        (WasmGlobalMessage *)wasm_runtime_addr_app_to_native(instance,
                                                             message_ptr);
    if (module->m_global_sender) {
      GlobalMessage global_message =
          WasmGlobalMessage::to(module->m_module_instance, *raw_global_message);
      module->m_global_sender->send(global_message);
    }
  } else {
    logger::warning("No sender with id={}", scheduler_id);
  }
}

void SampleModule::print(wasm_exec_env_t env, uint32_t wasm_str, uint32_t len) {
  wasm_module_inst_t instance = wasm_runtime_get_module_inst(env);
  const char *str = reinterpret_cast<const char *>(
      wasm_runtime_addr_app_to_native(instance, wasm_str));
  logger::info("{}", std::string_view(str, len));
}

} // namespace whl
