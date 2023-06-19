#include <whl/task/task_scheduler.hpp>

#include <mutex>
#include <thread>

using namespace std::chrono_literals;

namespace whl {
namespace task {

TaskScheduler::TaskScheduler(std::string name, DequeueFn dequeue_fn)
    : m_name{std::move(name)}, m_dequeue_fn{std::move(dequeue_fn)} {
  m_sleep_fn = [this](std::chrono::microseconds delay) {
    std::unique_lock<std::mutex> lock{m_mutex};
    m_cv.wait_for(lock, delay);
  };
  m_time_fn = []() { return std::chrono::steady_clock::now(); };
}

TaskScheduler::TaskScheduler(std::string name, DequeueFn dequeue_fn,
                             TimeFn timeFn, SleepFn sleep_fn)
    : m_name{std::move(name)}, m_dequeue_fn{std::move(dequeue_fn)},
      m_time_fn{std::move(timeFn)}, m_sleep_fn{std::move(sleep_fn)} {}

TaskId TaskScheduler::schedule(std::chrono::microseconds delay,
                               Priority priority, TaskFn fn, GroupId group_id,
                               std::optional<StopFn> stop_fn) {
  return schedule_at(m_time_fn() + delay, priority, std::move(fn), group_id,
                     std::move(stop_fn));
}

TaskId TaskScheduler::schedule_repeatedly(std::chrono::microseconds delay,
                                          Priority priority, TaskFn fn,
                                          GroupId group_id,
                                          std::optional<StopFn> stop_fn) {
  return schedule_task(Task{delay, m_time_fn() + delay, priority, std::move(fn),
                            group_id, std::move(stop_fn), true});
}

TaskId TaskScheduler::schedule_at(std::chrono::steady_clock::time_point time,
                                  Priority priority, TaskFn fn,
                                  GroupId group_id,
                                  std::optional<StopFn> stop_fn) {
  return schedule_task(Task{m_time_fn(), time, priority, std::move(fn),
                            group_id, std::move(stop_fn)});
}

TaskId TaskScheduler::schedule_now(Priority priority, TaskFn fn,
                                   GroupId group_id,
                                   std::optional<StopFn> stop_fn) {
  auto now = m_time_fn();
  return schedule_task(
      Task{now, now, priority, std::move(fn), group_id, std::move(stop_fn)});
}

void TaskScheduler::run(
    std::optional<std::reference_wrapper<std::promise<void>>> barrier) {
  {
    std::unique_lock<std::mutex> lock{m_mutex};
    m_started = true;
  }

  if (barrier.has_value()) {
    barrier->get().set_value();
  }

  while (true) {
    SchedulerState state{SchedulerState::STANDBY};
    std::chrono::steady_clock::time_point now;
    std::chrono::steady_clock::time_point time;
    TaskFn task_fn;

    {
      const std::unique_lock<std::mutex> lock(m_mutex);
      if (!m_started)
        break;
      if (m_queue.empty()) {
        state = SchedulerState::STANDBY;
      } else {
        // Get the top task and check if we need to wait before execution
        const Task &t = m_queue.top();
        now = std::chrono::steady_clock::now();
        time = t.time;
        state = time > now ? SchedulerState::SLEEP : SchedulerState::EXECUTE;

        if (state == SchedulerState::EXECUTE) {
          // If we don't need to wait, move the top task before popping it
          Task popped_task = m_queue.top_and_pop();
          if (popped_task.repeats) {
            // Make a copy of the function to execute
            task_fn = popped_task.fn;

            // Update the fire time and push it in the queue again
            popped_task.time = popped_task.time + popped_task.delay;
            m_queue.push(std::move(popped_task));
          } else {
            // Otherwise, take ownership of the task function
            task_fn = std::move(popped_task.fn);
          }
        }
      }
    }

    if (state == SchedulerState::SLEEP) {
      m_sleep_fn(
          std::chrono::duration_cast<std::chrono::microseconds>(time - now));
    } else if (state == SchedulerState::EXECUTE) {
      // TODO(Corentin): send to message queue
      if (task_fn) {
        m_dequeue_fn(std::move(task_fn));
      }

      /**
       * This allows other threads to run after the operation.
       * https://stackoverflow.com/a/3727460/3371882
       */
      std::this_thread::sleep_for(0s);
    } else {
      std::unique_lock<std::mutex> lock(m_mutex);
      if (!m_started)
        break;
      m_cv.wait(lock);
    }
  }
}

void TaskScheduler::stop() {
  TaskQueue remaining_tasks;
  {
    const std::unique_lock<std::mutex> lock(m_mutex);
    std::swap(m_queue, remaining_tasks);
    m_started = false;
  }

  while (!remaining_tasks.empty()) {
    Task task = remaining_tasks.top_and_pop();
    if (task.on_stop_fn.has_value() && task.on_stop_fn.value()) {
      m_dequeue_fn(
          TaskFn{[fn = std::move(task.on_stop_fn.value())]() { fn(false); }});
    }
  }

  m_cv.notify_one();
}

void TaskScheduler::cancel(TaskId taskId) {
  std::optional<Task> cancelled_task;
  {
    const std::unique_lock<std::mutex> lock(m_mutex);
    cancelled_task = m_queue.remove(taskId);
  }

  if (cancelled_task.has_value() && cancelled_task->on_stop_fn.has_value() &&
      cancelled_task->on_stop_fn.value()) {
    m_dequeue_fn(TaskFn{
        [fn = std::move(cancelled_task->on_stop_fn.value())]() { fn(true); }});
  }
}
void TaskScheduler::cancel_group(GroupId group_id) {
  std::vector<Task> cancelled_tasks;
  cancelled_tasks.reserve(16);
  {
    const std::unique_lock<std::mutex> lock(m_mutex);
    m_queue.remove_group(group_id, cancelled_tasks);
  }

  for (auto &task : cancelled_tasks) {
    if (task.on_stop_fn.has_value() && task.on_stop_fn.value()) {
      m_dequeue_fn(
          TaskFn{[fn = std::move(task.on_stop_fn.value())]() { fn(true); }});
    }
  }
}
void TaskScheduler::cancel_all() {
  TaskQueue remaining_tasks;
  {
    const std::unique_lock<std::mutex> lock(m_mutex);
    std::swap(m_queue, remaining_tasks);
  }

  while (!remaining_tasks.empty()) {
    Task task = remaining_tasks.top_and_pop();
    if (task.on_stop_fn.has_value() && task.on_stop_fn.value()) {
      m_dequeue_fn(
          TaskFn{[fn = std::move(task.on_stop_fn.value())]() { fn(true); }});
    }
  }
}

bool TaskScheduler::empty() const {
  const std::unique_lock<std::mutex> lock(m_mutex);
  return m_queue.empty();
}

TaskId TaskScheduler::schedule_task(Task task) {
  const auto task_id = task.id;

  {
    const std::unique_lock<std::mutex> lock(m_mutex);
    m_queue.push(std::move(task));
  }

  m_cv.notify_one();
  return task_id;
}

} // namespace task
} // namespace whl
