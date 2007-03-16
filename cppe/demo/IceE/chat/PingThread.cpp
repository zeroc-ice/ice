// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <PingThread.h>

using namespace std;

SessionPingThread::SessionPingThread(const Glacier2::SessionPrx& session, long timeout) :
    _session(session),
    _timeout(IceUtil::Time::seconds(timeout)),
    _destroy(false)
{
}

void 
SessionPingThread::run()
{
    Lock sync(*this);
    while(!_destroy)
    {
        timedWait(_timeout);
        if(_destroy)
        {
            break;
        }
        try
        {
            _session->ice_ping();
        }
        catch(const Ice::Exception&)
        {
            break;
        }
    }
}

void 
SessionPingThread::destroy()
{
    Lock sync(*this);
    _destroy = true;
    notify();
}
