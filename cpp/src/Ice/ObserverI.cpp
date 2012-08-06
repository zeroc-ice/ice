// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/ObserverI.h>

#include <Ice/Connection.h>
#include <Ice/Endpoint.h>
#include <Ice/ObjectAdapter.h>

using namespace std;
using namespace Ice;
using namespace Ice::Instrumentation;
using namespace IceMX;

namespace 
{

int ConnectionMetrics::*
getConnectionStateMetric(ConnectionState s)
{
    switch(s)
    {
    case ConnectionStateValidating:
        return &ConnectionMetrics::validating;
    case ConnectionStateActive:
        return &ConnectionMetrics::active;
    case ConnectionStateHolding:
        return &ConnectionMetrics::holding;
    case ConnectionStateClosing:
        return &ConnectionMetrics::closing;
    case ConnectionStateClosed:
        return &ConnectionMetrics::closed;
    }
}    

int ThreadMetrics::*
getThreadStateMetric(ThreadState s)
{
    switch(s)
    {
    case ThreadStateIdle:
        return 0;
    case ThreadStateInUseForIO:
        return &ThreadMetrics::inUseForIO;
    case ThreadStateInUseForUser:
        return &ThreadMetrics::inUseForUser;
    case ThreadStateInUseForOther:
        return &ThreadMetrics::inUseForOther;
    }
}    

template<typename Helper>
void addEndpointAttributes(typename Helper::Attributes& attrs)
{
    attrs.add("endpointType", &Helper::getEndpointInfo, &EndpointInfo::type);
    attrs.add("endpointIsDatagram", &Helper::getEndpointInfo, &EndpointInfo::datagram);
    attrs.add("endpointIsSecure", &Helper::getEndpointInfo, &EndpointInfo::secure);
    attrs.add("endpointProtocolVersion", &Helper::getEndpointInfo, &EndpointInfo::protocol);
    attrs.add("endpointEncodingVersion", &Helper::getEndpointInfo, &EndpointInfo::encoding);
    attrs.add("endpointTimeout", &Helper::getEndpointInfo, &EndpointInfo::timeout);
    attrs.add("endpointCompress", &Helper::getEndpointInfo, &EndpointInfo::compress);
    
    attrs.add("endpointHost", &Helper::getEndpointInfo, &IPEndpointInfo::host);
    attrs.add("endpointPort", &Helper::getEndpointInfo, &IPEndpointInfo::port);
}

template<typename Helper>
void addConnectionAttributes(typename Helper::Attributes& attrs)
{
    attrs.add("incoming", &Helper::getConnectionInfo, &ConnectionInfo::incoming);
    attrs.add("adapterName", &Helper::getConnectionInfo, &ConnectionInfo::adapterName);
    attrs.add("connectionId", &Helper::getConnectionInfo, &ConnectionInfo::connectionId);
    
    attrs.add("localHost", &Helper::getConnectionInfo, &IPConnectionInfo::localAddress);
    attrs.add("localPort", &Helper::getConnectionInfo, &IPConnectionInfo::localPort);
    attrs.add("remoteHost", &Helper::getConnectionInfo, &IPConnectionInfo::remoteAddress);
    attrs.add("remotePort", &Helper::getConnectionInfo, &IPConnectionInfo::remotePort);
            
    attrs.add("mcastHost", &Helper::getConnectionInfo, &UDPConnectionInfo::mcastAddress);
    attrs.add("mcastPort", &Helper::getConnectionInfo, &UDPConnectionInfo::mcastPort);
    
    addEndpointAttributes<Helper>(attrs);
}

class ConnectionHelper : public MetricsHelperT<ConnectionMetrics>
{
public:

    class Attributes : public AttributeResolverT<ConnectionHelper>
    {
    public:
        
        Attributes()
        {
            add("parent", &ConnectionHelper::getParent);
            add("id", &ConnectionHelper::getId);

            addConnectionAttributes<ConnectionHelper>(*this);
        }
    };
    static Attributes attributes;
    
    ConnectionHelper(const ConnectionInfoPtr& con, 
                     const EndpointInfoPtr& endpt,
                     ConnectionState state) : 
        _connection(con), _endpoint(endpt), _state(state)
    {
    }

    virtual string operator()(const string& attribute) const
    {
        return attributes(this, attribute);
    }

