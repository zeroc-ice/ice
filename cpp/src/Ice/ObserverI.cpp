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
    
    ConnectionHelper(const ConnectionInfoPtr& con, const EndpointInfoPtr& endpt, ConnectionState state) : 
        MetricsHelperT("Connection"), _connection(con), _endpoint(endpt), _state(state)
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
            os << info->localAddress << ':' << info->localPort << " -> " << info->remoteAddress << ':' << info->remotePort;
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

class DispatchHelper : public MetricsHelperT<Metrics>
{
public:

    class Attributes : public AttributeResolverT<DispatchHelper>
    {
    public:
        
        Attributes()
        {
            add("parent", &DispatchHelper::getParent);
            add("id", &DispatchHelper::getId);

            addConnectionAttributes<DispatchHelper>(*this);

            add("operation", &DispatchHelper::getCurrent, &Current::operation);
            add("identityCategory", &DispatchHelper::getIdentity, &Identity::category);
            add("identityName", &DispatchHelper::getIdentity, &Identity::name);
            add("facet", &DispatchHelper::getCurrent, &Current::facet);
            add("encoding", &DispatchHelper::getCurrent, &Current::encoding);
            add("mode", &DispatchHelper::getMode);
        }
    };
    static Attributes attributes;
    
    DispatchHelper(const Current& current) : MetricsHelperT("Dispatch"), _current(current)
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
        os << _current.id.name << " [" << _current.operation << ']';
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

DispatchHelper::Attributes DispatchHelper::attributes;

class InvocationHelper : public MetricsHelperT<InvocationMetrics>
{
public:

    class Attributes : public AttributeResolverT<InvocationHelper>
    {
    public:
        
        Attributes()
        {
            add("parent", &InvocationHelper::getParent);
            add("id", &InvocationHelper::getId);

            add("operation", &InvocationHelper::_operation);
            add("identityCategory", &InvocationHelper::getIdentity, &Identity::category);
            add("identityName", &InvocationHelper::getIdentity, &Identity::name);
            add("facet", &InvocationHelper::getProxy, &IceProxy::Ice::Object::ice_getFacet);
            add("encoding", &InvocationHelper::getProxy, &IceProxy::Ice::Object::ice_getEncodingVersion);
            add("mode", &InvocationHelper::getMode);
            add("proxy", &InvocationHelper::getProxy);
        }
    };
    static Attributes attributes;
    
    InvocationHelper(const Ice::ObjectPrx& proxy, const string& op, const Ice::Context& ctx = Ice::Context()) :
        MetricsHelperT("Invocation"), _proxy(proxy), _operation(op), _context(ctx)
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
        os << _proxy << " [" << _operation << ']';
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


    Identity
    getIdentity() const
    {
        return _proxy->ice_getIdentity();
    }

private:

    const ObjectPrx& _proxy;
    string _operation;
    const Ice::Context& _context;
};

InvocationHelper::Attributes InvocationHelper::attributes;

class RemoteInvocationHelper : public MetricsHelperT<Metrics>
{
public:

    class Attributes : public AttributeResolverT<RemoteInvocationHelper>
    {
    public:
        
        Attributes()
        {
            add("parent", &RemoteInvocationHelper::getParent);
            add("id", &RemoteInvocationHelper::getId);
            addConnectionAttributes<RemoteInvocationHelper>(*this);
        }
    };
    static Attributes attributes;
    
    RemoteInvocationHelper(const ConnectionPtr& con) : MetricsHelperT("Remote"), _connection(con)
    {
    }

    virtual string operator()(const string& attribute) const
    {
        return attributes(this, attribute);
    }

    string 
    getId() const
    {
        ostringstream os;
        IPConnectionInfoPtr info = IPConnectionInfoPtr::dynamicCast(_connection->getInfo());
        if(info)
        {
            os << info->remoteAddress << ':' << info->remotePort;
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
        if(_connection->getAdapter())
        {
            return _connection->getAdapter()->getName();
        }
        else
        {
            return "Communicator";
        }
    }
    
    ::Ice::ConnectionInfoPtr
    getConnectionInfo() const
    {
        return _connection->getInfo();
    }

    ::Ice::EndpointInfoPtr
    getEndpointInfo() const
    {
        return _connection->getEndpoint()->getInfo();
    }
    
private:

    ConnectionPtr _connection;
};

RemoteInvocationHelper::Attributes RemoteInvocationHelper::attributes;

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
        MetricsHelperT("Thread"), _parent(parent), _id(id), _state(state)
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

class EndpointHelper : public MetricsHelperT<Metrics>
{
public:

