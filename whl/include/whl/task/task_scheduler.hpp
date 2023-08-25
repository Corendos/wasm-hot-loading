// Copyright 2023 - Corentin Godeau
// SPDX-License-Identifier: MIT

#pragma once

#include <whl/task/prelude.hpp>
#include <whl/task/task_queue.hpp>

#include <condition_variable>
#include <future>

namespace whl {
namespace task {

/// Signature of the function to run when a task is dequeued.
using DequeueFn = std::function<void(TaskFn)>;

/// Represents the current state of a Task Scheduler
enum class SchedulerState { STANDBY, EXECUTE, SLEEP };

/// A class responsible for executing a given dequeue function at exact
/// intervals. This class is configurable to adapt to various use-case. TimeFn
/// is a function that should return what is admitted to be the current time.
/// SleepFn is a function that should wait for the duration given in parameter.
///  For example, by injecting a time function returning the current value of a
/// fake clock and a sleep function advancing this same clock, we can simulate
/// very precise timing in unit tests.
class TaskScheduler {
public:
  /// Constructors
  TaskScheduler(std::string name, DequeueFn dequeue_fn);
  TaskScheduler(std::string name, DequeueFn dequeue_fn, TimeFn timeFn,
                SleepFn sleep_fn);

  /// Schedule a `task` to be executed in a certain `delay` and an optional task
  /// to be run on stop. Returns the [TaskId] of the newly created task.
  TaskId schedule(std::chrono::microseconds delay, Priority priority, TaskFn fn,
                  GroupId group_id, std::optional<StopFn> stop_fn);

  /// Schedule a `task` to be executed in a certain `delay` and an optional task
  /// to be run on stop. Once the task is executed, the scheduler will
  /// reschedule it over and over, until the task is cancelled. Returns the
  /// [TaskId] of the newly created task.
  TaskId schedule_repeatedly(std::chrono::microseconds delay, Priority priority,
                             TaskFn fn, GroupId group_id,
                             std::optional<StopFn> stop_fn);

  /// Schedule a `task` to be executed at an absolute `time` and an optional
  /// task to be run on stop. Why this is the most common usage, the
  /// implementation relies on the absolute time to prioritize tasks. Returns
  /// the [TaskId] of the newly created task.
  TaskId schedule_at(std::chrono::steady_clock::time_point time,
                     Priority priority, TaskFn fn, GroupId group_id,
                     std::optional<StopFn> stop_fn);

  /// Schedule a `task` to be executed as soon as possible and an optional task
  /// to be run on stop. Returns the [TaskId] of the newly created task.
  TaskId schedule_now(Priority priority, TaskFn fn, GroupId group_id,
                      std::optional<StopFn> stop_fn);

  /// Stops the scheduler loop and dequeue any remaining tasks present in the
  /// queue.
  void stop();

  /// Runs the scheduler.
  /// `barrier` can be used to wait for the scheduler to be started.
  void run(std::optional<std::reference_wrapper<std::promise<void>>> barrier);

  /// Cancel a task through its associated [TaskId].
  void cancel(TaskId taskId);

  /// Cancel all tasks through its associated [GroupId].
  void cancel_group(GroupId group_id);

  /// Cancel all tasks.
  void cancel_all();

  /// Indicates if the scheduler has tasks to be scheduled.
  bool empty() const;

private:
  /// The name of this task scheduler.
  std::string m_name;
  /// The function to execute on dequeue.
  DequeueFn m_dequeue_fn;
  /// The function to execute to get the current timepoint.
  TimeFn m_time_fn;
  /// The function to execute to pause the execution for a given amount of time.
  SleepFn m_sleep_fn;

  /// Is the scheduler started.
  bool m_started{true};

  /// Mutex to lock `mQueue` operations between multiple calling threads.
  mutable std::mutex m_mutex;
  /// Conditional variable to wake the loop on new events.
  std::condition_variable m_cv;

  /// Queue of tasks.
  TaskQueue m_queue;

  /// Pushes a task to the internal queue in a thread-safe manner.
  TaskId schedule_task(Task task);
};

} // namespace task
} // namespace whl
