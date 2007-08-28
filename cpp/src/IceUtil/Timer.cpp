// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Timer.h>
#include <IceUtil/Exception.h>

using namespace std;
using namespace IceUtil;

bool
TimerTask::operator<(const TimerTask& r) const
{
    return this < &r;
}

Timer::Timer() : _destroyed(false)
{
    start();
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
        _tokens.clear();
    }
    getThreadControl().join();
}

void
Timer::schedule(const TimerTaskPtr& task, const IceUtil::Time& time)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
    if(_destroyed)
    {
        return;
    }

#if defined(_MSC_VER) && (_MSC_VER < 1300)
    Token token;
    token.scheduledTime = time;
    token.task = task;
#else
    const Token token = { time, IceUtil::Time(), task };
#endif
    _tokens.insert(token);
    _tasks.insert(make_pair(task, token.scheduledTime));

    if(token.scheduledTime <= _tokens.begin()->scheduledTime)
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
        return;
    }

#if defined(_MSC_VER) && (_MSC_VER < 1300)
    Token token;
    token.scheduledTime = IceUtil::Time::now() + delay;
    token.delay = delay;
    token.task = task;
#else
    Token token = { IceUtil::Time::now() + delay, delay, task };
#endif
    _tokens.insert(token); 
    _tasks.insert(make_pair(task, token.scheduledTime));
   
    if(token.scheduledTime <= _tokens.begin()->scheduledTime)
    {
        _monitor.notify();
    }
}

bool
Timer::cancel(const TimerTaskPtr& task)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
    map<TimerTaskPtr, IceUtil::Time>::iterator p = _tasks.find(task);
    if(p == _tasks.end())
    {
        return false;
    }
    _tasks.erase(p);

#if defined(_MSC_VER) && (_MSC_VER < 1300)
    Token token;
    token.scheduledTime = p->second;
    token.task = task = p->first;
#else
    Token token = { p->second, IceUtil::Time(), p->first };
#endif
    _tokens.erase(token);
    return true;
}

void
Timer::run()
{
    Token token;
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
                    map<TimerTaskPtr, IceUtil::Time>::iterator p = _tasks.find(token.task);
                    if(p != _tasks.end())
                    {
                        token.scheduledTime = IceUtil::Time::now() + token.delay;
                        p->second = token.scheduledTime;
                        _tokens.insert(token);
                    }
                }
                token = Token();

                if(_tokens.empty())
                {
                    _monitor.wait();
                }
            }
            
            if(_destroyed)
            {
                break;
            }
            
            while(!_tokens.empty() && !_destroyed)
            {
                const IceUtil::Time now = IceUtil::Time::now();
                const Token& first = *(_tokens.begin());
                if(first.scheduledTime <= now)
                {
                    _tokens.erase(_tokens.begin());
                    if(token.delay == IceUtil::Time())
                    {
                        _tasks.erase(token.task);
                    }
                    token = first;
                    break;
                }

                _monitor.timedWait(first.scheduledTime - now);
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
                token.task->run();
            }
            catch(const std::exception& e)
            {
                cerr << "IceUtil::Timer::run(): uncaught exception: ";
                cerr << e.what() << endl;
            } 
            catch(...)
            {
                cerr << "IceUtil::Timer::run(): uncaught exception" << endl;
            }
        }
    }
}
