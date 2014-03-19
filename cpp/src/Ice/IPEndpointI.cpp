// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/IPEndpointI.h>
#include <Ice/ProtocolInstance.h>
#include <Ice/Instance.h>
#include <Ice/LocalException.h>
#include <Ice/PropertiesI.h>
#include <Ice/LoggerUtil.h>
#include <Ice/HashUtil.h>
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

IceUtil::Shared* IceInternal::upCast(IPEndpointI* p) { return p; }
IceUtil::Shared* IceInternal::upCast(EndpointHostResolver* p) { return p; }

IceInternal::IPEndpointInfoI::IPEndpointInfoI(const EndpointIPtr& endpoint) : _endpoint(endpoint)
{
}

IceInternal::IPEndpointInfoI::~IPEndpointInfoI()
{
}

Ice::Short
IceInternal::IPEndpointInfoI::type() const
{
    return _endpoint->type();
}

bool
IceInternal::IPEndpointInfoI::datagram() const
{
    return _endpoint->datagram();
}

bool
IceInternal::IPEndpointInfoI::secure() const
{
    return _endpoint->secure();
}

Ice::EndpointInfoPtr 
IceInternal::IPEndpointI::getInfo() const
{
    Ice::IPEndpointInfoPtr info = new IPEndpointInfoI(const_cast<IPEndpointI*>(this));
    fillEndpointInfo(info.get());
    return info;
}


Ice::Short
IceInternal::IPEndpointI::type() const
{
    return _instance->type();
}

const string&
IceInternal::IPEndpointI::protocol() const
{
    return _instance->protocol();
}

void
IceInternal::IPEndpointI::streamWrite(BasicStream* s) const
{
    s->startWriteEncaps();
    streamWriteImpl(s);
    s->endWriteEncaps();
}

const string&
IceInternal::IPEndpointI::connectionId() const
{
    return _connectionId;
}

EndpointIPtr
IceInternal::IPEndpointI::connectionId(const string& connectionId) const
{
    if(connectionId == _connectionId)
    {
        return const_cast<IPEndpointI*>(this);
    }
    else
    {
        return createEndpoint(_host, _port, connectionId);
    }
}

vector<ConnectorPtr>
IceInternal::IPEndpointI::connectors(Ice::EndpointSelectionType selType) const
{
    return _instance->resolve(_host, _port, selType, const_cast<IPEndpointI*>(this));
}

const std::string& 
IceInternal::IPEndpointI::host() const
{
    return _host;
}

int
IceInternal::IPEndpointI::port() const
{
    return _port;
}

void
IceInternal::IPEndpointI::connectors_async(Ice::EndpointSelectionType selType, const EndpointI_connectorsPtr& cb) const
{
    _instance->resolve(_host, _port, selType, const_cast<IPEndpointI*>(this), cb);
}

vector<EndpointIPtr>
IceInternal::IPEndpointI::expand() const
{
    vector<EndpointIPtr> endps;
    vector<string> hosts = getHostsForEndpointExpand(_host, _instance->protocolSupport(), false);
    if(hosts.empty())
    {
        endps.push_back(const_cast<IPEndpointI*>(this));
    }
    else
    {
        for(vector<string>::const_iterator p = hosts.begin(); p != hosts.end(); ++p)
        {
            endps.push_back(createEndpoint(*p, _port, _connectionId));
        }
    }
    return endps;
}

bool
IceInternal::IPEndpointI::equivalent(const EndpointIPtr& endpoint) const
{
    const IPEndpointI* ipEndpointI = dynamic_cast<const IPEndpointI*>(endpoint.get());
    if(!ipEndpointI)
    {
        return false;
    }
    return ipEndpointI->type() == type() && ipEndpointI->_host == _host && ipEndpointI->_port == _port;
}

Ice::Int
IceInternal::IPEndpointI::hash() const
{
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(hashMutex);
    if(!_hashInitialized)
    {
        _hashValue = 5381;
        hashAdd(_hashValue, type());
        hashInit(_hashValue);
    }
    return _hashValue;
}

string
IceInternal::IPEndpointI::options() const
{
    //
    // WARNING: Certain features, such as proxy validation in Glacier2,
    // depend on the format of proxy strings. Changes to toString() and
    // methods called to generate parts of the reference string could break
    // these features. Please review for all features that depend on the
    // format of proxyToString() before changing this and related code.
    //
    ostringstream s;

    if(!_host.empty())
    {
        s << " -h ";
        bool addQuote = _host.find(':') != string::npos;
        if(addQuote)
        {
            s << "\"";
        }
        s << _host;
        if(addQuote)
        {
            s << "\"";
        }
    }

    s << " -p " << _port;

    return s.str();
}

bool
IceInternal::IPEndpointI::operator==(const LocalObject& r) const
{
    const IPEndpointI* p = dynamic_cast<const IPEndpointI*>(&r);
    if(!p)
    {
        return false;
    }

    if(this == p)
    {
        return true;
    }

    if(_host != p->_host)
    {
        return false;
    }

    if(_port != p->_port)
    {
        return false;
    }

    if(_connectionId != p->_connectionId)
    {
        return false;
    }

    return true;
}