    virtual void initMetrics(const ConnectionMetricsPtr& v) const
    {
        ++(v.get()->*getConnectionStateMetric(_state));
    }

    string 
    getId() const
    {
        ostringstream os;
        IPConnectionInfoPtr info = IPConnectionInfoPtr::dynamicCast(_connection);
        if(info)
        {
            os << info->localAddress << ':' << info->localPort << '/' << info->remoteAddress << ':' << info->remotePort;
        }
        else
        {
            os << "connection-" << _connection.get();
        }
        return os.str();
    }
    
    string 
    getParent() const
    {
        if(!_connection->adapterName.empty())
        {
            return _connection->adapterName;
        }
        else
        {
            return "Communicator";
        }
    }
    
    ::Ice::ConnectionInfoPtr
    getConnectionInfo() const
    {
        return _connection;
    }

    ::Ice::EndpointInfoPtr
    getEndpointInfo() const
    {
        return _endpoint;
    }
    
private:

    ConnectionInfoPtr _connection;
    EndpointInfoPtr _endpoint;
    ConnectionState _state;
};

ConnectionHelper::Attributes ConnectionHelper::attributes;

class DispatchRequestHelper : public MetricsHelperT<Metrics>
{
public:

    class Attributes : public AttributeResolverT<DispatchRequestHelper>
    {
    public:
        
        Attributes()
        {
            add("parent", &DispatchRequestHelper::getParent);
            add("id", &DispatchRequestHelper::getId);

            addConnectionAttributes<DispatchRequestHelper>(*this);

            add("operation", &DispatchRequestHelper::getCurrent, &Current::operation);
            add("identityCategory", &DispatchRequestHelper::getIdentity, &Identity::category);
            add("identityName", &DispatchRequestHelper::getIdentity, &Identity::name);
            add("facet", &DispatchRequestHelper::getCurrent, &Current::facet);
            add("encoding", &DispatchRequestHelper::getCurrent, &Current::encoding);
            add("mode", &DispatchRequestHelper::getMode);
        }
    };
    static Attributes attributes;
    
    DispatchRequestHelper(const Current& current) : _current(current)
    {
    }

    virtual string operator()(const string& attribute) const
    {
        if(attribute.find("context.") == 0)
        {
            Ice::Context::const_iterator p = _current.ctx.find(attribute.substr(8));
            if(p != _current.ctx.end())
            {
                return p->second;
            }
            return "unknown";
        }
        else
        {
            return attributes(this, attribute);
        }
    }

    string
    getMode() const
    {
        return _current.requestId == 0 ? "oneway" : "twoway";
    }

    string 
    getId() const
    {
        ostringstream os;
        if(!_current.id.category.empty())
        {
            os << _current.id.category << '/';
        }
        os << _current.id.name << '-' << _current.operation;
        return os.str();
    }

    string 
    getParent() const
    {
        return _current.adapter->getName();
    }

    ::Ice::ConnectionInfoPtr
    getConnectionInfo() const
    {
        return _current.con->getInfo();
    }

    ::Ice::EndpointInfoPtr
    getEndpointInfo() const
    {
        return _current.con->getEndpoint()->getInfo();
    }

    const Current&
    getCurrent() const
    {
        return _current;
    }

    const Identity&
    getIdentity() const
    {
        return _current.id;
    }

private:

    const Current& _current;
};

DispatchRequestHelper::Attributes DispatchRequestHelper::attributes;

class InvocationRequestHelper : public MetricsHelperT<Metrics>
{
public:

    class Attributes : public AttributeResolverT<InvocationRequestHelper>
    {
    public:
        
        Attributes()
        {
            add("parent", &InvocationRequestHelper::getParent);
            add("id", &InvocationRequestHelper::getId);

            addConnectionAttributes<InvocationRequestHelper>(*this);

            add("operation", &InvocationRequestHelper::_operation);
            add("identityCategory", &InvocationRequestHelper::getIdentity, &Identity::category);
            add("identityName", &InvocationRequestHelper::getIdentity, &Identity::name);
            add("facet", &InvocationRequestHelper::getProxy, &IceProxy::Ice::Object::ice_getFacet);
            add("encoding", &InvocationRequestHelper::getProxy, &IceProxy::Ice::Object::ice_getEncodingVersion);
            add("mode", &InvocationRequestHelper::getMode);
        }
    };
    static Attributes attributes;
    
