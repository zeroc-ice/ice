// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/InstrumentationI.h>

#include <Ice/Connection.h>
#include <Ice/Endpoint.h>
#include <Ice/ObjectAdapter.h>
#include <Ice/LocalException.h>
#include <Ice/Communicator.h>
#include <Ice/LoggerUtil.h>

using namespace std;
using namespace Ice;
using namespace Ice::Instrumentation;
using namespace IceMX;

namespace 
{

Context emptyCtx;

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
    default:
        assert(false);
        return 0;
    }
}    

struct ThreadStateChanged 
{
    ThreadStateChanged(ThreadState oldState, ThreadState newState) : oldState(oldState), newState(newState)
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
            add("endpoint", &ConnectionHelper::getEndpoint);
            add("state", &ConnectionHelper::getState);
            addConnectionAttributes<ConnectionHelper>(*this);
        }
    };
    static Attributes attributes;
    
    ConnectionHelper(const ConnectionInfoPtr& con, const EndpointPtr& endpt, ConnectionState state) : 
        _connectionInfo(con), _endpoint(endpt), _state(state)
    {
    }

    virtual string operator()(const string& attribute) const
    {
        return attributes(this, attribute);
    }

    const string&
    getId() const
    {
        if(_id.empty())
        {
            ostringstream os;
            IPConnectionInfoPtr info = IPConnectionInfoPtr::dynamicCast(_connectionInfo);
            if(info)
            {
                os << info->localAddress << ':' << info->localPort;
                os << " -> ";
                os << info->remoteAddress << ':' << info->remotePort;
            }
            else
            {
                os << "connection-" << _connectionInfo.get();
            }
            if(!_connectionInfo->connectionId.empty())
            {
                os << " [" << _connectionInfo->connectionId << "]";
            }
            _id = os.str();
        }
        return _id;
    }

    string
    getState() const
    {
        switch(_state)
        {
        case ConnectionStateValidating:
            return "validating";
        case ConnectionStateHolding:
            return "holding";
        case ConnectionStateActive:
            return "active";
        case ConnectionStateClosing:
            return "closing";
        case ConnectionStateClosed:
            return "closed";
        default:
            assert(false);
            return "";
        }
    }
    
    string 
    getParent() const
    {
        if(!_connectionInfo->adapterName.empty())
        {
            return _connectionInfo->adapterName;
        }
        else
        {
            return "Communicator";
        }
    }
    
    const ConnectionInfoPtr&
    getConnectionInfo() const
    {
        return _connectionInfo;
    }

    const EndpointPtr&
    getEndpoint() const
    {
        return _endpoint;
    }

    const EndpointInfoPtr&
    getEndpointInfo() const
    {
        if(!_endpointInfo)
        {
            _endpointInfo = _endpoint->getInfo();
        }
        return _endpointInfo;
    }
    
private:

    const ConnectionInfoPtr& _connectionInfo;
    const EndpointPtr& _endpoint;
    const ConnectionState _state;
    mutable string _id;
    mutable EndpointInfoPtr _endpointInfo;
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
            add("endpoint", &DispatchHelper::getEndpoint);
            add("connection", &DispatchHelper::getConnection);

            addConnectionAttributes<DispatchHelper>(*this);

            add("operation", &DispatchHelper::getCurrent, &Current::operation);
            add("identity", &DispatchHelper::getIdentity);
            add("facet", &DispatchHelper::getCurrent, &Current::facet);
            add("mode", &DispatchHelper::getMode);

            setDefault(&DispatchHelper::resolve);
        }
    };
    static Attributes attributes;
    
    DispatchHelper(const Current& current) : _current(current)
    {
    }

    virtual string operator()(const string& attribute) const
    {
        return attributes(this, attribute);
    }

    string resolve(const string& attribute) const
    {
        if(attribute.compare(0, 8, "context.") == 0)
        {
            Context::const_iterator p = _current.ctx.find(attribute.substr(8));
            if(p != _current.ctx.end())
            {
                return p->second;
            }
        }
        throw invalid_argument(attribute);
    }

    string
    getMode() const
    {
        return _current.requestId == 0 ? "oneway" : "twoway";
    }

    const string&
    getId() const
    {
        if(_id.empty())
        {
            ostringstream os;
            if(!_current.id.category.empty())
            {
                os << _current.id.category << '/';
            }
            os << _current.id.name << " [" << _current.operation << ']';
            _id = os.str();
        }
        return _id;
    }

    string 
    getParent() const
    {
        return _current.adapter->getName();
    }

    ConnectionInfoPtr
    getConnectionInfo() const
    {
        return _current.con->getInfo();
    }

    EndpointPtr
    getEndpoint() const
    {
        return _current.con->getEndpoint();
    }

    const ConnectionPtr&
    getConnection() const
    {
        return _current.con;
    }

    const EndpointInfoPtr&
    getEndpointInfo() const
    {
        if(!_endpointInfo)
        {
            _endpointInfo = _current.con->getEndpoint()->getInfo();
        }
        return _endpointInfo;
    }

    const Current&
    getCurrent() const
    {
        return _current;
    }

    string
    getIdentity() const
    {
        return _current.adapter->getCommunicator()->identityToString(_current.id);
    }

