// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceGrid/WaitQueue.h>

using namespace std;
using namespace IceGrid;

WaitItem::WaitItem()
{
}

WaitItem::~WaitItem()
{
}

const IceUtil::Time&
WaitItem:: getExpirationTime()
{
    return _expiration;
}

void
WaitItem::setExpirationTime(const IceUtil::Time& time)
{
    _expiration = time;
}

WaitQueue::WaitQueue() :
    Thread("IceGrid wait queue thread"),
    _destroyed(false)
{
}

void
WaitQueue::run()
{
    while(true)
    {
        list<WaitItemPtr> expired;
        {
            Lock sync(*this);
            if(_waitQueue.empty() && !_destroyed)
            {
                wait();
            }

            if(_destroyed)
            {
                break;
            }

            //
            // Notify expired items.
            //
            while(!_waitQueue.empty() && !_destroyed)
            {
                WaitItemPtr item = _waitQueue.front();
                if(item->getExpirationTime() <= IceUtil::Time::now(IceUtil::Time::Monotonic))
                {
                    expired.push_back(item);
                    _waitQueue.pop_front();
                }
                else if(!expired.empty())
                {
                    break;
                }
                else
                {
                    //
                    // Wait until the next item expire or a notification. Note: in any case we
                    // get out of this loop to get a chance to execute the work queue.
                    //
                    timedWait(item->getExpirationTime() - IceUtil::Time::now(IceUtil::Time::Monotonic));
                }
            }
        }

        if(!expired.empty())
        {
            for(list<WaitItemPtr>::iterator p = expired.begin(); p != expired.end(); ++p)
            {
                try
                {
                    (*p)->expired(false);
                }
                catch(const Ice::LocalException&)
                {
                    //
                    // TODO: Add some tracing.
                    //
                }
            }
        }

        if(_destroyed)
        {
            break;
        }
    }

    if(!_waitQueue.empty())
    {
        for(list<WaitItemPtr>::iterator p = _waitQueue.begin(); p != _waitQueue.end(); ++p)
        {
            (*p)->expired(true);
        }
    }
    _waitQueue.clear(); // Break cyclic reference counts.
}

void
WaitQueue::destroy()
{
    {
        Lock sync(*this);
        _destroyed = true;
        notify();
    }

    getThreadControl().join();
}

void
WaitQueue::add(const WaitItemPtr& item, const IceUtil::Time& wait)
{
    Lock sync(*this);
    if(_destroyed)
    {
        return;
    }

    //
    // We'll have to notify the thread if it's sleeping for good.
    //
    bool notifyThread = _waitQueue.empty();

    IceUtil::Time expire = IceUtil::Time::now(IceUtil::Time::Monotonic) + wait;
    item->setExpirationTime(expire);

    list<WaitItemPtr>::iterator p = _waitQueue.begin();
    while(p != _waitQueue.end())
    {
        if((*p)->getExpirationTime() >= expire)
        {
            break;
        }
        ++p;
    }
    _waitQueue.insert(p, item);

    if(notifyThread)
    {
        notify();
    }
}

bool
WaitQueue::remove(const WaitItemPtr& item)
{
    Lock sync(*this);
    list<WaitItemPtr>::iterator p = _waitQueue.begin();
    while(p != _waitQueue.end())
    {
        if((*p).get() == item.get())
        {
            _waitQueue.erase(p);
            return true;
        }
        ++p;
    }
    return false;
}
