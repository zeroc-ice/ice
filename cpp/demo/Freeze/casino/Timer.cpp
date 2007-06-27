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
    Entry entry = { task, time };

    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);

    bool notify = _queue.empty();

    //
    // Insert it at the proper position
    //
    deque<Entry>::iterator p = _queue.begin();
    while(p != _queue.end())
    {
        if(time < p->time)
        {
            _queue.insert(p, entry);
            break;
        }
        ++p;
    }

    if(p == _queue.end())
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
                ready == (_monitor.timedWait(now - entry.time) == false);
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
