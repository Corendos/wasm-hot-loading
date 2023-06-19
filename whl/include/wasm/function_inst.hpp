#pragma once

#include <wasm/exec_env.hpp>
#include <whl/logger.hpp>

#include <wasm_export.h>

#include <cassert>
#include <iostream>
#include <utility>

namespace wasm {

/// Utility struct used to convert value to/from Wasm.
/// By default, it does not provide any implementation so that you can control
/// what conversion take place.
template <typename T> struct ValueConverter {};

/// Converts to/from int32_t
template <> struct ValueConverter<int32_t> {
  static constexpr char signature_char{'i'};
  static wasm_val_t to_wasm_val_t(int32_t input) {
    return {.kind = WASM_I32, .of = {.i32 = input}};
  }
  static int32_t from_wasm_val_t(wasm_val_t input) { return input.of.i32; }
};

/// Converts to/from uint32_t
template <> struct ValueConverter<uint32_t> {
  static constexpr char signature_char{'i'};
  static wasm_val_t to_wasm_val_t(uint32_t input) {
    return {
        .kind = WASM_I32,
        .of = {.i32 = *reinterpret_cast<int32_t *>(&input)},
    };
  }
  static int32_t from_wasm_val_t(wasm_val_t input) {
    return *reinterpret_cast<uint32_t *>(&input.of.i32);
  }
};

/// Converts to/from int64_t
template <> struct ValueConverter<int64_t> {
  static constexpr char signature_char{'I'};
  static wasm_val_t to_wasm_val_t(int64_t input) {
    return {.kind = WASM_I64, .of = {.i64 = input}};
  }
  static int64_t from_wasm_val_t(wasm_val_t input) { return input.of.i64; }
};

/// Converts to/from uint64_t
template <> struct ValueConverter<uint64_t> {
  static constexpr char signature_char{'i'};
  static wasm_val_t to_wasm_val_t(uint64_t input) {
    return {
        .kind = WASM_I64,
        .of = {.i64 = *reinterpret_cast<int64_t *>(&input)},
    };
  }
  static int64_t from_wasm_val_t(wasm_val_t input) {
    return *reinterpret_cast<uint64_t *>(&input.of.i64);
  }
};

/// Converts to/from float
template <> struct ValueConverter<float> {
  static constexpr char signature_char{'f'};
  static wasm_val_t to_wasm_val_t(float input) {
    return {.kind = WASM_F32, .of = {.f32 = input}};
  }
  static float from_wasm_val_t(wasm_val_t input) { return input.of.f32; }
};

/// Converts to/from double
template <> struct ValueConverter<double> {
  static constexpr char signature_char{'F'};
  static wasm_val_t to_wasm_val_t(double input) {
    return {.kind = WASM_F64, .of = {.f64 = input}};
  }
  static double from_wasm_val_t(wasm_val_t input) { return input.of.f64; }
};

/// Converts to/from const char*
template <> struct ValueConverter<const char *> {
  static constexpr char signature_char{'*'};
  static wasm_val_t to_wasm_val_t(const char *input) {
    return {
        .kind = WASM_I32,
        .of = {.i32 = static_cast<int32_t>(reinterpret_cast<size_t>(input))},
    };
  }
  static const char *from_wasm_val_t(wasm_val_t input) {
    return reinterpret_cast<const char *>(input.of.i32);
  }
};

/// Converts to/from void*
template <> struct ValueConverter<void *> {
  static constexpr char signature_char{'*'};
  static wasm_val_t to_wasm_val_t(void *input) {
    return {
        .kind = WASM_ANYREF,
        .of = {.foreign = reinterpret_cast<uintptr_t>(input)},
    };
  }
  static void *from_wasm_val_t(wasm_val_t input) {
    return reinterpret_cast<void *>(input.of.foreign);
  }
};

class ModuleInstance;

/// Base represention of an instance of a function associated with a Wasm
/// module.
/// This is used as a common base for templated version of functions.
class FunctionInstBase {
public:
  /// Constructor / Destructor
  FunctionInstBase() = default;
  ~FunctionInstBase() = default;
  FunctionInstBase(const FunctionInstBase &other) = delete;
  FunctionInstBase(FunctionInstBase &&other);
  FunctionInstBase &operator=(const FunctionInstBase &other) = delete;
  FunctionInstBase &operator=(FunctionInstBase &&other);

  static void swap(FunctionInstBase &a, FunctionInstBase &b);

  /// Returns the underlying WAMR handle.
  wasm_function_inst_t native_handle() const;

protected:
  /// Creates a FunctionInstBase using the underlying WAMR handle.
  FunctionInstBase(wasm_function_inst_t function);

  /// The underlying WAMR handle.
  wasm_function_inst_t m_function{nullptr};
};

/// Represents a function with a non-void return type and variadic arguments.
template <typename R, typename... Args>
class FunctionInst : public FunctionInstBase {
public:
  static constexpr char signature[] = {
      '(',
      ValueConverter<Args>::signature_char...,
      ')',
      ValueConverter<R>::signature_char,
  };

  FunctionInst() = default;
  ~FunctionInst() = default;
  FunctionInst(const FunctionInst &other) = delete;
  FunctionInst(FunctionInst &&other) = default;
  FunctionInst &operator=(const FunctionInst &other) = delete;
  FunctionInst &operator=(FunctionInst &&other) = default;

  R operator()(const ExecEnv &exec_env, Args... raw_args) {
    wasm_val_t args[]{ValueConverter<Args>::to_wasm_val_t(raw_args)...};
    wasm_val_t result[1]{};

    bool call_result =
        wasm_runtime_call_wasm_a(exec_env.native_handle(), m_function, 1,
                                 result, sizeof...(raw_args), args);

    if (!call_result) {
      wasm_module_inst_t instance =
          wasm_runtime_get_module_inst(exec_env.native_handle());
      whl::logger::error("Wasm exception: {}",
                         wasm_runtime_get_exception(instance));
    }

    assert(call_result);

    return ValueConverter<R>::from_wasm_val_t(result[0]);
  }

  friend ModuleInstance;

private:
  FunctionInst(wasm_function_inst_t function) : FunctionInstBase(function) {}
};

/// Represents a function with only variadic arguments.
template <typename... Args>
class FunctionInst<void, Args...> : public FunctionInstBase {
public:
  static constexpr char signature[] = {
      '(', ValueConverter<Args>::signature_char..., ')'};

  FunctionInst() = default;
  ~FunctionInst() = default;
  FunctionInst(const FunctionInst &other) = delete;
  FunctionInst(FunctionInst &&other) = default;
  FunctionInst &operator=(const FunctionInst &other) = delete;
  FunctionInst &operator=(FunctionInst &&other) = default;

  void operator()(const ExecEnv &exec_env, Args... raw_args) {
    wasm_val_t args[]{ValueConverter<Args>::to_wasm_val_t(raw_args)...};

    bool call_result =
        wasm_runtime_call_wasm_a(exec_env.native_handle(), m_function, 0,
                                 nullptr, sizeof...(raw_args), args);
    if (!call_result) {
      wasm_module_inst_t instance =
          wasm_runtime_get_module_inst(exec_env.native_handle());
      whl::logger::error("Wasm exception: {}",
                         wasm_runtime_get_exception(instance));
    }

    assert(call_result);
  }

  friend ModuleInstance;

private:
  FunctionInst(wasm_function_inst_t function) : FunctionInstBase(function) {}
};

} // namespace wasm