    class Attributes : public AttributeResolverT<EndpointHelper>
    {
    public:
        
        Attributes()
        {
            add("parent", &EndpointHelper::getParent);
            add("id", &EndpointHelper::_id);
            addEndpointAttributes<EndpointHelper>(*this);
        }
    };
    static Attributes attributes;
    
    EndpointHelper(const string& mapName, 
                   const EndpointInfoPtr& endpt, 
                   const string& id) : MetricsHelperT(mapName), _id(id), _endpoint(endpt)
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

EndpointHelper::Attributes EndpointHelper::attributes;

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

InvocationObserverI::InvocationObserverI()
{
}

void
InvocationObserverI::retried()
{
    forEach(inc(&InvocationMetrics::retry));
}

ObserverPtr
InvocationObserverI::getRemoteObserver(const Ice::ConnectionPtr& connection)
{
    return getObserver<ObserverI>(RemoteInvocationHelper(connection));
}

CommunicatorObserverI::CommunicatorObserverI(const MetricsAdminIPtr& metrics) : 
    _metrics(metrics),
    _connections(metrics),
    _dispatch(metrics),
    _invocations(metrics),
    _threads(metrics),
    _connects(metrics),
    _endpointLookups(metrics)
{
    metrics->addFactory("Connection", _connections.newFactory());
    metrics->addFactory("Thread", _threads.newFactory());
    metrics->addFactory("Dispatch", _dispatch.newFactory());
    metrics->addFactory("ConnectionEstablishment", _connects.newFactory());
    metrics->addFactory("EndpointLookup", _endpointLookups.newFactory());

    map<string, MetricsMap InvocationMetrics::*> subMaps;
    subMaps["Remote"] = &InvocationMetrics::remotes;
    metrics->addFactory("Invocation", _invocations.newFactory(subMaps));
}

void
CommunicatorObserverI::setObserverUpdater(const ObserverUpdaterPtr& updater)
{
    _metrics->addUpdater("Connection", newUpdater(updater, &ObserverUpdater::updateConnectionObservers));
    _metrics->addUpdater("Thread", newUpdater(updater, &ObserverUpdater::updateThreadObservers));
}

ObserverPtr
CommunicatorObserverI::getConnectionEstablishmentObserver(const Ice::EndpointInfoPtr& endpt, const string& connector)
{
    return _connects.getObserver(EndpointHelper("ConnectionEstablishment", endpt, connector));
}

ObserverPtr
CommunicatorObserverI::getEndpointLookupObserver(const Ice::EndpointInfoPtr& endpt, const string& endpoint)
{
    return _endpointLookups.getObserver(EndpointHelper("EndpointLookup", endpt, endpoint));
}

ConnectionObserverPtr 
CommunicatorObserverI::getConnectionObserver(const ConnectionInfoPtr& con, 
                                             const EndpointInfoPtr& endpt,
                                             ConnectionState state, 
                                             const ConnectionObserverPtr& observer)
{
    return _connections.getObserver(ConnectionHelper(con, endpt, state), observer);
}

ThreadObserverPtr 
CommunicatorObserverI::getThreadObserver(const string& parent, 
                                         const string& id,
                                         ThreadState state,
                                         const ThreadObserverPtr& observer)
{
    return _threads.getObserver(ThreadHelper(parent, id, state), observer);
}

InvocationObserverPtr 
CommunicatorObserverI::getInvocationObserver(const ObjectPrx& proxy, const string& op)
{
    return _invocations.getObserver(InvocationHelper(proxy, op));
}

InvocationObserverPtr 
CommunicatorObserverI::getInvocationObserverWithContext(const ObjectPrx& proxy, 
                                                        const string& op, 
                                                        const Context& ctx)
{
    return _invocations.getObserver(InvocationHelper(proxy, op, ctx));
}

ObserverPtr 
CommunicatorObserverI::getDispatchObserver(const Current& current)
{
    return _dispatch.getObserver(DispatchHelper(current));
}
