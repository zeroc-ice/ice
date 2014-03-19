// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/ConnectionMonitor.h>
#include <Ice/ConnectionI.h>
#include <Ice/LocalException.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Instance.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(ConnectionMonitor* p) { return p; }

void
IceInternal::ConnectionMonitor::checkIntervalForACM(int acmTimeout)
{
    if(acmTimeout <= 0)
    {
        return;
    }

    //
    // If Ice.MonitorConnections isn't set (_interval == 0), the given ACM is used
    // to determine the check interval: 1/10 of the ACM timeout with a minmal value
    // of 5 seconds and a maximum value of 5 minutes.
    //
    // Note: if Ice.MonitorConnections is set, the timer is schedulded only if ACM 
    // is configured for the communicator or some object adapters.
    //
    int interval;
    if(_interval == 0)
    {
        interval = min(300, max(5, (int)acmTimeout / 10));
    }
    else if(_scheduledInterval == _interval)
    {
        return; // Nothing to do, the timer is already scheduled.
    }
    else
    {
        interval = _interval;
    }

    //
    // If no timer is scheduled yet or if the given ACM requires a smaller interval,
    // we re-schedule the timer.
    //
    Lock sync(*this);
    if(_scheduledInterval == 0 || _scheduledInterval > interval)
    {
        _scheduledInterval = interval;
        _instance->timer()->cancel(this);
        _instance->timer()->scheduleRepeated(this, IceUtil::Time::seconds(interval));
    }
}

void
IceInternal::ConnectionMonitor::destroy()
{
    IceUtil::Mutex::Lock sync(*this);
        
    assert(_instance);
    _instance = 0;
    _connections.clear();
}

void
IceInternal::ConnectionMonitor::add(const ConnectionIPtr& connection)
{
    IceUtil::Mutex::Lock sync(*this);
    assert(_instance);
    _connections.insert(connection);
}

void
IceInternal::ConnectionMonitor::remove(const ConnectionIPtr& connection)
{
    IceUtil::Mutex::Lock sync(*this);
    assert(_instance);
    _connections.erase(connection);
}

IceInternal::ConnectionMonitor::ConnectionMonitor(const InstancePtr& instance, int interval) :
    _instance(instance),
    _interval(interval),
    _scheduledInterval(0)
{
}

IceInternal::ConnectionMonitor::~ConnectionMonitor()
{
    assert(!_instance);
    assert(_connections.empty());
}

void
IceInternal::ConnectionMonitor::runTimerTask()
{
    set<ConnectionIPtr> connections;
    
    {
        IceUtil::Mutex::Lock sync(*this);
        if(!_instance)
        {
            return;
        }

        connections = _connections;
    }

        
    //
    // Monitor connections outside the thread synchronization, so
    // that connections can be added or removed during monitoring.
    //
    IceUtil::Time now = IceUtil::Time::now(IceUtil::Time::Monotonic);
    for(set<ConnectionIPtr>::const_iterator p = connections.begin(); p != connections.end(); ++p)
    {
        try
        {          
            (*p)->monitor(now);
        }
        catch(const exception& ex)
        {   
            IceUtil::Mutex::Lock sync(*this);
            if(!_instance)
            {
                return;
            }

            Error out(_instance->initializationData().logger);
            out << "exception in connection monitor:\n" << ex.what();
        }
        catch(...)
        {
            IceUtil::Mutex::Lock sync(*this);
            if(!_instance)
            {
                return;
            }

            Error out(_instance->initializationData().logger);
            out << "unknown exception in connection monitor";
        }
    }
}
