// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Timer.h>
#include <IceUtil/Exception.h>

using namespace std;
using namespace IceUtil;

Timer::Timer() :
    Thread("IceUtil timer thread"),
    _destroyed(false)
{
    __setNoDelete(true);
    start();
    __setNoDelete(false);
}

Timer::Timer(int priority) :
    Thread("IceUtil timer thread"),
    _destroyed(false)
{
    __setNoDelete(true);
    start(0, priority);
    __setNoDelete(false);
}

void
Timer::destroy()
{
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
        if(_destroyed)
        {
            return;
        }
        _destroyed = true;
        _monitor.notify();
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
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
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
        throw IllegalArgumentException(__FILE__, __LINE__, "task is already schedulded");
    }
    _tokens.insert(Token(time, IceUtil::Time(), task));

    if(_wakeUpTime == IceUtil::Time() || time < _wakeUpTime)
    {
        _monitor.notify();
    }
}

void
Timer::scheduleRepeated(const TimerTaskPtr& task, const IceUtil::Time& delay)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
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
        throw IllegalArgumentException(__FILE__, __LINE__, "task is already schedulded");
    }
    _tokens.insert(token); 
   
    if(_wakeUpTime == IceUtil::Time() || token.scheduledTime < _wakeUpTime)
    {
        _monitor.notify();
    }
}

bool
Timer::cancel(const TimerTaskPtr& task)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
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
            IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);

            if(!_destroyed)
            {
                //
                // If the task we just ran is a repeated task, schedule it
                // again for executation if it wasn't canceled.
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
                    _monitor.wait();
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
                try 
                {
                    _monitor.timedWait(first.scheduledTime - now);
                } 
                catch(const IceUtil::InvalidTimeoutException&)
                {
                    IceUtil::Time timeout = (first.scheduledTime - now) / 2;
                    while(timeout > IceUtil::Time())
                    {
                        try 
                        {
                            _monitor.timedWait(timeout);
                            break;
                        } 
                        catch(const IceUtil::InvalidTimeoutException&)
                        {
                            timeout = timeout / 2;
                        }
                    }
                }
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
                cerr << "IceUtil::Timer::run(): uncaught exception:\n" << e.what();
#ifdef __GNUC__
                cerr << "\n" << e.ice_stackTrace();
#endif
                cerr << endl;
            } 
            catch(const std::exception& e)
            {
                cerr << "IceUtil::Timer::run(): uncaught exception:\n" << e.what() << endl;
            } 
            catch(...)
            {
                cerr << "IceUtil::Timer::run(): uncaught exception" << endl;
            }
        }
    }
}

void
Timer::runTimerTask(const TimerTaskPtr& task)
{
    task->runTimerTask();
}
