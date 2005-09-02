// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceStorm/QueuedProxy.h>

using namespace std;

IceStorm::QueuedProxy::QueuedProxy() :
    _busy(false)
{
}

void
IceStorm::QueuedProxy::publish(const EventPtr& event)
{
    IceUtil::Mutex::Lock sync(_mutex);

    if(_exception.get())
    {
        _exception->ice_throw();
    }

    _events.push_back(event);

    //
    // If another thread is busy delivering events, then we
    // have nothing left to do.
    //
    if(_busy)
    {
        return;
    }

    _busy = true;

    try
    {
        while(!_events.empty())
        {
            //
            // Get the current set of events, but release the lock before
            // attempting to deliver the events. This allows other threads
            // to add events in case we block (such as during connection
            // establishment).
            //
            vector<EventPtr> v;
            v.swap(_events);
            sync.release();

            //
            // Deliver the events without holding the lock.
            //
	    deliver(v);

            //
            // Reacquire the lock before we check the queue again.
            //
            sync.acquire();
        }

        _busy = false;
    }
    catch(const Ice::LocalException& ex)
    {
        assert(!sync.acquired());
        sync.acquire();
        _busy = false;
        _exception.reset(dynamic_cast<Ice::LocalException*>(ex.ice_clone()));
        throw;
    }
}

