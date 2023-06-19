#include <wasm/exec_env.hpp>
#include <wasm/module_instance.hpp>
#include <whl/logger.hpp>

#include <iostream>

namespace wasm {

ExecEnv::ExecEnv(wasm_exec_env_t exec_env) : m_exec_env{exec_env} {}

ExecEnv::~ExecEnv() {
  if (m_exec_env) {
    wasm_runtime_destroy_exec_env(m_exec_env);
  }
}

ExecEnv::ExecEnv(ExecEnv &&other) : ExecEnv{} { swap(*this, other); }

ExecEnv &ExecEnv::operator=(ExecEnv &&other) {
  swap(*this, other);
  return *this;
}

wasm_exec_env_t ExecEnv::native_handle() const { return m_exec_env; }

bool ExecEnv::is_valid() const { return m_exec_env != nullptr; }

std::optional<ExecEnv> ExecEnv::from_instance(const ModuleInstance &instance) {
  wasm_exec_env_t exec_env =
      wasm_runtime_create_exec_env(instance.native_handle(), 1024 * 1024);
  if (!exec_env) {
    whl::logger::error("Failed to create execution environment");
    return std::nullopt;
  }

  return ExecEnv{exec_env};
}

void ExecEnv::swap(ExecEnv &a, ExecEnv &b) {
  using std::swap;

  swap(a.m_exec_env, b.m_exec_env);
}

} // namespace wasm
