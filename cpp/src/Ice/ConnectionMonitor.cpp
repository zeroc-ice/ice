// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
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
    _instance(instance)
{
    assert(interval > 0);
    __setNoDelete(true);
    try
    {
        instance->timer()->scheduleRepeated(this, IceUtil::Time::seconds(interval));
    }
    catch(...)
    {
        __setNoDelete(false);
        throw;
    }
    __setNoDelete(false);
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
        catch(const Exception& ex)
        {   
            IceUtil::Mutex::Lock sync(*this);
            if(!_instance)
            {
                return;
            }

            Error out(_instance->initializationData().logger);
            out << "exception in connection monitor:\n" << ex;
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
