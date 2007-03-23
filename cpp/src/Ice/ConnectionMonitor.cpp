// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        
        assert(_instance);
        _instance = 0;
        _connections.clear();
        
        notify();
    }

    getThreadControl().join();
}

void
IceInternal::ConnectionMonitor::add(const ConnectionIPtr& connection)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    assert(_instance);
    _connections.insert(connection);
}

void
IceInternal::ConnectionMonitor::remove(const ConnectionIPtr& connection)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    assert(_instance);
    _connections.erase(connection);
}

IceInternal::ConnectionMonitor::ConnectionMonitor(const InstancePtr& instance, int interval) :
    _instance(instance),
    _interval(IceUtil::Time::seconds(interval))
{
    assert(interval > 0);
    start();
}

IceInternal::ConnectionMonitor::~ConnectionMonitor()
{
    assert(!_instance);
    assert(_connections.empty());
}

void
IceInternal::ConnectionMonitor::run()
{
    while(true)
    {
        set<ConnectionIPtr> connections;
        
        {
            IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
            if(_instance && !timedWait(_interval))
            {
                connections = _connections;
            }

            if(!_instance)
            {
                return;
            }
        }
        
        //
        // Monitor connections outside the thread synchronization, so
        // that connections can be added or removed during monitoring.
        //
        for(set<ConnectionIPtr>::const_iterator p = connections.begin(); p != connections.end(); ++p)
        {
            try
            {          
                (*p)->monitor();
            }
            catch(const Exception& ex)
            {   
                IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
                if(!_instance)
                {
                    return;
                }

                Error out(_instance->initializationData().logger);
                out << "exception in connection monitor:\n" << ex;
            }
            catch(...)
            {
                IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
                if(!_instance)
                {
                    return;
                }

                Error out(_instance->initializationData().logger);
                out << "unknown exception in connection monitor";
            }
        }
    }
}
