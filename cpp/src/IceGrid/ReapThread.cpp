// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceGrid/ReapThread.h>

using namespace std;
using namespace IceGrid;

ReapThread::ReapThread() :
    _terminated(false)
{
}

void
ReapThread::run()
{
    vector<ReapableItem> reap;
    while(true)
    {
        {
            Lock sync(*this);
            if(_terminated)
            {
                break;
            }

            calcWakeInterval();

            //
            // If the wake interval is zero then we wait forever.
            //
            if(_wakeInterval == IceUtil::Time())
            {
                assert(_sessions.empty());
                wait();
            }
            else
            {
                assert(!_sessions.empty());
                timedWait(_wakeInterval);
            }
            
            if(_terminated)
            {
                break;
            }

            list<ReapableItem>::iterator p = _sessions.begin();
            while(p != _sessions.end())
            {
                try
                {
                    if((IceUtil::Time::now() - p->item->timestamp()) > p->timeout)
                    {
                        reap.push_back(*p);
                        p = _sessions.erase(p);
                    }
                    else
                    {
                        ++p;
                    }
                }
                catch(const Ice::ObjectNotExistException&)
                {
                    p = _sessions.erase(p);
                }
            }
        }

        for(vector<ReapableItem>::const_iterator p = reap.begin(); p != reap.end(); ++p)
        {
            p->item->destroy(false);
        }
        reap.clear();
    }
}

void
ReapThread::terminate()
{
    list<ReapableItem> reap;
    {
        Lock sync(*this);
        if(_terminated)
        {
            assert(_sessions.empty());
            return;
        }
        _terminated = true;
        notify();
        reap.swap(_sessions);
    }

    for(list<ReapableItem>::iterator p = reap.begin(); p != reap.end(); ++p)
    {
        p->item->destroy(true);
    }
}

void
ReapThread::add(const ReapablePtr& reapable, int timeout)
{
    Lock sync(*this);
    if(_terminated)
    {
        return;
    }

    //
    // 10 seconds is the minimum permissable timeout.
    //
    if(timeout < 10)
    {
        timeout = 10;
    }

    ReapableItem item;
    item.item = reapable;
    item.timeout = IceUtil::Time::seconds(timeout);
    _sessions.push_back(item);

    //
    // If there is a new minimum wake interval then wake the reaping
    // thread.
    //
    if(calcWakeInterval())
    {
        notify();
    }

    //
    // Since we just added a new session there must be a non-zero
    // wakeInterval.
    //
    assert(_wakeInterval != IceUtil::Time());
}

// Returns true iff the calculated wake interval is less than the
// current wake interval (or if the original wake interval was
// "forever").
bool
ReapThread::calcWakeInterval()
{
    // Re-calculate minimum timeout
    IceUtil::Time oldWakeInterval = _wakeInterval;
    IceUtil::Time minimum;
    bool first = true;
    for(list<ReapableItem>::const_iterator p = _sessions.begin(); p != _sessions.end(); ++p)
    {
        if(first || p->timeout < minimum)
        {
            minimum = p->timeout;
            first = false;
        }
    }

    _wakeInterval = minimum;
    return oldWakeInterval == IceUtil::Time() || minimum < oldWakeInterval;
}
