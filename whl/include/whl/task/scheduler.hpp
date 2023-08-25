// Copyright 2023 - Corentin Godeau
// SPDX-License-Identifier: MIT

#pragma once

#include <whl/sync/message_queue.hpp>
#include <whl/task/prelude.hpp>
#include <whl/task/scheduler_interface.hpp>
#include <whl/task/task_queue.hpp>
#include <whl/task/task_scheduler.hpp>

#include <condition_variable>
#include <future>
#include <mutex>
#include <type_traits>
#include <variant>

namespace whl {
namespace task {

template <typename T, typename Listener>
class Scheduler final : public IScheduler<T, Listener> {
public:
  Scheduler(const std::string &name, Listener *listener = nullptr);
  ~Scheduler() override;

  TaskId schedule(std::chrono::microseconds delay, Priority priority, TaskFn fn,
                  GroupId groupId = 0,
                  std::optional<StopFn> stopFn = std::nullopt) override;

  TaskId schedule_repeatedly(std::chrono::microseconds delay, Priority priority,
                             TaskFn fn, GroupId groupId,
                             std::optional<StopFn> stopFn) override;

  TaskId schedule_at(std::chrono::steady_clock::time_point time,
                     Priority priority, TaskFn fn, GroupId groupId,
                     std::optional<StopFn> stopFn) override;

  TaskId schedule_now(Priority priority, TaskFn fn, GroupId groupId = 0,
                      std::optional<StopFn> stopFn = std::nullopt) override;

  void send(T &&message) override;
  void send(const T &message) override;

  void start() override;
  void run() override;
  void stop() override;
  void cancel(TaskId taskId) override;
  void cancel_group(GroupId groupId) override;
  void cancel_all() override;

  void set_listener(Listener *listener) override;

  bool empty() const override;

private:
  using MessageType = std::variant<T, TaskFn>;
  struct Message {
    MessageType value;
  };

  bool m_started{false};
  std::string m_name;

  TaskScheduler m_task_scheduler;
  std::thread m_task_scheduler_worker_thread;

  MessageQueue<Message> m_message_queue;
  Listener *m_listener;
};

template <typename T, typename Listener>
Scheduler<T, Listener>::Scheduler(const std::string &name, Listener *listener)
    : m_name(name), m_task_scheduler{name,
                                     [this](TaskFn task) {
                                       m_message_queue.push(
                                           Message{std::move(task)});
                                     }},
      m_listener(listener) {}

template <typename T, typename Listener> Scheduler<T, Listener>::~Scheduler() {
  if (m_task_scheduler_worker_thread.joinable()) {
    m_task_scheduler_worker_thread.join();
  }
}

template <typename T, typename Listener>
TaskId Scheduler<T, Listener>::schedule(std::chrono::microseconds delay,
                                        Priority priority, TaskFn fn,
                                        GroupId groupId,
                                        std::optional<StopFn> stopFn) {
  return m_task_scheduler.schedule(delay, priority, std::move(fn), groupId,
                                   std::move(stopFn));
}

template <typename T, typename Listener>
TaskId Scheduler<T, Listener>::schedule_repeatedly(
    std::chrono::microseconds delay, Priority priority, TaskFn fn,
    GroupId groupId, std::optional<StopFn> stopFn) {
  return m_task_scheduler.schedule_repeatedly(delay, priority, std::move(fn),
                                              groupId, std::move(stopFn));
}

template <typename T, typename Listener>
TaskId Scheduler<T, Listener>::schedule_at(
    std::chrono::steady_clock::time_point time, Priority priority, TaskFn fn,
    GroupId groupId, std::optional<StopFn> stopFn) {
  return m_task_scheduler.schedule_at(time, priority, std::move(fn), groupId,
                                      std::move(stopFn));
}

template <typename T, typename Listener>
TaskId Scheduler<T, Listener>::schedule_now(Priority priority, TaskFn fn,
                                            GroupId groupId,
                                            std::optional<StopFn> stopFn) {
  return m_task_scheduler.schedule_now(priority, std::move(fn), groupId,
                                       std::move(stopFn));
}

template <typename T, typename Listener>
void Scheduler<T, Listener>::send(T &&message) {
  m_message_queue.push(Message{std::move(message)});
}

template <typename T, typename Listener>
void Scheduler<T, Listener>::send(const T &message) {
  m_message_queue.push(Message{message});
}

template <typename T, typename Listener> void Scheduler<T, Listener>::start() {
  std::promise<void> start_barrier{};
  m_task_scheduler_worker_thread = std::thread(
      [this, &start_barrier]() { m_task_scheduler.run(start_barrier); });
  start_barrier.get_future().get();
}

template <typename T, typename Listener> void Scheduler<T, Listener>::run() {
  while (true) {
    std::optional<Message> queued_message = m_message_queue.pop_wait();

    if (!queued_message.has_value()) {
      return;
    }

    Message &message = queued_message.value();

    switch (message.value.index()) {
    case 0: {
      if (m_listener) {
        auto &value = std::get<T>(message.value);
        m_listener->on_message(std::move(value));
      }
    } break;
    case 1: {
      std::get<TaskFn>(message.value)();
    } break;
    default:
      return;
    }
  }
}

template <typename T, typename Listener> void Scheduler<T, Listener>::stop() {
  m_task_scheduler.stop();

  m_message_queue.close();
}

template <typename T, typename Listener>
void Scheduler<T, Listener>::cancel(TaskId taskId) {
  m_task_scheduler.cancel(taskId);
}

template <typename T, typename Listener>
void Scheduler<T, Listener>::cancel_group(GroupId groupId) {
  m_task_scheduler.cancel_group(groupId);
}

template <typename T, typename Listener>
void Scheduler<T, Listener>::cancel_all() {
  m_task_scheduler.cancel_all();
}

template <typename T, typename Listener>
void Scheduler<T, Listener>::set_listener(Listener *listener) {
  m_listener = listener;
}

template <typename T, typename Listener>
bool Scheduler<T, Listener>::empty() const {
  return m_task_scheduler.empty();
}

inline std::unique_ptr<IScheduler<EmptyMessage, DummyListener>>
basicScheduler(const std::string &name) {
  return std::make_unique<Scheduler<EmptyMessage, DummyListener>>(name);
}

template <typename Message, typename Listener>
inline std::unique_ptr<IScheduler<Message, Listener>>
scheduler(const std::string &name) {
  return std::make_unique<Scheduler<Message, Listener>>(name);
}

} // namespace task
} // namespace whl