    InvocationRequestHelper(const Ice::ObjectPrx& proxy, 
                            const string& op, 
                            const Ice::Context& ctx, 
                            const Ice::ConnectionPtr& con) :
        _proxy(proxy), _operation(op), _context(ctx), _connection(con)
    {
    }

    virtual string operator()(const string& attribute) const
    {
        if(attribute.find("context.") == 0)
        {
            Ice::Context::const_iterator p = _context.find(attribute.substr(8));
            if(p != _context.end())
            {
                return p->second;
            }
            return "unknown";
        }
        else
        {
            return attributes(this, attribute);
        }
    }

    string
    getMode() const
    {
        if(_proxy->ice_isTwoway())
        {
            return "twoway";
        } 
        else if(_proxy->ice_isOneway())
        {
            return "oneway";
        } 
        else if(_proxy->ice_isBatchOneway())
        {
            return "batch-oneway";
        } 
        else if(_proxy->ice_isDatagram())
        {
            return "datagram";
        } 
        else if(_proxy->ice_isBatchDatagram())
        {
            return "batch-datagram";
        } 
        else
        {
            return "unknown";
        }
    }

    string 
    getId() const
    {
        ostringstream os;
        const Ice::Identity& id = _proxy->ice_getIdentity();
        if(!id.category.empty())
        {
            os << id.category << '/';
        }
        os << id.name << '-' << _operation;
        return os.str();
    }

    string 
    getParent() const
    {
        return "Communicator";
    }

    const ObjectPrx&
    getProxy() const
    {
        return _proxy;
    }

    ::Ice::ConnectionInfoPtr
    getConnectionInfo() const
    {
        if(!_connection)
        {
            return 0;
        }
        return _connection->getInfo();
    }

    ::Ice::EndpointInfoPtr
    getEndpointInfo() const
    {
        if(!_connection)
        {
            return 0;
        }
        return _connection->getEndpoint()->getInfo();
    }

    Identity
    getIdentity() const
    {
        return _proxy->ice_getIdentity();
    }

private:

    const ObjectPrx& _proxy;
    string _operation;
    const Ice::Context& _context;
    const Ice::ConnectionPtr& _connection;
};

InvocationRequestHelper::Attributes InvocationRequestHelper::attributes;

class ThreadHelper : public MetricsHelperT<ThreadMetrics>
{
public:

    class Attributes : public AttributeResolverT<ThreadHelper>
    {
    public:
        
        Attributes()
        {
            add("parent", &ThreadHelper::_parent);
            add("id", &ThreadHelper::_id);
        }
    };
    static Attributes attributes;
    
    ThreadHelper(const string& parent, const string& id, ThreadState state) :
        _parent(parent), _id(id), _state(state)
    {
    }

    virtual string operator()(const string& attribute) const
    {
        return attributes(this, attribute);
    }

    virtual void initMetrics(const ThreadMetricsPtr& v) const
    {
        if(_state != ThreadStateIdle)
        {
            ++(v.get()->*getThreadStateMetric(_state));
        }
    }

private:
    
    const std::string _parent;
    const std::string _id;
    const ThreadState _state;
};

ThreadHelper::Attributes ThreadHelper::attributes;

class ConnectHelper : public MetricsHelperT<Metrics>
{
public:

    class Attributes : public AttributeResolverT<ConnectHelper>
    {
    public:
        
        Attributes()
        {
            add("parent", &ConnectHelper::getParent);
            add("id", &ConnectHelper::_id);
            addEndpointAttributes<ConnectHelper>(*this);
        }
    };
    static Attributes attributes;
    
    ConnectHelper(const EndpointInfoPtr& endpt, const string& id) : _id(id), _endpoint(endpt)
    {
    }

    virtual string operator()(const string& attribute) const
    {
        return attributes(this, attribute);
    }

    Ice::EndpointInfoPtr
    getEndpointInfo() const
    {
        return _endpoint;
    }

    std::string
    getParent() const
    {
        return "Communicator";
    }

private:
    
    const std::string _id;
    const Ice::EndpointInfoPtr _endpoint;
};

ConnectHelper::Attributes ConnectHelper::attributes;

}

