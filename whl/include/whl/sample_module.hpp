#pragma once

#include <wasm/exec_env.hpp>
#include <wasm/function_inst.hpp>
#include <wasm/module.hpp>
#include <wasm/runtime.hpp>
#include <whl/messages.hpp>
#include <whl/task/scheduler.hpp>
#include <whl/task/sender.hpp>

#include <fmt/format-inl.h>
#include <wasm_export.h>

namespace whl {

/// Configuration for the SampleModule Wasm runtime.
struct SampleModuleConfiguration {
  /// The size of the stack.
  uint32_t wasm_stack_size{1024 * 1024};
  /// The size of the heap.
  uint32_t wasm_heap_size{64 * 1024};
};

class SampleModule : public task::MessageListener<SampleMessage> {
public:
  SampleModule(wasm::Module &module, SampleModuleConfiguration configuration,
               task::Scheduler<SampleMessage,
                               task::MessageListener<SampleMessage>> *scheduler,
               task::Sender<GlobalMessage> *global_sender);
  ~SampleModule();

  void on_message(SampleMessage message) override;

  static bool register_native_exports();

private:
  static uint32_t repeat_task(wasm_exec_env_t env, uint32_t function_index,
                              uint32_t context, uint32_t delay_us);
  static void cancel_task(wasm_exec_env_t env, uint32_t task_id);

  static void send_message(wasm_exec_env_t env, uint32_t scheduler_id,
                           uint32_t message_ptr);
  static void print(wasm_exec_env_t env, uint32_t wasm_str, uint32_t len);

  void init_exec_env();

  void register_scheduler(uint32_t scheduler_id,
                          std::string_view scheduler_name);

  SampleModuleConfiguration m_configuration;
  wasm::ModuleInstance m_module_instance;
  wasm::ExecEnv m_exec_env;
  wasm::FunctionInst<uint32_t> m_init_function;
  wasm::FunctionInst<void, uint32_t> m_deinit_function;
  wasm::FunctionInst<void, uint32_t, uint32_t> m_on_message_function;
  wasm::FunctionInst<void, uint32_t, uint32_t, uint32_t, uint32_t>
      m_register_scheduler_function;
  uint32_t m_module_context;

  task::Scheduler<SampleMessage, task::MessageListener<SampleMessage>>
      *m_scheduler{nullptr};
  task::Sender<GlobalMessage> *m_global_sender{nullptr};
};

} // namespace whl
