// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UTIL_TIMER_H
#define ICE_UTIL_TIMER_H

#include <IceUtil/Shared.h>
#include <IceUtil/Thread.h>
#include <IceUtil/Monitor.h>
#include <IceUtil/Time.h>

#include <set>
#include <map>

namespace IceUtil
{

class Timer;
typedef IceUtil::Handle<Timer> TimerPtr;

//
// Extend the TimerTask class and override the runTimerTask() method to execute
// code at a specific time or repeatedly.
//
class ICE_UTIL_API TimerTask : virtual public IceUtil::Shared
{
public:

    virtual ~TimerTask() { }

    virtual void runTimerTask() = 0;
};
typedef IceUtil::Handle<TimerTask> TimerTaskPtr;

//
// The timer class is used to schedule tasks for one-time execution or
// repeated execution. Tasks are executed by the dedicated timer thread 
// sequentially.
//
class ICE_UTIL_API Timer : virtual public IceUtil::Shared, private IceUtil::Thread
{
public:

    //
    // Construct a timer and starts its execution thread.
    //
    Timer();


    //
    // Construct a timer and starts its execution thread with the priority.
    //
    Timer(int priority);

    //
    // Destroy the timer and detach its execution thread if the calling thread 
    // is the timer thread, join the timer execution thread otherwise.
    //
    void destroy();

    //
    // Schedule a task for execution after a given delay.
    //
    void schedule(const TimerTaskPtr&, const IceUtil::Time&);

    //
    // Schedule a task for repeated execution with the given delay
    // between each execution.
    //
    void scheduleRepeated(const TimerTaskPtr&, const IceUtil::Time&);

    //
    // Cancel a task. Returns true if the task has not yet run or if
    // it's a task scheduled for repeated execution. Returns false if
    // the task has already run, was already cancelled or was never
    // schedulded.
    //
    bool cancel(const TimerTaskPtr&);

protected:

    virtual void run();
    virtual void runTimerTask(const TimerTaskPtr&);

    struct Token
    {
        IceUtil::Time scheduledTime;
        IceUtil::Time delay;
        TimerTaskPtr task;

        inline Token(const IceUtil::Time&, const IceUtil::Time&, const TimerTaskPtr&);
        inline bool operator<(const Token& r) const;
    };

    IceUtil::Monitor<IceUtil::Mutex> _monitor;
    bool _destroyed;
    std::set<Token> _tokens;
    
    class TimerTaskCompare : public std::binary_function<TimerTaskPtr, TimerTaskPtr, bool>
    {
    public:
        
        bool operator()(const TimerTaskPtr& lhs, const TimerTaskPtr& rhs) const
        {
            return lhs.get() < rhs.get();
        }
    };
    std::map<TimerTaskPtr, IceUtil::Time, TimerTaskCompare> _tasks;
    IceUtil::Time _wakeUpTime;
};
typedef IceUtil::Handle<Timer> TimerPtr;

inline 
Timer::Token::Token(const IceUtil::Time& st, const IceUtil::Time& d, const TimerTaskPtr& t) :
    scheduledTime(st), delay(d), task(t)
{
}

inline bool
Timer::Token::operator<(const Timer::Token& r) const
{
    if(scheduledTime < r.scheduledTime)
    {
        return true;
    }
    else if(scheduledTime > r.scheduledTime)
    {
        return false;
    }
    
    return task.get() < r.task.get();
}

}

#endif

