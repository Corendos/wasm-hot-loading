#pragma once

#include <whl/task/prelude.hpp>

#include <atomic>
#include <utility>

namespace whl {
namespace task {

/// Container for task to be executed.
struct Task {
  /// Id of the task.
  TaskId id;

  /// Group Id of the task.
  GroupId group_id;

  /// Scheduled `time` of the event to be executed.
  std::chrono::steady_clock::time_point time;

  /// Priority of the event, events scheduled for the same time will be executed
  /// in the order of `priority`. Priority is expressed in a human-readable way.
  /// A lower numerical `priority` implies a higher order in the execution
  /// chain. A `priority` of 1 is de-facto considered superior than a priority
  /// of `10`.
  Priority priority;

  /// The `action` to be executed.
  TaskFn fn;

  /// The `action` to be executed when the scheduler is stopped.
  std::optional<StopFn> on_stop_fn{std::nullopt};

  /// The delay between the scheduling and the executing of `fn`.
  std::chrono::microseconds delay;

  /// Is the task supposed to be rescheduled once executed.
  bool repeats;

  Task(std::chrono::microseconds delay,
       std::chrono::steady_clock::time_point time, Priority priority, TaskFn fn,
       GroupId group_id = 0, std::optional<StopFn> on_stop_fn = std::nullopt,
       bool repeats = false)
      : group_id{group_id}, time{time}, priority{priority}, fn{std::move(fn)},
        on_stop_fn{std::move(on_stop_fn)}, delay{delay}, repeats{repeats} {
    static std::atomic<TaskId> TASK_ID{0};
    id = ++TASK_ID;
  }

  Task(std::chrono::steady_clock::time_point now,
       std::chrono::steady_clock::time_point time, Priority priority, TaskFn fn,
       GroupId group_id = 0, std::optional<StopFn> on_stop_fn = std::nullopt,
       bool repeats = false)
      : Task{std::chrono::duration_cast<std::chrono::milliseconds>(time - now),
             time,
             priority,
             std::move(fn),
             group_id,
             std::move(on_stop_fn),
             repeats} {}

  inline bool operator==(const Task &e) const {
    return time == e.time && priority == e.priority;
  }

  inline bool operator<(const Task &e) const {
    return time < e.time || (time == e.time && priority < e.priority);
  }

  inline bool operator<=(const Task &e) const {
    return *this == e || *this < e;
  }

  inline bool operator>(const Task &e) const { return !(*this <= e); }

  inline bool operator>=(const Task &e) const {
    return *this == e || *this > e;
  }
};

} // namespace task
} // namespace whl
