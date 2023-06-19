#pragma once

namespace whl {
namespace task {

/// Interface used to get a handle to a Scheduler to which we only want to send
/// messages.
template <typename T> class Sender {
public:
  virtual ~Sender() = default;

  /// Sends a message to this Scheduler for execution.
  virtual void send(T &&message) = 0;
  virtual void send(const T &message) = 0;
};

} // namespace task
} // namespace whl
