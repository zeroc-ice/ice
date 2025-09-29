// Copyright (c) ZeroC, Inc.

#include "Timer.h"
#include "ConsoleUtil.h"
#include "Ice/Exception.h"

using namespace std;
using namespace Ice;
using namespace IceInternal;

TimerTask::~TimerTask() = default; // Out of line to avoid weak vtable

Timer::Timer() : _wakeUpTime(chrono::steady_clock::time_point()), _worker(&Timer::run, this) {}

void
Timer::destroy()
{
    if (std::this_thread::get_id() == _worker.get_id())
    {
        throw std::runtime_error("a timer task cannot destroy the timer");
    }
    {
        std::lock_guard lock(_mutex);
        if (_destroyed)
        {
            return;
        }
        _destroyed = true;
        _tasks.clear();
        _tokens.clear();
        _condition.notify_one();
    }
    _worker.join();
}

bool
Timer::cancel(const TimerTaskPtr& task) noexcept
{
    lock_guard lock(_mutex);
    return cancelNoSync(task);
}

bool
Timer::isScheduled(const TimerTaskPtr& task)
{
    lock_guard lock(_mutex);
    if (_destroyed)
    {
        return false;
    }
    return _tasks.find(task) != _tasks.end();
}

void
Timer::run()
{
    Token token{chrono::steady_clock::time_point(), nullopt, nullptr};
    while (true)
    {
        {
            unique_lock lock(_mutex);

            if (!_destroyed)
            {
                // If the task we just ran is a repeated task, schedule it again for execution if it wasn't canceled.
                if (token.delay)
                {
                    auto p = _tasks.find(token.task);
                    if (p != _tasks.end())
                    {
                        token.scheduledTime = chrono::steady_clock::now() + token.delay.value();
                        p->second = token.scheduledTime;
                        _tokens.insert(token);
                    }
                }
                token = {chrono::steady_clock::time_point(), nullopt, nullptr};

                if (_tokens.empty())
                {
                    _wakeUpTime = chrono::steady_clock::time_point();
                    _condition.wait(lock);
                }
            }

            if (_destroyed)
            {
                break;
            }

            while (!_tokens.empty() && !_destroyed)
            {
                const auto now = chrono::steady_clock::now();
                const Token& first = *(_tokens.begin());
                if (first.scheduledTime <= now)
                {
                    token = first;
                    _tokens.erase(_tokens.begin());
                    if (!token.delay)
                    {
                        _tasks.erase(token.task);
                    }
                    break;
                }

                _wakeUpTime = first.scheduledTime;
                _condition.wait_for(lock, first.scheduledTime - now);
            }

            if (_destroyed)
            {
                break;
            }
        }

        if (token.task)
        {
            try
            {
                runTimerTask(token.task);
            }
            catch (const Ice::Exception& e)
            {
                consoleErr << "Ice::Timer::run(): uncaught exception:\n" << e << endl;
            }
            catch (const std::exception& e)
            {
                consoleErr << "Ice::Timer::run(): uncaught exception:\n" << e.what() << endl;
            }
            catch (...)
            {
                consoleErr << "Ice::Timer::run(): uncaught exception" << endl;
            }

            if (!token.delay)
            {
                // If the task is not a repeated task, clear the task reference now rather than
                // in the synchronization block above. Clearing the task reference might end up
                // calling user code which could trigger a deadlock. See also issue #352.
                token.task = nullptr;
            }
        }
    }
}

bool
Timer::cancelNoSync(const TimerTaskPtr& task) noexcept
{
    if (_destroyed)
    {
        return false;
    }

    auto p = _tasks.find(task);
    if (p == _tasks.end())
    {
        return false;
    }

    _tokens.erase(Token{p->second, nullopt, p->first});
    _tasks.erase(p);

    return true;
}

void
Timer::runTimerTask(const TimerTaskPtr& task)
{
    task->runTimerTask();
}
