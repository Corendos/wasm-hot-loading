#pragma once

#include <functional>

namespace whl {

/// Wrapper object that will run a given callback on destruction.
class Defer {
public:
  Defer(std::function<void()> &&f) : m_callable(std::move(f)) {}
  ~Defer() {
    if (m_callable) {
      m_callable();
    }
  }

  Defer(const Defer &other) = delete;
  Defer(Defer &&other) = delete;
  Defer &operator=(const Defer &other) = delete;
  Defer &operator=(Defer &&other) = delete;

private:
  /// The function to execute.
  std::function<void()> m_callable{};
};
} // namespace whl

/// Convenience define to easily define something we want to defer to scope
/// exit.
#define DEFER_CONCAT(x, y) x##y
#define DEFER_VAR_NAME(x, y) DEFER_CONCAT(x, y)
#define DEFER(block)                                                           \
  whl::Defer DEFER_VAR_NAME(defer_, __COUNTER__)([&] { block })
