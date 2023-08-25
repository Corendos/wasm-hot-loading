// Copyright 2023 - Corentin Godeau
// SPDX-License-Identifier: MIT

#pragma once

#include <whl/task/task.hpp>

#include <algorithm>
#include <functional>
#include <queue>

namespace whl {
namespace task {

/// Priority queue for tasks ordering.
/// The priority queue is intentionally reversed for prioritizing tasks with
/// lower `TimePoint` and lower priority.
struct TaskQueue
    : std::priority_queue<Task, std::vector<Task>, std::greater<>> {
  /// Clear all the queue.
  void clear() {
    decltype(c) empty;
    std::swap(c, empty);
  }

  /// eturns the top object by moving it from the queue.
  Task top_and_pop() {
    std::pop_heap(c.begin(), c.end(), comp);
    Task t = std::move(c.back());
    c.pop_back();
    return t;
  }

  /// Remove one task given its `id` in the queue. If the task does not exist,
  /// returns std::nullopt, otherwise returns the cancelled task.
  std::optional<Task> remove(TaskId id) {
    auto new_end = std::partition(c.begin(), c.end(),
                                  [id](const Task &t) { return t.id != id; });
    // If there was nothing to remove, return early
    if (new_end == c.end())
      return std::nullopt;

    Task t = std::move(*new_end);
    c.erase(new_end, c.end());
    std::make_heap(c.begin(), c.end(), TaskQueue::comp);
    return t;
  }

  /// Remove all tasks given their group `id` and push them to the given vector.
  /// Returns the number of task cancelled.
  size_t remove_group(GroupId id, std::vector<Task> &cancelledTasks) {
    auto new_end = std::partition(
        c.begin(), c.end(), [id](const Task &t) { return t.group_id != id; });
    // If there was nothing to remove, return early
    if (new_end == c.end())
      return 0;

    size_t count = std::distance(new_end, c.end());
    if (cancelledTasks.size() + count > cancelledTasks.capacity()) {
      cancelledTasks.reserve(cancelledTasks.size() + count);
    }

    std::move(new_end, c.end(), std::back_inserter(cancelledTasks));
    c.erase(new_end, c.end());
    std::make_heap(c.begin(), c.end(), TaskQueue::comp);
    return count;
  }
};

} // namespace task
} // namespace whl
