#pragma once

#include <array>
#include <atomic>
#include <cassert>
#include <condition_variable>
#include <list>
#include <mutex>
#include <optional>

namespace whl {

/// Generic implementation of a Message Queue. This is the simplest/dumbest
/// implementation possible of a queue. It is a MPMC unbounded queue.
template <typename T> class MessageQueue final {
public:
  /// Constructor / Destructor
  MessageQueue();
  ~MessageQueue();
  MessageQueue(const MessageQueue &other) = delete;
  MessageQueue(MessageQueue &&other) noexcept = delete;
  MessageQueue &operator=(const MessageQueue &other) = delete;
  MessageQueue &operator=(MessageQueue &&other) noexcept = delete;

  /// Pushes a message to the queue as an universal reference.
  template <typename T2> bool push(T2 &&item);

  /// Tries to pop a message from the queue or return std::nullopt if the
  /// queue is empty.
  std::optional<T> pop();

  /// Waits for a message and returns it. Returns std::nullopt if the queue
  /// has been closed.
  std::optional<T> pop_wait();

  /// Closes the queue.
  void close();

private:
  /// Default allocation size for the queue nodes, the largest between 16
  /// objects or 4096 bytes of objects (for very large objects).
  static constexpr size_t NodeSize{std::max<size_t>(4096 / sizeof(T), 16)};

  /// Represents a node in the linked list of currently allocated nodes.
  struct Node {
    std::array<T, NodeSize> slots;
  };

  /// Tries to pop an element without owning the lock.
  std::optional<T> pop_unsafe();

  /// The condition variable used for synchronization.
  std::condition_variable m_cond{};
  /// The mutex protecting the queue.
  std::mutex m_mutex{};
  /// The list of currently allocated nodes.
  std::list<std::unique_ptr<Node>> m_nodes{};
  /// Boolean indicating if the queue has been closed.
  bool m_closed{false};
  /// Index of the current tail (where messages are read from).
  size_t m_tail_index{0};
  /// Index of the current head (where messages are written to).
  size_t m_head_index{0};
  /// The current number of elements in the queue.
  size_t m_count{0};
};

template <typename T> MessageQueue<T>::MessageQueue() {
  m_nodes.emplace_back(new MessageQueue<T>::Node());
}

template <typename T> MessageQueue<T>::~MessageQueue() = default;

template <typename T>
template <typename T2>
bool MessageQueue<T>::push(T2 &&item) {
  bool should_notify{false};
  {
    std::unique_lock<std::mutex> lock{m_mutex};
    if (m_closed)
      return false;

    // NOTE(Corentin): If we observe a count of 0, it means that the consumer is
    //                 probably sleeping waiting for messages, and we should
    //                 notify.
    should_notify = m_count == 0;
    if (m_head_index >= MessageQueue::NodeSize) {
      assert(m_head_index == MessageQueue::NodeSize);
      m_head_index = 0;
      m_nodes.emplace_back(new Node());
    }

    m_nodes.back()->slots[m_head_index] = std::forward<T>(item);
    m_head_index += 1;
    m_count += 1;
  }

  if (should_notify) {
    m_cond.notify_all();
  }

  return true;
}
template <typename T> std::optional<T> MessageQueue<T>::pop() {
  std::unique_lock<std::mutex> lock{m_mutex};

  return pop_unsafe();
}

template <typename T> std::optional<T> MessageQueue<T>::pop_wait() {
  std::unique_lock<std::mutex> lock{m_mutex};

  auto element_opt = pop_unsafe();
  if (element_opt.has_value())
    return element_opt.value();

  m_cond.wait(lock, [this]() { return m_count > 0 || m_closed; });

  if (m_closed && m_count == 0) {
    return std::nullopt;
  }

  // NOTE(Corentin): here we force the unwrapping of the optional because we
  //                 want to be sure were turn something. If the optional is
  //                 empty, then it's a bug and better crash.
  return pop_unsafe().value();
}

template <typename T> void MessageQueue<T>::close() {
  {
    std::unique_lock<std::mutex> lock{m_mutex};
    m_closed = true;
  }
  m_cond.notify_all();
}

template <typename T> std::optional<T> MessageQueue<T>::pop_unsafe() {
  if (m_count == 0)
    return std::nullopt;

  if (m_tail_index >= MessageQueue::NodeSize) {
    assert(m_tail_index == MessageQueue::NodeSize);
    m_tail_index = 0;
    m_nodes.pop_front();
  }

  T local{std::move(m_nodes.front()->slots[m_tail_index])};
  m_tail_index += 1;
  m_count -= 1;

  return local;
}

} // namespace whl
