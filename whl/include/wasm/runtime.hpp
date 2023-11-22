// Copyright 2023 - Corentin Godeau
// SPDX-License-Identifier: MIT

#pragma once

#include <wasm/function.hpp>
#include <wasm/module.hpp>
#include <whl/logger.hpp>

#include <wasm3.h>

#include <forward_list>
#include <optional>
#include <tuple>

namespace wasm {

class Environment;

template <char c> struct ParameterSignature {
  static const char value = c;
};
template <typename T, typename = void> struct TypeToSignature;

template <typename T, typename U>
using is_enum_of_t = typename std::enable_if<
    std::is_enum<T>::value &&
    std::is_same<std::underlying_type_t<T>, U>::value>::type;

template <typename T>
struct TypeToSignature<T, is_enum_of_t<T, int32_t>> : ParameterSignature<'i'> {
};

template <class T>
struct TypeToSignature<T, is_enum_of_t<T, int64_t>> : ParameterSignature<'I'> {
};

template <> struct TypeToSignature<int32_t> : ParameterSignature<'i'> {};
template <> struct TypeToSignature<int64_t> : ParameterSignature<'I'> {};
template <> struct TypeToSignature<float> : ParameterSignature<'f'> {};
template <> struct TypeToSignature<double> : ParameterSignature<'F'> {};
template <> struct TypeToSignature<void> : ParameterSignature<'v'> {};
template <> struct TypeToSignature<void *> : ParameterSignature<'*'> {};
template <> struct TypeToSignature<const void *> : ParameterSignature<'*'> {};

template <typename Ret, typename... Args> struct FunctionSignature {
  constexpr static size_t n_args = sizeof...(Args);
  constexpr static const char value[n_args + 4] = {
      TypeToSignature<Ret>::value, '(', TypeToSignature<Args>::value..., ')',
      0};
};

class Runtime;

struct LinkEntry {
  void *userdata;
  void *function_ptr;
};

template <typename T>
void arg_from_stack(T &dest, uint64_t *&_sp, [[maybe_unused]] void *mem) {
  m3ApiGetArg(T, tmp);
  dest = tmp;
}

template <typename T>
void arg_from_stack(T *&dest, uint64_t *&_sp, [[maybe_unused]] void *_mem) {
  m3ApiGetArgMem(T *, tmp);
  dest = tmp;
};

template <typename T>
void arg_from_stack(const T *&dest, uint64_t *&_sp,
                    [[maybe_unused]] void *_mem) {
  m3ApiGetArgMem(const T *, tmp);
  dest = tmp;
};

template <typename... Args>
static void get_args_from_stack(uint64_t *&sp, void *mem,
                                std::tuple<Args...> &tuple) {
  std::apply([&](auto &...item) { (arg_from_stack(item, sp, mem), ...); },
             tuple);
}

template <typename Func> struct WrapHelper;

template <typename Ret, typename... Args> struct WrapHelper<Ret(Args...)> {
  using Func = Ret(void *, Args...);
  static const void *wrap_fn([[maybe_unused]] IM3Runtime runtime,
                             IM3ImportContext context, uint64_t *_sp,
                             void *mem) {
    std::tuple<Args...> args;
    // The order here matters: m3ApiReturnType should go before calling
    // get_args_from_stack, since both modify `_sp`, and the return value on the
    // stack is reserved before the arguments.
    m3ApiReturnType(Ret);
    get_args_from_stack(_sp, mem, args);
    LinkEntry *link_entry = reinterpret_cast<LinkEntry *>(context->userdata);
    Func *function = reinterpret_cast<Func *>(link_entry->function_ptr);

    auto parameters = std::tuple_cat(std::tuple<void *>(link_entry->userdata),
                                     std::move(args));
    Ret r = std::apply(function, parameters);
    m3ApiReturn(r);
  }
};

template <typename... Args> struct WrapHelper<void(Args...)> {
  using Func = void(void *, Args...);
  static const void *wrap_fn([[maybe_unused]] IM3Runtime runtime,
                             IM3ImportContext context, uint64_t *_sp,
                             void *mem) {
    std::tuple<Args...> args;
    get_args_from_stack(_sp, mem, args);
    LinkEntry *link_entry = reinterpret_cast<LinkEntry *>(context->userdata);
    Func *function = reinterpret_cast<Func *>(link_entry->function_ptr);

    auto parameters = std::tuple_cat(std::tuple<void *>(link_entry->userdata),
                                     std::move(args));

    std::apply(function, parameters);
    m3ApiSuccess();
  }
};

class Runtime {
public:
  Runtime(Runtime const &other) = delete;
  Runtime(Runtime &&other);
  Runtime &operator=(Runtime const &other) = delete;
  Runtime &operator=(Runtime &&other);
  ~Runtime();

  static void swap(Runtime &a, Runtime &b);

  bool load_module(Module &&module);

  IM3Runtime native_handle() const;

  template <typename Ret, typename... Args>
  void link(const char *module_name, const char *function_name, void *userdata,
            Ret (*function)(void *, Args...));

  template <typename Ret, typename... Args>
  std::optional<Function<Ret, Args...>>
  find_function(const char *function_name);
  void *offset_to_ptr(int32_t offset);
  int32_t ptr_to_offset(void *ptr);

private:
  friend Environment;

  Runtime(IM3Runtime runtime);

  IM3Runtime m_runtime{nullptr};

  std::string m_loaded_module_data{};
  IM3Module m_loaded_module{nullptr};

  std::forward_list<LinkEntry> m_link_entries;
};

template <typename Ret, typename... Args>
void Runtime::link(const char *module_name, const char *function_name,
                   void *userdata, Ret (*function)(void *, Args...)) {
  if (m_loaded_module == nullptr) {
    return;
  }

  m_link_entries.push_front(
      LinkEntry{userdata, reinterpret_cast<void *>(function)});
  LinkEntry *entry_ptr = &m_link_entries.front();

  m3_LinkRawFunctionEx(m_loaded_module, module_name, function_name,
                       FunctionSignature<Ret, Args...>::value,
                       &WrapHelper<Ret(Args...)>::wrap_fn,
                       reinterpret_cast<void *>(entry_ptr));
}

template <typename Ret, typename... Args>
std::optional<Function<Ret, Args...>>
Runtime::find_function(const char *function_name) {
  IM3Function function{nullptr};
  M3Result result = m3_FindFunction(&function, m_runtime, function_name);
  if (result == m3Err_none) {
    return Function<Ret, Args...>(function);
  }
  return std::nullopt;
}
} // namespace wasm
