// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
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

const Ice::EncodingVersion&
IceInternal::EndpointI::encoding() const
{
    return _encoding;
}

const Ice::ProtocolVersion&
IceInternal::EndpointI::protocol() const
{
    return _protocol;
}

const string&
IceInternal::EndpointI::connectionId() const
{
    return _connectionId;
}

Ice::Int
IceInternal::EndpointI::ice_getHash() const
{
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(hashMutex);
    if(!_hashInitialized)
    {
        _hashValue = hashInit();
    }
    return _hashValue;
}

vector<ConnectorPtr>
IceInternal::EndpointI::connectors(const vector<Address>& addrs) const
{
    //
    // This method must be extended by endpoints which use the EndpointHostResolver to create
    // connectors from IP addresses.
    //
    assert(false);
    return vector<ConnectorPtr>();
}

IceInternal::EndpointI::EndpointI(const Ice::ProtocolVersion& protocol, 
                                  const Ice::EncodingVersion& encoding, 
                                  const std::string& connectionId) : 
    _protocol(protocol),
    _encoding(encoding), 
    _connectionId(connectionId),
    _hashInitialized(false)
{
}

IceInternal::EndpointI::EndpointI() : 
    _protocol(Ice::currentProtocol),
    _encoding(Ice::currentEncoding), 
    _hashInitialized(false) 
{
}

void
IceInternal::EndpointI::parseOption(const string& option, const string& arg, const string& desc, const string& str)
{
    if(option == "-v")
    {
        if(arg.empty())
        {
            Ice::EndpointParseException ex(__FILE__, __LINE__);
            ex.str = "no argument provided for -v option in endpoint `" + desc + " " + str + "'";
            throw ex;
        }

        try 
        {
            const_cast<Ice::ProtocolVersion&>(_protocol) = Ice::stringToProtocolVersion(arg);
        }
        catch(const Ice::VersionParseException& e)
        {
            Ice::EndpointParseException ex(__FILE__, __LINE__);
            ex.str = "invalid protocol version `" + arg + "' in endpoint `" + desc + " " + str + "':\n" + e.str;
            throw ex;
        }
    }
    else if(option == "-e")
    {
        if(arg.empty())
        {
            Ice::EndpointParseException ex(__FILE__, __LINE__);
            ex.str = "no argument provided for -e option in endpoint `" + desc + " " + str + "'";
            throw ex;
        }

        try 
        {
            const_cast<Ice::EncodingVersion&>(_encoding) = Ice::stringToEncodingVersion(arg);
        }
        catch(const Ice::VersionParseException& e)
        {
            Ice::EndpointParseException ex(__FILE__, __LINE__);
            ex.str = "invalid encoding version `" + arg + "' in endpoint `" + desc + " " + str + "':\n" + e.str;
            throw ex;
        }
    }
    else 
    {
        Ice::EndpointParseException ex(__FILE__, __LINE__);
        ex.str = "unknown option `" + option + "' in `" + desc + " " + str + "'";
        throw ex;
    }
}

#ifndef ICE_OS_WINRT

IceInternal::EndpointHostResolver::EndpointHostResolver(const InstancePtr& instance) :
    IceUtil::Thread("Ice.HostResolver"),
    _instance(instance),
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
IceInternal::EndpointHostResolver::resolve(const string& host, int port, const EndpointIPtr& endpoint)
{
    //
    // Try to get the addresses without DNS lookup. If this doesn't
    // work, we retry with DNS lookup (and observer).
    //
    vector<struct sockaddr_storage> addrs = getAddresses(host, port, _instance->protocolSupport(), false);
    if(!addrs.empty())
    {
        return endpoint->connectors(addrs);
    }

    ObserverHelperT<> observer;
    const CommunicatorObserverPtr& obsv = _instance->initializationData().observer;
    if(obsv)
    {
        observer.attach(obsv->getEndpointLookupObserver(endpoint));
    }
    
    vector<ConnectorPtr> connectors;
    try 
    {
        connectors = endpoint->connectors(getAddresses(host, port, _instance->protocolSupport(), true));
    }
    catch(const Ice::LocalException& ex)
    {
        observer.failed(ex.ice_name());
        throw;
    }
    return connectors;
}

void
IceInternal::EndpointHostResolver::resolve(const string& host, int port, const EndpointIPtr& endpoint,
                                           const EndpointI_connectorsPtr& callback)
{
    //
    // Try to get the addresses without DNS lookup. If this doesn't work, we queue a resolve
    // entry and the thread will take care of getting the endpoint addresses.
    //
    try
    {
        vector<struct sockaddr_storage> addrs = getAddresses(host, port, _instance->protocolSupport(), false);
        if(!addrs.empty())
        {
            callback->connectors(endpoint->connectors(addrs));
            return;
        }
    }
    catch(const Ice::LocalException& ex)
    {
        callback->exception(ex);
        return;
    }

    Lock sync(*this);
    assert(!_destroyed);

    ResolveEntry entry;
    entry.host = host;
    entry.port = port;
    entry.endpoint = endpoint;
    entry.callback = callback;

    const CommunicatorObserverPtr& obsv = _instance->initializationData().observer;
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

        const ProtocolSupport protocol = _instance->protocolSupport();
        try
        {
            if(threadObserver)
            {
                threadObserver->stateChanged(ThreadStateIdle, ThreadStateInUseForOther);
            }

            r.callback->connectors(r.endpoint->connectors(getAddresses(r.host, r.port, protocol, true)));

            if(threadObserver)
            {
                threadObserver->stateChanged(ThreadStateInUseForOther, ThreadStateIdle);
            }
        }
        catch(const Ice::LocalException& ex)
        {
            if(r.observer)
            {
                r.observer->failed(ex.ice_name());
            }
            r.callback->exception(ex);
        }

        if(r.observer)
        {
            r.observer->detach();
        }
    }

    for(deque<ResolveEntry>::const_iterator p = _queue.begin(); p != _queue.end(); ++p)
    {
        Ice::CommunicatorDestroyedException ex(__FILE__, __LINE__);
        p->callback->exception(ex);
        if(p->observer)
        {
            p->observer->failed(ex.ice_name());
            p->observer->detach();
        }
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
    const CommunicatorObserverPtr& obsv = _instance->initializationData().observer;
    if(obsv)
    {
        _observer.attach(obsv->getThreadObserver("Communicator", name(), ThreadStateIdle, _observer.get()));
    }
}

#else

IceInternal::EndpointHostResolver::EndpointHostResolver(const InstancePtr& instance)
{
}

vector<ConnectorPtr>
IceInternal::EndpointHostResolver::resolve(const string& host, int port, const EndpointIPtr& endpoint)
{
    return endpoint->connectors();
}

void
IceInternal::EndpointHostResolver::resolve(const string&, int,
                                           const EndpointIPtr& endpoint, 
                                           const EndpointI_connectorsPtr& callback)
{
    //
    // No DNS lookup support with WinRT.
    //
    callback->connectors(endpoint->connectors());
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
