// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/EndpointI.h>
#include <Ice/Instance.h>
#include <Ice/LocalException.h>
#include <Ice/PropertiesI.h>
#include <Ice/LoggerUtil.h>
#include <IceUtil/MutexPtrLock.h>

using namespace std;
using namespace Ice::Instrumentation;
using namespace IceInternal;

namespace
{

IceUtil::Mutex* hashMutex = 0;

class Init
{
public:

    Init()
    {
        hashMutex = new IceUtil::Mutex;
    }

    ~Init()
    {
        delete hashMutex;
        hashMutex = 0;
    }
};

Init init;

}

Ice::LocalObject* IceInternal::upCast(EndpointI* p) { return p; }
IceUtil::Shared* IceInternal::upCast(EndpointHostResolver* p) { return p; }

vector<ConnectorPtr>
IceInternal::EndpointI::connectors(const vector<Address>& /*addrs*/, const NetworkProxyPtr& /*proxy*/) const
{
    //
    // This method must be extended by endpoints which use the EndpointHostResolver to create
    // connectors from IP addresses.
    //
    assert(false);
    return vector<ConnectorPtr>();
}

const string&
IceInternal::EndpointI::connectionId() const
{
    return _connectionId;
}

Ice::Int
IceInternal::EndpointI::internal_getHash() const
{
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(hashMutex);
    if(!_hashInitialized)
    {
        _hashValue = hashInit();
    }
    return _hashValue;
}

IceInternal::EndpointI::EndpointI(const std::string& connectionId) : 
    _connectionId(connectionId),
    _hashInitialized(false)
{
}

IceInternal::EndpointI::EndpointI() : 
    _hashInitialized(false) 
{
}

#ifndef ICE_OS_WINRT

