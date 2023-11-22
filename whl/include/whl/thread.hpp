// Copyright 2023 - Corentin Godeau
// SPDX-License-Identifier: MIT

#pragma once

#include <functional>
#include <thread>

namespace whl {

class Thread {
public:
  using native_handle_type = std::thread::native_handle_type;

  Thread() noexcept : m_thread{} {}
  Thread(Thread &&other) noexcept : Thread() { swap(other); }
  template <typename Function, typename... Args>
  explicit Thread(Function &&f, Args &&...args)
      : m_thread{&Thread::run<Function, Args...>, std::forward<Function>(f),
                 std::forward<Args>(args)...} {}
  Thread(const Thread &other) = delete;
  ~Thread() = default;

  Thread &operator=(Thread &&other) noexcept {
    swap(other);
    return *this;
  }

  bool joinable() const noexcept { return m_thread.joinable(); }
  std::thread::id get_id() const noexcept { return m_thread.get_id(); }
  std::thread::native_handle_type native_handle() {
    return m_thread.native_handle();
  }

  void join() { m_thread.join(); }
  void detach() { m_thread.detach(); }

  void swap(Thread &other) noexcept {
    using std::swap;
    swap(m_thread, other.m_thread);
  }

  static unsigned int hardware_concurrency() noexcept {
    return std::thread::hardware_concurrency();
  }

  friend void swap(Thread &a, Thread &b) noexcept { a.swap(b); }

private:
  std::thread m_thread;

  template <typename Function, typename... Args>
  static void run(Function &&f, Args &&...args) {
    auto f2 = std::ref(f);
    f2(std::forward<Args>(args)...);
  }
};

} // namespace whl
