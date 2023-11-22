// Copyright 2023 - Corentin Godeau
// SPDX-License-Identifier: MIT

#pragma once

#include <wasm3.h>

#include <array>
#include <string>

namespace wasm {

class Runtime;

template <typename Ret, typename... Args> class Function {
public:
  Function() = default;
  Function(Function const &other) = delete;
  Function(Function &&other);
  Function &operator=(Function const &other) = delete;
  Function &operator=(Function &&other);
  ~Function();

  IM3Function native_handle() const;

  Ret call(Args... args);

  static void swap(Function &a, Function &b);

private:
  friend Runtime;
  Function(IM3Function function);

  IM3Function m_function{nullptr};
};

template <typename Ret, typename... Args>
Function<Ret, Args...>::Function(IM3Function function) : m_function{function} {}

template <typename Ret, typename... Args> Function<Ret, Args...>::~Function() {}

template <typename Ret, typename... Args>
Function<Ret, Args...>::Function(Function &&other) {
  swap(*this, other);
}

template <typename Ret, typename... Args>
Function<Ret, Args...> &Function<Ret, Args...>::operator=(Function &&other) {
  swap(*this, other);
  return *this;
}

template <typename Ret, typename... Args>
IM3Function Function<Ret, Args...>::native_handle() const {
  return m_function;
}

template <typename Ret, typename... Args>
void Function<Ret, Args...>::swap(Function &a, Function &b) {
  using std::swap;

  swap(a.m_function, b.m_function);
}

template <typename Ret, typename... Args>
Ret Function<Ret, Args...>::call(Args... args) {
  std::array<const void *, sizeof...(args)> arg_ptrs{
      reinterpret_cast<const void *>(&args)...};
  M3Result res = m3_Call(m_function, arg_ptrs.size(), arg_ptrs.data());

  if constexpr (!std::is_void<Ret>::value) {
    Ret ret;
    const void *ret_ptrs[] = {&ret};
    res = m3_GetResults(m_function, 1, ret_ptrs);
    return ret;
  }
}

} // namespace wasm