IceInternal::EndpointHostResolver::EndpointHostResolver(const InstancePtr& instance) :
    IceUtil::Thread("Ice.HostResolver"),
    _instance(instance),
    _protocol(instance->protocolSupport()),
    _preferIPv6(instance->preferIPv6()),
    _destroyed(false)
{
    __setNoDelete(true);
    try
    {
        updateObserver();
        bool hasPriority = _instance->initializationData().properties->getProperty("Ice.ThreadPriority") != "";
        int priority = _instance->initializationData().properties->getPropertyAsInt("Ice.ThreadPriority");
        if(hasPriority)
        {
            start(0, priority);
        }
        else
        {
            start();
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        {
            Ice::Error out(_instance->initializationData().logger);
            out << "cannot create thread for enpoint host resolver:\n" << ex;
        }
        throw;
    }
    __setNoDelete(false);
}

vector<ConnectorPtr>
IceInternal::EndpointHostResolver::resolve(const string& host, int port, Ice::EndpointSelectionType selType, 
                                           const EndpointIPtr& endpoint)
{
    //
    // Try to get the addresses without DNS lookup. If this doesn't
    // work, we retry with DNS lookup (and observer).
    //
    NetworkProxyPtr networkProxy = _instance->networkProxy();
    if(!networkProxy)
    {
        vector<Address> addrs = getAddresses(host, port, _protocol, selType, _preferIPv6, false);
        if(!addrs.empty())
        {
            return endpoint->connectors(addrs, 0);
        }
    }

    ObserverHelperT<> observer;
    const CommunicatorObserverPtr& obsv = _instance->getObserver();
    if(obsv)
    {
        observer.attach(obsv->getEndpointLookupObserver(endpoint));
    }
    
    vector<ConnectorPtr> connectors;
    try 
    {
        if(networkProxy)
        {
            networkProxy = networkProxy->resolveHost();
        }

        connectors = endpoint->connectors(getAddresses(host, port, _protocol, selType, _preferIPv6, true),
                                          networkProxy);
    }
    catch(const Ice::LocalException& ex)
    {
        observer.failed(ex.ice_name());
        throw;
    }
    return connectors;
}

void
IceInternal::EndpointHostResolver::resolve(const string& host, int port, Ice::EndpointSelectionType selType, 
                                           const EndpointIPtr& endpoint, const EndpointI_connectorsPtr& callback)
{
    //
    // Try to get the addresses without DNS lookup. If this doesn't work, we queue a resolve
    // entry and the thread will take care of getting the endpoint addresses.
    //
    NetworkProxyPtr networkProxy = _instance->networkProxy();
    if(!networkProxy)
    {
        try
        {
            vector<Address> addrs = getAddresses(host, port, _protocol, selType, _preferIPv6, false);
            if(!addrs.empty())
            {
                callback->connectors(endpoint->connectors(addrs, 0));
                return;
            }
        }
        catch(const Ice::LocalException& ex)
        {
            callback->exception(ex);
            return;
        }
    }

    Lock sync(*this);
    assert(!_destroyed);

    ResolveEntry entry;
    entry.host = host;
    entry.port = port;
    entry.selType = selType;
    entry.endpoint = endpoint;
    entry.callback = callback;

    const CommunicatorObserverPtr& obsv = _instance->getObserver();
    if(obsv)
    {
        entry.observer = obsv->getEndpointLookupObserver(endpoint);
        if(entry.observer)
        {
            entry.observer->attach();
        }
    }

    _queue.push_back(entry);
    notify();
}

void
IceInternal::EndpointHostResolver::destroy()
{
    Lock sync(*this);
    assert(!_destroyed);
    _destroyed = true;
    notify();
}

void
IceInternal::EndpointHostResolver::run()
{
    while(true)
    {
        ResolveEntry r;
        ThreadObserverPtr threadObserver;
        {
            Lock sync(*this);
            while(!_destroyed && _queue.empty())
            {
                wait();
            }

            if(_destroyed)
            {
                break;
            }

            r = _queue.front();
            _queue.pop_front();
            threadObserver = _observer.get();
        }

        try
        {
            if(threadObserver)
            {
                threadObserver->stateChanged(ThreadStateIdle, ThreadStateInUseForOther);
            }

            NetworkProxyPtr networkProxy = _instance->networkProxy();
            if(networkProxy)
            {
                networkProxy = networkProxy->resolveHost();
            }

            r.callback->connectors(r.endpoint->connectors(getAddresses(r.host, 
                                                                       r.port, 
                                                                       _protocol,
                                                                       r.selType, 
                                                                       _preferIPv6, true),
                                                          networkProxy));

            if(threadObserver)
            {
                threadObserver->stateChanged(ThreadStateInUseForOther, ThreadStateIdle);
            }

            if(r.observer)
            {
                r.observer->detach();
            }
        }
        catch(const Ice::LocalException& ex)
        {
            if(r.observer)
            {
                r.observer->failed(ex.ice_name());
                r.observer->detach();
            }
            r.callback->exception(ex);
        }
    }

    for(deque<ResolveEntry>::const_iterator p = _queue.begin(); p != _queue.end(); ++p)
    {
        Ice::CommunicatorDestroyedException ex(__FILE__, __LINE__);
        if(p->observer)
        {
            p->observer->failed(ex.ice_name());
            p->observer->detach();
        }
        p->callback->exception(ex);
    }
    _queue.clear();

    if(_observer)
    {
        _observer.detach();
    }
}

void
IceInternal::EndpointHostResolver::updateObserver()
{
    Lock sync(*this);
    const CommunicatorObserverPtr& obsv = _instance->getObserver();
    if(obsv)
    {
        _observer.attach(obsv->getThreadObserver("Communicator", name(), ThreadStateIdle, _observer.get()));
    }
}

#else

IceInternal::EndpointHostResolver::EndpointHostResolver(const InstancePtr& instance) :
    _instance(instance)
{
}

vector<ConnectorPtr>
IceInternal::EndpointHostResolver::resolve(const string& host, int port, Ice::EndpointSelectionType selType, 
                                           const EndpointIPtr& endpoint)
{
    vector<Address> addrs =
        getAddresses(host, port, _instance->protocolSupport(), selType, _instance->preferIPv6(), false);
    return endpoint->connectors(addrs, 0);
}

void
IceInternal::EndpointHostResolver::resolve(const string&, int,
                                           Ice::EndpointSelectionType selType, 
                                           const EndpointIPtr& endpoint, 
                                           const EndpointI_connectorsPtr& callback)
{
    //
    // No DNS lookup support with WinRT.
    //
    callback->connectors(endpoint->connectors(selType));
}

void
IceInternal::EndpointHostResolver::destroy()
{
}

void
IceInternal::EndpointHostResolver::run()
{
}

void
IceInternal::EndpointHostResolver::updateObserver()
{
}

#endif
