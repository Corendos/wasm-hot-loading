#pragma once

#include <whl/task/prelude.hpp>
#include <whl/task/sender.hpp>

#include <cstddef>
#include <functional>
#include <memory>

namespace whl {
namespace task {

/// The [Scheduler] class allows to schedule tasks in the future and handle
/// messages without the requirement of having a thread pool behind. This class
/// is an interface, allowing different implementations depending on the
/// platform.
///
/// For messages, there is a blocking message queue waiting for activity.
/// For scheduled tasks, it relies behind the hood on a priority queue and a
/// worker thread waiting for the correct time to execute pending operations.
/// These tasks are then pushed at the correct timing to the message queue for
/// execution.
///
/// On web, this is a passthrough to the browser event loop which is giving us
/// this behaviour for free.

template <typename T, typename Listener> class IScheduler : public Sender<T> {
public:
  virtual ~IScheduler() {}

  /// Schedule a `task` to be executed in a certain `delay` and an optional task
  /// to be run on stop. Returns the [TaskId] of the newly created task.
  TaskId schedule(std::chrono::microseconds delay, TaskFn fn,
                  GroupId groupId = 0,
                  std::optional<StopFn> stopFn = std::nullopt);
  virtual TaskId schedule(std::chrono::microseconds delay, Priority priority,
                          TaskFn fn, GroupId groupId = 0,
                          std::optional<StopFn> stopFn = std::nullopt) = 0;

  /// Schedule a `task` to be executed in a certain `delay` and an optional task
  /// to be run on stop. Once the task is executed, the scheduler willreschedule
  /// it over and over, until the task is cancelled. Returns the [TaskId] of the
  /// newly created task.
  TaskId schedule_repeatedly(std::chrono::microseconds delay, TaskFn fn,
                             GroupId groupId = 0,
                             std::optional<StopFn> stopFn = std::nullopt);
  virtual TaskId
  schedule_repeatedly(std::chrono::microseconds delay, Priority priority,
                      TaskFn fn, GroupId groupId = 0,
                      std::optional<StopFn> stopFn = std::nullopt) = 0;

  /// Schedule a `task` to be executed at an absolute `time` and an optional
  /// task to be run on stop. Why this is the most common usage, the
  /// implementation relies on the absolute time to prioritize tasks. Returns
  /// the [TaskId] of the newly created task.
  TaskId schedule_at(std::chrono::steady_clock::time_point time, TaskFn task,
                     GroupId groupId = 0,
                     std::optional<StopFn> stopFn = std::nullopt);
  virtual TaskId schedule_at(std::chrono::steady_clock::time_point time,
                             Priority priority, TaskFn fn, GroupId groupId = 0,
                             std::optional<StopFn> stopFn = std::nullopt) = 0;

  /// Schedule a `task` to be executed as soon as possible and an optional task
  /// to be run on stop. Returns the [TaskId] of the newly created task.
  TaskId schedule_now(TaskFn task, GroupId groupId = 0,
                      std::optional<StopFn> stopFn = std::nullopt);
  virtual TaskId schedule_now(Priority priority, TaskFn fn, GroupId groupId = 0,
                              std::optional<StopFn> stopFn = std::nullopt) = 0;

  /// Starts handling messages and executing scheduled tasks on the scheduler.
  virtual void start() = 0;

  /// Stop the main [Scheduler] loop by dropping all the tasks of its internal
  /// queue. A successive call of `run()` will start again the scheduler with an
  /// empty queue.
  virtual void stop() = 0;

  /// Run the main [Scheduler] loop. `run()` is blocking on the calling thread
  /// until there are tasks to be executed in the queue.
  virtual void run() = 0;

  /// Cancel a task through its associated [TaskId].
  virtual void cancel(TaskId taskId) = 0;

  /// Cancel all tasks through its associated [GroupId].
  virtual void cancel_group(GroupId groupId) = 0;

  /// Cancel all tasks.
  virtual void cancel_all() = 0;

  /// Set the listener to use for messages.
  virtual void set_listener(Listener *listener) = 0;

  /// Indicates if the scheduler has tasks to be scheduled.
  virtual bool empty() const = 0;
};

/// Constructs a scheduler with the given name.
template <typename T, typename Listener>
std::unique_ptr<IScheduler<T, Listener>> scheduler(const std::string &name);

template <typename T, typename Listener>
TaskId IScheduler<T, Listener>::schedule(std::chrono::microseconds delay,
                                         TaskFn fn, GroupId groupId,
                                         std::optional<StopFn> stopFn) {
  return schedule(delay, std::numeric_limits<Priority>::max(), std::move(fn),
                  groupId, std::move(stopFn));
}

template <typename T, typename Listener>
TaskId
IScheduler<T, Listener>::schedule_repeatedly(std::chrono::microseconds delay,
                                             TaskFn fn, GroupId groupId,
                                             std::optional<StopFn> stopFn) {
  return schedule_repeatedly(delay, std::numeric_limits<Priority>::max(),
                             std::move(fn), groupId, std::move(stopFn));
}

template <typename T, typename Listener>
TaskId
IScheduler<T, Listener>::schedule_at(std::chrono::steady_clock::time_point time,
                                     TaskFn fn, GroupId groupId,
                                     std::optional<StopFn> stopFn) {
  return schedule_at(time, std::numeric_limits<Priority>::max(), std::move(fn),
                     groupId, std::move(stopFn));
}

template <typename T, typename Listener>
TaskId IScheduler<T, Listener>::schedule_now(TaskFn fn, GroupId groupId,
                                             std::optional<StopFn> stopFn) {
  return schedule_now(std::numeric_limits<Priority>::max(), std::move(fn),
                      groupId, std::move(stopFn));
}

using IBasicScheduler = IScheduler<EmptyMessage, DummyListener>;

} // namespace task
} // namespace whl
