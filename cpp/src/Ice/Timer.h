// Copyright (c) ZeroC, Inc.

#ifndef ICE_TIMER_H
#define ICE_TIMER_H

#include "Ice/Config.h"
#include "Ice/TimerTask.h"

#include <cassert>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <map>
#include <mutex>
#include <optional>
#include <set>
#include <stdexcept>
#include <thread>

namespace IceInternal
{
#ifdef __clang__
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wweak-vtables"
#endif

    // Adapts a function<void()> to a TimerTask.
    class InlineTimerTask final : public TimerTask
    {
    public:
        InlineTimerTask(std::function<void()> function) : _function(std::move(function)) {}

        void runTimerTask() final { _function(); }

    private:
        std::function<void()> _function;
    };

#ifdef __clang__
#    pragma clang diagnostic pop
#endif

    // The timer class is used to schedule tasks for one-time execution or repeated execution. Tasks are executed by a
    // dedicated timer thread sequentially.
    class ICE_API Timer
    {
    public:
        Timer();
        virtual ~Timer() = default;

        // Destroy the timer and join the timer execution thread. Must not be called from a timer task.
        void destroy();

        // Schedule a function-task for execution after a given delay.
        template<class Rep, class Period>
        void schedule(std::function<void()> function, const std::chrono::duration<Rep, Period>& delay)
        {
            schedule(std::make_shared<InlineTimerTask>(std::move(function)), delay);
        }

        // Schedule task for execution after a given delay.
        template<class Rep, class Period>
        void schedule(const TimerTaskPtr& task, const std::chrono::duration<Rep, Period>& delay)
        {
            std::lock_guard lock(_mutex);
            if (_destroyed)
            {
                throw std::invalid_argument("timer destroyed");
            }

            if (delay < std::chrono::nanoseconds::zero())
            {
                throw std::invalid_argument("invalid negative delay");
            }

            auto now = std::chrono::steady_clock::now();
            auto time = now + delay;
            if (delay > std::chrono::nanoseconds::zero() && time < now)
            {
                throw std::invalid_argument("delay too large, resulting in overflow");
            }

            bool inserted = _tasks.insert(make_pair(task, time)).second;
            if (!inserted)
            {
                throw std::invalid_argument("task is already scheduled");
            }
            _tokens.insert({time, std::nullopt, task});

            if (_wakeUpTime == std::chrono::steady_clock::time_point() || time < _wakeUpTime)
            {
                _condition.notify_one();
            }
        }

        // Reschedule a task for execution after a given delay. This function also succeeds if the task was not
        // previously scheduled.
        template<class Rep, class Period>
        void reschedule(const TimerTaskPtr& task, const std::chrono::duration<Rep, Period>& delay)
        {
            std::lock_guard lock(_mutex);
            if (_destroyed)
            {
                throw std::invalid_argument("timer destroyed");
            }

            if (delay < std::chrono::nanoseconds::zero())
            {
                throw std::invalid_argument("invalid negative delay");
            }

            auto now = std::chrono::steady_clock::now();
            auto time = now + delay;
            if (delay > std::chrono::nanoseconds::zero() && time < now)
            {
                throw std::invalid_argument("delay too large, resulting in overflow");
            }

            cancelNoSync(task);

            _tasks.insert(make_pair(task, time));
            _tokens.insert({time, std::nullopt, task});

            if (_wakeUpTime == std::chrono::steady_clock::time_point() || time < _wakeUpTime)
            {
                _condition.notify_one();
            }
        }

        // Schedule a task for repeated execution with the given delay between each execution.
        template<class Rep, class Period>
        void scheduleRepeated(const TimerTaskPtr& task, const std::chrono::duration<Rep, Period>& delay)
        {
            std::lock_guard lock(_mutex);
            if (_destroyed)
            {
                throw std::invalid_argument("timer destroyed");
            }

            if (delay < std::chrono::nanoseconds::zero())
            {
                throw std::invalid_argument("invalid negative delay");
            }

            auto now = std::chrono::steady_clock::now();
            auto time = now + delay;
            if (delay > std::chrono::nanoseconds::zero() && time < now)
            {
                throw std::invalid_argument("delay too large, resulting in overflow");
            }

            bool inserted = _tasks.insert(make_pair(task, time)).second;
            if (!inserted)
            {
                throw std::invalid_argument("task is already scheduled");
            }
            _tokens.insert({time, std::chrono::duration_cast<std::chrono::nanoseconds>(delay), task});

            if (_wakeUpTime == std::chrono::steady_clock::time_point() || time < _wakeUpTime)
            {
                _condition.notify_one();
            }
        }

        //
        // Cancel a task. Returns true if the task has not yet run or if
        // it's a task scheduled for repeated execution. Returns false if
        // the task has already run, was already cancelled or was never
        // scheduled.
        //
        bool cancel(const TimerTaskPtr&) noexcept;

        // Checks if this timer task is scheduled.
        bool isScheduled(const TimerTaskPtr&);

    protected:
        virtual void runTimerTask(const TimerTaskPtr&);

    private:
        struct Token
        {
            std::chrono::steady_clock::time_point scheduledTime;
            std::optional<std::chrono::nanoseconds> delay;
            TimerTaskPtr task;
            bool operator<(const Token& other) const
            {
                if (scheduledTime < other.scheduledTime)
                {
                    return true;
                }
                else if (scheduledTime > other.scheduledTime)
                {
                    return false;
                }
                return task < other.task;
            }
        };

        void run();
        bool cancelNoSync(const TimerTaskPtr& task) noexcept;

        std::mutex _mutex;
        std::condition_variable _condition;
        std::set<Token> _tokens;
        std::map<TimerTaskPtr, std::chrono::steady_clock::time_point> _tasks;
        bool _destroyed{false};
        std::chrono::steady_clock::time_point _wakeUpTime;
        std::thread _worker;
    };
}

#endif
