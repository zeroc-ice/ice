// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef CASINO_TIMER_H
#define CASINO_TIMER_H

#include <IceUtil/IceUtil.h>
#include <deque>

//
// A simplified version of java.util.Timer just for this demo
//

class TimerTask : public IceUtil::Shared
{
public:
    
    virtual void run() = 0;
};

typedef IceUtil::Handle<TimerTask> TimerTaskPtr;

class Timer : public virtual IceUtil::Shared, private virtual IceUtil::Thread
{
public:

    Timer();
    
    void cancel();
    void schedule(const TimerTaskPtr&, const IceUtil::Time&);
   
private:

    struct Entry
    {
        TimerTaskPtr task;
        IceUtil::Time time;
    };

    virtual void run();

    IceUtil::Monitor<IceUtil::Mutex> _monitor;
    bool _canceled;
    std::deque<Entry> _queue;
    
};

typedef IceUtil::Handle<Timer> TimerPtr;

#endif

