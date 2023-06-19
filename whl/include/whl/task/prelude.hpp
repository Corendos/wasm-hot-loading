#pragma once

#include <chrono>
#include <cstddef>
#include <functional>
#include <memory>
#include <optional>

namespace whl {
namespace task {

/// Signature of function that can be scheduled on a Scheduler.
using TaskFn = std::function<void()>;

/// Signature of function that are executed when tasks are cancelled.
using StopFn = std::function<void(bool isCancellation)>;

/// Represents a unique identifier for Tasks.
using TaskId = std::size_t;

/// Represents a unique identifier for groups of Tasks.
using GroupId = std::size_t;

/// Represents a priority of a given task.
using Priority = uint8_t;

/// Function providing current time.
using TimeFn = std::function<std::chrono::steady_clock::time_point()>;

/// Function providing a way to sleep on the current thread.
using SleepFn = std::function<void(std::chrono::microseconds)>;

/// Dummy struct used for scheduler not handling any particular message.
struct EmptyMessage {};

/// Interface for class able to handle Message.
template <typename Message> class MessageListener {
public:
  virtual void on_message(Message) = 0;
};

/// Dummy implementation of MessageListener for Empty.Message.
/// This is used for scheduler not handling any particular executor.
class DummyListener : public MessageListener<EmptyMessage> {
public:
  void on_message(EmptyMessage) override {}
};

} // namespace task
} // namespace whl
