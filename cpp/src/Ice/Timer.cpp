//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/Timer.h>
#include <IceUtil/Exception.h>
#include <Ice/ConsoleUtil.h>

using namespace std;
using namespace IceUtil;
using namespace IceInternal;

TimerTask::~TimerTask()
{
    // Out of line to avoid weak vtable
}

TimerPtr
Timer::create()
{
    auto timer = shared_ptr<Timer>(new Timer());
    timer->start();
    return timer;
}

TimerPtr
Timer::create(int priority)
{
    auto timer = shared_ptr<Timer>(new Timer());
    timer->start(0, priority);
    return timer;
}

Timer::Timer() :
    Thread("IceUtil timer thread"),
    _destroyed(false)
{
}

void
Timer::destroy()
{
    {
        lock_guard lock(_mutex);
        if(_destroyed)
        {
            return;
        }
        _destroyed = true;
        _conditionVariable.notify_one();
        _tasks.clear();
        _tokens.clear();
    }

    if(getThreadControl() == ThreadControl())
    {
        getThreadControl().detach();
    }
    else
    {
        getThreadControl().join();
    }
}

void
Timer::schedule(const TimerTaskPtr& task, const IceUtil::Time& delay)
{
    lock_guard lock(_mutex);
    if(_destroyed)
    {
        throw IllegalArgumentException(__FILE__, __LINE__, "timer destroyed");
    }

    IceUtil::Time now = IceUtil::Time::now(IceUtil::Time::Monotonic);
    IceUtil::Time time = now + delay;
    if(delay > IceUtil::Time() && time < now)
    {
        throw IllegalArgumentException(__FILE__, __LINE__, "invalid delay");
    }

    bool inserted = _tasks.insert(make_pair(task, time)).second;
    if(!inserted)
    {
        throw IllegalArgumentException(__FILE__, __LINE__, "task is already scheduled");
    }
    _tokens.insert(Token(time, IceUtil::Time(), task));

    if(_wakeUpTime == IceUtil::Time() || time < _wakeUpTime)
    {
        _conditionVariable.notify_one();
    }
}

void
Timer::scheduleRepeated(const TimerTaskPtr& task, const IceUtil::Time& delay)
{
    lock_guard lock(_mutex);
    if(_destroyed)
    {
        throw IllegalArgumentException(__FILE__, __LINE__, "timer destroyed");
    }

    IceUtil::Time now = IceUtil::Time::now(IceUtil::Time::Monotonic);
    const Token token(now + delay, delay, task);
    if(delay > IceUtil::Time() && token.scheduledTime < now)
    {
        throw IllegalArgumentException(__FILE__, __LINE__, "invalid delay");
    }

    bool inserted = _tasks.insert(make_pair(task, token.scheduledTime)).second;
    if(!inserted)
    {
        throw IllegalArgumentException(__FILE__, __LINE__, "task is already scheduled");
    }
    _tokens.insert(token);

    if(_wakeUpTime == IceUtil::Time() || token.scheduledTime < _wakeUpTime)
    {
        _conditionVariable.notify_one();
    }
}

bool
Timer::cancel(const TimerTaskPtr& task)
{
    lock_guard lock(_mutex);
    if(_destroyed)
    {
        return false;
    }

    map<TimerTaskPtr, IceUtil::Time, TimerTaskCompare>::iterator p = _tasks.find(task);
    if(p == _tasks.end())
    {
        return false;
    }

    _tokens.erase(Token(p->second, IceUtil::Time(), p->first));
    _tasks.erase(p);

    return true;
}

void
Timer::run()
{
    Token token(IceUtil::Time(), IceUtil::Time(), 0);
    while(true)
    {
        {
            unique_lock lock(_mutex);

            if(!_destroyed)
            {
                //
                // If the task we just ran is a repeated task, schedule it
                // again for execution if it wasn't canceled.
                //
                if(token.delay != IceUtil::Time())
                {
                    map<TimerTaskPtr, IceUtil::Time, TimerTaskCompare>::iterator p = _tasks.find(token.task);
                    if(p != _tasks.end())
                    {
                        token.scheduledTime = IceUtil::Time::now(IceUtil::Time::Monotonic) + token.delay;
                        p->second = token.scheduledTime;
                        _tokens.insert(token);
                    }
                }
                token = Token(IceUtil::Time(), IceUtil::Time(), 0);

                if(_tokens.empty())
                {
                    _wakeUpTime = IceUtil::Time();
                    _conditionVariable.wait(lock);
                }
            }

            if(_destroyed)
            {
                break;
            }

            while(!_tokens.empty() && !_destroyed)
            {
                const IceUtil::Time now = IceUtil::Time::now(IceUtil::Time::Monotonic);
                const Token& first = *(_tokens.begin());
                if(first.scheduledTime <= now)
                {
                    token = first;
                    _tokens.erase(_tokens.begin());
                    if(token.delay == IceUtil::Time())
                    {
                        _tasks.erase(token.task);
                    }
                    break;
                }

                _wakeUpTime = first.scheduledTime;
                _conditionVariable.wait_for(lock, chrono::microseconds((first.scheduledTime - now).toMicroSeconds()));
            }

            if(_destroyed)
            {
                break;
            }
        }

        if(token.task)
        {
            try
            {
                runTimerTask(token.task);
            }
            catch(const IceUtil::Exception& e)
            {
                consoleErr << "IceUtil::Timer::run(): uncaught exception:\n" << e.what();
#ifdef __GNUC__
                consoleErr << "\n" << e.ice_stackTrace();
#endif
                consoleErr << endl;
            }
            catch(const std::exception& e)
            {
                consoleErr << "IceUtil::Timer::run(): uncaught exception:\n" << e.what() << endl;
            }
            catch(...)
            {
                consoleErr << "IceUtil::Timer::run(): uncaught exception" << endl;
            }

            if(token.delay == IceUtil::Time())
            {
                //
                // If the task is not a repeated task, clear the task reference now rather than
                // in the synchronization block above. Clearing the task reference might end up
                // calling user code which could trigger a deadlock. See also issue #352.
                //
                token.task = nullptr;
            }
        }
    }
}

void
Timer::runTimerTask(const TimerTaskPtr& task)
{
    task->runTimerTask();
}