bool
IceInternal::IPEndpointI::operator<(const LocalObject& r) const
{
    const IPEndpointI* p = dynamic_cast<const IPEndpointI*>(&r);
    if(!p)
    {
        const EndpointI* e = dynamic_cast<const EndpointI*>(&r);
        if(!e)
        {
            return false;
        }
        return type() < e->type();
    }

    if(this == p)
    {
        return false;
    }

    if(type() < p->type())
    {
        return true;
    }
    else if(p->type() < type())
    {
        return false;
    }

    if(_host < p->_host)
    {
        return true;
    }
    else if(p->_host < _host)
    {
        return false;
    }

    if(_port < p->_port)
    {
        return true;
    }
    else if(p->_port < _port)
    {
        return false;
    }

    if(_connectionId < p->_connectionId)
    {
        return true;
    }
    else if(p->_connectionId < _connectionId)
    {
        return false;
    }

    return false;
}

vector<ConnectorPtr>
IceInternal::IPEndpointI::connectors(const vector<Address>& addresses, const NetworkProxyPtr& proxy) const
{
    vector<ConnectorPtr> connectors;
    for(unsigned int i = 0; i < addresses.size(); ++i)
    {
        connectors.push_back(createConnector(addresses[i], proxy));
    }
    return connectors;
}

void
IceInternal::IPEndpointI::streamWriteImpl(BasicStream* s) const
{
    s->write(_host, false);
    s->write(_port);
}

void
IceInternal::IPEndpointI::hashInit(Ice::Int& h) const
{
    hashAdd(h, _host);
    hashAdd(h, _port);
    hashAdd(h, _connectionId);
}

void
IceInternal::IPEndpointI::fillEndpointInfo(Ice::IPEndpointInfo* info) const
{
    info->host = _host;
    info->port = _port;
}

void
IceInternal::IPEndpointI::initWithOptions(vector<string>& args, bool oaEndpoint)
{
    EndpointI::initWithOptions(args);

    if(_host.empty())
    {
        const_cast<string&>(_host) = _instance->defaultHost();
    }
    else if(_host == "*")
    {
        if(oaEndpoint)
        {
            const_cast<string&>(_host) = string();
        }
        else
        {
            Ice::EndpointParseException ex(__FILE__, __LINE__);
            ex.str = "`-h *' not valid for proxy endpoint `" + toString() + "'";
            throw ex;
        }
    }
}

bool
IceInternal::IPEndpointI::checkOption(const string& option, const string& argument, const string& endpoint)
{
    switch(option[1])
    {
    case 'h':
    {
        if(argument.empty())
        {
            Ice::EndpointParseException ex(__FILE__, __LINE__);
            ex.str = "no argument provided for -h option in endpoint " + endpoint;
            throw ex;
        }
        const_cast<string&>(_host) = argument;
        return true;
    }

    case 'p':
    {
        if(argument.empty())
        {
            Ice::EndpointParseException ex(__FILE__, __LINE__);
            ex.str = "no argument provided for -p option in endpoint " + endpoint;
            throw ex;
        }
        istringstream p(argument);
        if(!(p >> const_cast<Ice::Int&>(_port)) || !p.eof())
        {
            Ice::EndpointParseException ex(__FILE__, __LINE__);
            ex.str = "invalid port value `" + argument + "' in endpoint " + endpoint;
            throw ex;
        }
        else if(_port < 0 || _port > 65535)
        {
            Ice::EndpointParseException ex(__FILE__, __LINE__);
            ex.str = "port value `" + argument + "' out of range in endpoint " + endpoint;
            throw ex;
        }
        return true;
    }

    default:
    {
        return false;
    }
    }
}

IceInternal::IPEndpointI::IPEndpointI(const ProtocolInstancePtr& instance, const string& host, int port, 
                                      const string& connectionId) :
    _instance(instance),
    _host(host),
    _port(port),
    _connectionId(connectionId),
    _hashInitialized(false)
{
}

IceInternal::IPEndpointI::IPEndpointI(const ProtocolInstancePtr& instance) :
    _instance(instance),
    _port(0),
    _hashInitialized(false)
{
}

IceInternal::IPEndpointI::IPEndpointI(const ProtocolInstancePtr& instance, BasicStream* s) :
    _instance(instance),
    _port(0),
    _hashInitialized(false)
{
    s->read(const_cast<string&>(_host), false);
    s->read(const_cast<Ice::Int&>(_port));
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
                                           const IPEndpointIPtr& endpoint)
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
                                           const IPEndpointIPtr& endpoint, const EndpointI_connectorsPtr& callback)
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
                                           const IPEndpointIPtr& endpoint)
{
    return endpoint->connectors(getAddresses(host, port, _instance->protocolSupport(), selType,
                                             _instance->preferIPv6(), false));
}

void
IceInternal::EndpointHostResolver::resolve(const string&, int,
                                           Ice::EndpointSelectionType selType,
                                           const IPEndpointIPtr& endpoint,
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