private:

    const Current& _current;
    mutable string _id;
    mutable EndpointInfoPtr _endpointInfo;
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

            add("operation", &InvocationHelper::getOperation);
            add("identity", &InvocationHelper::getIdentity);
            add("facet", &InvocationHelper::getProxy, &IceProxy::Ice::Object::ice_getFacet);
            add("encoding", &InvocationHelper::getProxy, &IceProxy::Ice::Object::ice_getEncodingVersion);
            add("mode", &InvocationHelper::getMode);
            add("proxy", &InvocationHelper::getProxy);

            setDefault(&InvocationHelper::resolve);
        }
    };
    static Attributes attributes;
    
    InvocationHelper(const ObjectPrx& proxy, const string& op, const Context& ctx = emptyCtx) :
        _proxy(proxy), _operation(op), _context(ctx)
    {
    }

    string resolve(const string& attribute) const
    {
        if(attribute.compare(0, 8, "context.") == 0)
        {
            Context::const_iterator p = _context.find(attribute.substr(8));
            if(p != _context.end())
            {
                return p->second;
            }
        }
        throw invalid_argument(attribute);
    }

    virtual string operator()(const string& attribute) const
    {
        return attributes(this, attribute);
    }

    string
    getMode() const
    {
        if(!_proxy)
        {
            throw invalid_argument("mode");
        }

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
            throw invalid_argument("mode");
        }
    }

    const string&
    getId() const
    {
        if(_id.empty())
        {
            ostringstream os;
            if(_proxy)
            {
                try
                {
                    os << _proxy << " [" << _operation << ']';
                }
                catch(const FixedProxyException&)
                {
                    os << _proxy->ice_getCommunicator()->identityToString(_proxy->ice_getIdentity());
                    os << " [" << _operation << ']';
                }
            }
            else
            {
                os << _operation;
            }
            _id = os.str();
        }
        return _id;
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


    string
    getIdentity() const
    {
        if(_proxy)
        {
            return _proxy->ice_getCommunicator()->identityToString(_proxy->ice_getIdentity());
        }
        else
        {
            return "";
        }
    }

    const string&
    getOperation() const
    {
        return _operation;
    }

private:

    const ObjectPrx& _proxy;
    const string& _operation;
    const Context& _context;
    mutable string _id;
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
            add("endpoint", &RemoteInvocationHelper::getEndpoint);
            addConnectionAttributes<RemoteInvocationHelper>(*this);
        }
    };
    static Attributes attributes;
    
    RemoteInvocationHelper(const ConnectionInfoPtr& con, const EndpointPtr& endpt) : 
        _connectionInfo(con), _endpoint(endpt)
    {
    }

    virtual string operator()(const string& attribute) const
    {
        return attributes(this, attribute);
    }

    const string&
    getId() const
    {
        if(_id.empty())
        {
            _id = _endpoint->toString();
            if(!_connectionInfo->connectionId.empty())
            {
                _id += " [" + _connectionInfo->connectionId + "]";
            }
        }
        return _id;
    }
    
    string 
    getParent() const
    {
        if(!_connectionInfo->adapterName.empty())
        {
            return _connectionInfo->adapterName;
        }
        else
        {
            return "Communicator";
        }
    }
    
    const ConnectionInfoPtr&
    getConnectionInfo() const
    {
        return _connectionInfo;
    }

    const EndpointPtr&
    getEndpoint() const
    {
        return _endpoint;
    }

    const EndpointInfoPtr&
    getEndpointInfo() const
    {
        if(!_endpointInfo)
        {
            _endpointInfo = _endpoint->getInfo();
        }
        return _endpointInfo;
    }
    
private:

    const ConnectionInfoPtr& _connectionInfo;
    const EndpointPtr& _endpoint;
    mutable string _id;
    mutable EndpointInfoPtr _endpointInfo;
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
    
    ThreadHelper(const string& parent, const string& id, ThreadState state) : _parent(parent), _id(id), _state(state)
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
    
    const string _parent;
    const string _id;
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
            add("id", &EndpointHelper::getId);
            add("endpoint", &EndpointHelper::getEndpoint);
            addEndpointAttributes<EndpointHelper>(*this);
        }
    };
    static Attributes attributes;
    
    EndpointHelper(const EndpointPtr& endpt, const string& id) : _endpoint(endpt), _id(id)
    {
    }

    EndpointHelper(const EndpointPtr& endpt) : _endpoint(endpt)
    {
    }

    virtual string operator()(const string& attribute) const
    {
        return attributes(this, attribute);
    }

    const EndpointInfoPtr&
    getEndpointInfo() const
    {
        if(!_endpointInfo)
        {
            _endpointInfo = _endpoint->getInfo();
        }
        return _endpointInfo;
    }

    string
    getParent() const
    {
        return "Communicator";
    }

    const string&
    getId() const
    {
        if(_id.empty())
        {
            _id = _endpoint->toString();
        }
        return _id;
    }

    string
    getEndpoint() const
    {
        return _endpoint->toString();
    }