void
ConnectionObserverI::detach()
{
    ObserverT<ConnectionMetrics>::detach();
    forEach(dec(&ConnectionMetrics::closed));
}

void
ConnectionObserverI::stateChanged(ConnectionState oldState, ConnectionState newState)
{
    struct StateChanged 
    {
        StateChanged(ConnectionState oldState, ConnectionState newState) : 
            oldState(oldState), newState(newState)
        {
        }

        void operator()(const ConnectionMetricsPtr& v)
        {
            --(v.get()->*getConnectionStateMetric(oldState));
            ++(v.get()->*getConnectionStateMetric(newState));
        }


        ConnectionState oldState;
        ConnectionState newState;
    };
    forEach(StateChanged(oldState, newState));
}

void 
ConnectionObserverI::sentBytes(Int num, Long duration)
{
    forEach(chain(add(&ConnectionMetrics::sentBytes, num), add(&ConnectionMetrics::sentTime, duration)));
}

void 
ConnectionObserverI::receivedBytes(Int num, Long duration)
{
    forEach(chain(add(&ConnectionMetrics::receivedBytes, num), add(&ConnectionMetrics::receivedTime, duration)));
}

void
ThreadObserverI::stateChanged(ThreadState oldState, ThreadState newState)
{
    struct StateChanged 
    {
        StateChanged(ThreadState oldState, ThreadState newState) : oldState(oldState), newState(newState)
        {
        }

        void operator()(const ThreadMetricsPtr& v)
        {
            if(oldState != ThreadStateIdle)
            {
                --(v.get()->*getThreadStateMetric(oldState));
            }
            if(newState != ThreadStateIdle)
            {
                ++(v.get()->*getThreadStateMetric(newState));
            }
        }


        ThreadState oldState;
        ThreadState newState;
    };
    forEach(StateChanged(oldState, newState));
}

ObserverResolverI::ObserverResolverI(const MetricsAdminIPtr& metrics) : 
    _metrics(metrics),
    _connections("Connection", metrics),
    _requests("Request", metrics),
    _threads("Thread", metrics),
    _locatorQueries("LocatorQuery", metrics),
    _connects("Connect", metrics),
    _endpointResolves("EndpointResolve", metrics)
{
}

void
ObserverResolverI::setObserverUpdater(const ObserverUpdaterPtr& updater)
{
    _metrics->addUpdater("Connection", newUpdater(updater, &ObserverUpdater::updateConnectionObservers));
    _metrics->addUpdater("Thread", newUpdater(updater, &ObserverUpdater::updateThreadObservers));
}

Ice::Instrumentation::ObserverPtr
ObserverResolverI::getLocatorQueryObserver(const string& adapterId)
{
    return 0;
}

Ice::Instrumentation::ObserverPtr
ObserverResolverI::getConnectObserver(const Ice::EndpointInfoPtr& endpt, const string& connector)
{
    return _connects.getObserver(ConnectHelper(endpt, connector));
}

Ice::Instrumentation::ObserverPtr
ObserverResolverI::getEndpointResolveObserver(const Ice::EndpointInfoPtr& endpt, const string& endpoint)
{
    return _endpointResolves.getObserver(ConnectHelper(endpt, endpoint));
}

ConnectionObserverPtr 
ObserverResolverI::getConnectionObserver(const ConnectionInfoPtr& con, 
                                         const EndpointInfoPtr& endpt,
                                         ConnectionState state, 
                                         const ConnectionObserverPtr& observer)
{
    return _connections.getObserver(ConnectionHelper(con, endpt, state), observer);
}

ThreadObserverPtr 
ObserverResolverI::getThreadObserver(const string& parent, 
                                     const string& id,
                                     Ice::Instrumentation::ThreadState state,
                                     const Ice::Instrumentation::ThreadObserverPtr& observer)
{
    return _threads.getObserver(ThreadHelper(parent, id, state), observer);
}

ObserverPtr 
ObserverResolverI::getInvocationObserver(const ObjectPrx& proxy, 
                                         const string& op, 
                                         const Context& ctx, 
                                         const ConnectionPtr& con)
{
    return _requests.getObserver(InvocationRequestHelper(proxy, op, ctx, con));
}

ObserverPtr 
ObserverResolverI::getDispatchObserver(const Current& current)
{
    return _requests.getObserver(DispatchRequestHelper(current));
}
