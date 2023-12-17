// Copyright 2023 - Corentin Godeau
// SPDX-License-Identifier: MIT

#pragma once

#include <wasm/module.hpp>
#include <wasm/runtime.hpp>

#include <whl/messages.hpp>
#include <whl/sample_module/messages.hpp>
#include <whl/task/scheduler.hpp>
#include <whl/task/sender.hpp>

#include <fmt/format-inl.h>

namespace whl {

/// Configuration for the SampleModule Wasm runtime.
struct SampleModuleConfiguration {
  /// The size of the stack.
  uint32_t wasm_stack_size{1024 * 1024};
};

class SampleModule : public task::MessageListener<SampleMessage> {
public:
  SampleModule(wasm::Runtime &runtime, SampleModuleConfiguration configuration,
               task::Scheduler<SampleMessage,
                               task::MessageListener<SampleMessage>> *scheduler,
               task::Sender<GlobalMessage> *global_sender);
  ~SampleModule();

  void on_message(SampleMessage message) override;

private:
  static int32_t repeat_task(void *userdata, int32_t function_index,
                             int32_t context, int32_t delay_us);
  static void cancel_task(void *userdata, int32_t task_id);
  static void send_message(void *userdata, int32_t scheduler_id,
                           void *message_ptr);
  static void print(void *userdata, void *wasm_str, int32_t len);

  void init_exec_env();

  void register_scheduler(int32_t scheduler_id,
                          std::string_view scheduler_name);

  SampleModuleConfiguration m_configuration;
  wasm::Runtime &m_runtime;
  wasm::Function<int32_t> m_init_function;
  wasm::Function<void, int32_t> m_deinit_function;
  wasm::Function<int32_t, int32_t> m_allocate_function;
  wasm::Function<void, int32_t, int32_t> m_free_function;
  wasm::Function<void, int32_t, int32_t> m_on_message_function;
  wasm::Function<void, int32_t, int32_t, int32_t, int32_t>
      m_register_scheduler_function;
  wasm::Function<void, int32_t, int32_t, int32_t> m_call_function;
  int32_t m_module_context;

  task::Scheduler<SampleMessage, task::MessageListener<SampleMessage>>
      *m_scheduler{nullptr};
  task::Sender<GlobalMessage> *m_global_sender{nullptr};
};

} // namespace whl