private:
    
    const EndpointPtr _endpoint;
    mutable string _id;
    mutable EndpointInfoPtr _endpointInfo;
};

EndpointHelper::Attributes EndpointHelper::attributes;

}

void 
ConnectionObserverI::sentBytes(Int num)
{
    forEach(add(&ConnectionMetrics::sentBytes, num));
}

void 
ConnectionObserverI::receivedBytes(Int num)
{
    forEach(add(&ConnectionMetrics::receivedBytes, num));
}

void
ThreadObserverI::stateChanged(ThreadState oldState, ThreadState newState)
{
    forEach(ThreadStateChanged(oldState, newState));
}

void
InvocationObserverI::retried()
{
    forEach(inc(&InvocationMetrics::retry));
}

ObserverPtr
InvocationObserverI::getRemoteObserver(const ConnectionInfoPtr& connection, const EndpointPtr& endpoint)
{
    try
    {
        return getObserver<ObserverI>("Remote", RemoteInvocationHelper(connection, endpoint));
    }
    catch(const exception&)
    {
    }
    return 0;
}

CommunicatorObserverI::CommunicatorObserverI(const IceInternal::MetricsAdminIPtr& metrics) : 
    _metrics(metrics),
    _connections(metrics, "Connection"),
    _dispatch(metrics, "Dispatch"),
    _invocations(metrics, "Invocation"),
    _threads(metrics, "Thread"),
    _connects(metrics, "ConnectionEstablishment"),
    _endpointLookups(metrics, "EndpointLookup")
{
    _invocations.registerSubMap<Metrics>("Remote", &InvocationMetrics::remotes);
}

void
CommunicatorObserverI::setObserverUpdater(const ObserverUpdaterPtr& updater)
{
    _connections.setUpdater(newUpdater(updater, &ObserverUpdater::updateConnectionObservers));
    _threads.setUpdater(newUpdater(updater, &ObserverUpdater::updateThreadObservers));
}

ObserverPtr
CommunicatorObserverI::getConnectionEstablishmentObserver(const EndpointPtr& endpt, const string& connector)
{
    if(_connects.isEnabled())
    {
        try
        {
            return _connects.getObserver(EndpointHelper(endpt, connector));
        }
        catch(const exception& ex)
        {
            Error error(_metrics->getLogger());
            error << "unexpected exception trying to obtain observer:\n" << ex;
        }
    }
    return 0;
}

ObserverPtr
CommunicatorObserverI::getEndpointLookupObserver(const EndpointPtr& endpt)
{
    if(_endpointLookups.isEnabled())
    {
        try
        {
            return _endpointLookups.getObserver(EndpointHelper(endpt));
        }
        catch(const exception& ex)
        {
            Error error(_metrics->getLogger());
            error << "unexpected exception trying to obtain observer:\n" << ex;
        }
    }
    return 0;
}

ConnectionObserverPtr 
CommunicatorObserverI::getConnectionObserver(const ConnectionInfoPtr& con, 
                                             const EndpointPtr& endpt,
                                             ConnectionState state, 
                                             const ConnectionObserverPtr& observer)
{
    if(_connections.isEnabled())
    {
        try
        {
            return _connections.getObserver(ConnectionHelper(con, endpt, state), observer);
        }
        catch(const exception& ex)
        {
            Error error(_metrics->getLogger());
            error << "unexpected exception trying to obtain observer:\n" << ex;
        }
    }
    return 0;
}

ThreadObserverPtr 
CommunicatorObserverI::getThreadObserver(const string& parent, 
                                         const string& id,
                                         ThreadState state,
                                         const ThreadObserverPtr& observer)
{
    if(_threads.isEnabled())
    {
        try
        {
            return _threads.getObserver(ThreadHelper(parent, id, state), observer);
        }
        catch(const exception& ex)
        {
            Error error(_metrics->getLogger());
            error << "unexpected exception trying to obtain observer:\n" << ex;
        }
    }
    return 0;
}

InvocationObserverPtr 
CommunicatorObserverI::getInvocationObserver(const ObjectPrx& proxy, const string& op, const Context& ctx)
{
    if(_invocations.isEnabled())
    {
        try
        {
            return _invocations.getObserver(InvocationHelper(proxy, op, ctx));
        }
        catch(const exception& ex)
        {
            Error error(_metrics->getLogger());
            error << "unexpected exception trying to obtain observer:\n" << ex;
        }
    }
    return 0;
}

ObserverPtr 
CommunicatorObserverI::getDispatchObserver(const Current& current)
{
    if(_dispatch.isEnabled())
    {
        try
        {
            return _dispatch.getObserver(DispatchHelper(current));
        }
        catch(const exception& ex)
        {
            Error error(_metrics->getLogger());
            error << "unexpected exception trying to obtain observer:\n" << ex;
        }
    }
    return 0;
}

const IceInternal::MetricsAdminIPtr& 
CommunicatorObserverI::getMetricsAdmin() const
{
    return _metrics;
}
