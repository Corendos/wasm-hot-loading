#pragma once

#include <wasm_export.h>

#include <optional>

namespace wasm {

class ModuleInstance;

/// Represents a WAMR execution environment.
class ExecEnv {
public:
  /// Constructor / Destructor
  ExecEnv() = default;
  ~ExecEnv();
  ExecEnv(const ExecEnv &other) = delete;
  ExecEnv(ExecEnv &&other);
  ExecEnv &operator=(const ExecEnv &other) = delete;
  ExecEnv &operator=(ExecEnv &&other);

  /// Returns the underlying WAMR handle.
  wasm_exec_env_t native_handle() const;

  /// Returns true if the environment is valid.
  bool is_valid() const;

  /// Creates an ExecEnv from a ModuleInstance.
  static std::optional<ExecEnv> from_instance(const ModuleInstance &instance);

  static void swap(ExecEnv &a, ExecEnv &b);

private:
  /// Creates an ExecEnv from the underlying WAMR handle.
  ExecEnv(wasm_exec_env_t exec_env);

  /// Underlying WAMR handle.
  wasm_exec_env_t m_exec_env{nullptr};
};

} // namespace wasm
