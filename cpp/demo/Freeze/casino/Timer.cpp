// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Timer.h>
#include <iostream>

using namespace std;
using namespace IceUtil;

Timer::Timer() :
    _canceled(false)
{
    start();
}

void
Timer::cancel()
{
    bool join = false;
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
        if(!_canceled)
        {
            _canceled = true;
            join = true;
            _monitor.notify();
        }
    }
    
    if(join)
    {
        getThreadControl().join();
    }
}


void 
Timer::schedule(const TimerTaskPtr& task, const IceUtil::Time& time)
{
#if defined(_MSC_VER) && (_MSC_VER < 1300)
    Entry entry;
    entry.task = task;
    entry.time = time;
#else
    Entry entry = { task, time };
#endif

    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);

    // cout << "Scheduling task for " << time.toDateTime() << endl;

    bool notify = _queue.empty();

    //
    // Insert it at the proper position
    //
    bool inserted = false;
    deque<Entry>::iterator p = _queue.begin();
    while(!inserted && p != _queue.end())
    {
        if(time < p->time)
        {
            _queue.insert(p, entry);
            inserted = true;
        }
        else
        {
            ++p;
        }
    }

    if(!inserted)
    {
        _queue.push_back(entry);
    }
    
    if(notify)
    {
        _monitor.notify();
    }
}

void
Timer::run()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);

    while(!_canceled)
    {
        while(!_canceled && _queue.empty())
        {
            _monitor.wait(); // wait forever
        }

        if(_canceled)
        {
            break;
        }
        
        assert(!_queue.empty());

        Entry entry = _queue.front();
        _queue.pop_front();

        bool ready = false;

        do
        {
            IceUtil::Time now = IceUtil::Time::now();

            ready = (entry.time <= now);
            if(!ready)
            {
                // cout << "Waiting for " << (entry.time - now).toDuration() << endl;

                ready = (_monitor.timedWait(entry.time - now) == false);
            }
        } while(!_canceled && !ready);

        if(ready)
        {
            sync.release();
            
            try
            {
                entry.task->run();
            }
            catch(...)
            {
                cerr << "Task raised an exception" << endl;
                // ignored
            }

            sync.acquire();
        }
    }
}
