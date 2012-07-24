// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/MetricsObserverI.h>
#include <Ice/MetricsAdminI.h>

#include <Ice/Connection.h>
#include <Ice/Endpoint.h>

using namespace std;
using namespace Ice;
using namespace IceMX;
using namespace IceMX::Ice;

namespace 
{


class ConnectionHelper : public ObjectHelperT<ConnectionMetricsObject>
{
public:
    
    ConnectionHelper(const ConnectionPtr& con) : _connection(con)
    {
    }

    virtual string operator()(const string&) const;

private:

    friend class ConnectionAttributeResolver;

    ::Ice::ConnectionInfo*
    getConnectionInfo() const
    {
        return _connection->getInfo().get();
    }

    ::Ice::EndpointInfo*
    getEndpointInfo() const
    {
        return _connection->getEndpoint()->getInfo().get();
    }
    
    ::Ice::ConnectionPtr _connection;
};


class ConnectionAttributeResolver : public ObjectAttributeResolverT<ConnectionHelper>
{
public:

    ConnectionAttributeResolver()
    {
        //add("connectionId")

        add("incoming", &ConnectionHelper::getConnectionInfo, &ConnectionInfo::incoming);
        add("adapterName", &ConnectionHelper::getConnectionInfo, &ConnectionInfo::adapterName);

        add("localHost", &ConnectionHelper::getConnectionInfo, &IPConnectionInfo::localAddress);
        add("localPort", &ConnectionHelper::getConnectionInfo, &IPConnectionInfo::localPort);
        add("remoteHost", &ConnectionHelper::getConnectionInfo, &IPConnectionInfo::remoteAddress);
        add("remotePort", &ConnectionHelper::getConnectionInfo, &IPConnectionInfo::remotePort);

        add("mcastHost", &ConnectionHelper::getConnectionInfo, &UDPConnectionInfo::mcastAddress);
        add("mcastPort", &ConnectionHelper::getConnectionInfo, &UDPConnectionInfo::mcastPort);

        add("endpointType", &ConnectionHelper::getEndpointInfo, &EndpointInfo::type);
        add("endpointIsDatagram", &ConnectionHelper::getEndpointInfo, &EndpointInfo::datagram);
        add("endpointIsSecure", &ConnectionHelper::getEndpointInfo, &EndpointInfo::secure);
        add("endpointProtocolVersion", &ConnectionHelper::getEndpointInfo, &EndpointInfo::protocol);
        add("endpointEncodingVersion", &ConnectionHelper::getEndpointInfo, &EndpointInfo::encoding);
        add("endpointTimeout", &ConnectionHelper::getEndpointInfo, &EndpointInfo::timeout);
        add("endpointCompress", &ConnectionHelper::getEndpointInfo, &EndpointInfo::compress);

        add("endpointHost", &ConnectionHelper::getEndpointInfo, &IPEndpointInfo::host);
        add("endpointPort", &ConnectionHelper::getEndpointInfo, &IPEndpointInfo::port);
    }
};
ConnectionAttributeResolver connectionAttributes;

string
ConnectionHelper::operator()(const string& name) const
{
    return connectionAttributes(this, name);
}

}

void
ConnectionObserverI::attach()
{
    struct Attach 
    {
        void operator()(const ConnectionMetricsObjectPtr& v)
        {
            ++v->total;
            ++v->current;
            ++v->initializing;
        }
    };
    forEach(Attach());
}

void
ConnectionObserverI::detach()
{
    struct Detach 
    {
        void operator()(const ConnectionMetricsObjectPtr& v)
        {
            --v->current;
            --v->closed;
        }
    };
    forEach(Detach());
}

void
ConnectionObserverI::stateChanged(ObserverConnectionState oldState, ObserverConnectionState newState)
{
    struct StateChanged 
    {
        StateChanged(ObserverConnectionState oldState, ObserverConnectionState newState) : 
            oldState(oldState), newState(newState)
        {
        }

        void operator()(const ConnectionMetricsObjectPtr& v)
        {
            --(v.get()->*getObserverConnectionStateMetric(oldState));
            ++(v.get()->*getObserverConnectionStateMetric(newState));
        }

        int ConnectionMetricsObject::*
        getObserverConnectionStateMetric(ObserverConnectionState s)
        {
            switch(s)
            {
            case ObserverConnectionStateInitializing:
                return &ConnectionMetricsObject::initializing;
            case ObserverConnectionStateActive:
                return &ConnectionMetricsObject::active;
            case ObserverConnectionStateHolding:
                return &ConnectionMetricsObject::holding;
            case ObserverConnectionStateClosing:
                return &ConnectionMetricsObject::closing;
            case ObserverConnectionStateClosed:
                return &ConnectionMetricsObject::closed;
            }
        }    

        ObserverConnectionState oldState;
        ObserverConnectionState newState;
    }
    forEach(StateChanged(oldState, newState));
}

void 
ConnectionObserverI::sentBytes(Int num, Long duration)
{
    forEach(chain(add(&ConnectionMetricsObject::sentBytes, num), 
                  add(&ConnectionMetricsObject::sentTime, duration)));
}

void 
ConnectionObserverI::receivedBytes(Int num, Long duration)
{
    forEach(chain(add(&ConnectionMetricsObject::receivedBytes, num),
                  add(&ConnectionMetricsObject::receivedTime, duration)));
}

ObserverResolverI::ObserverResolverI(const MetricsAdminIPtr& metrics) : _metrics(metrics)
{
}

void
ObserverResolverI::setObserverUpdater(const ObserverUpdaterPtr& updater)
{

    _metrics->addUpdater("Connection", newUpdater(updater, &ObserverUpdater::updateConnectionObservers, _connections));
    //_metrics->addUpdater("Thread", new Updater(updater, &ObserverUpdater::updateThreadObservers));
    //_metrics->addUpdater("ThreadPoolThread", new Updater(updater, &ObserverUpdater::updateThreadPoolThreadObservers));
}

ConnectionObserverPtr 
ObserverResolverI::getConnectionObserver(const ConnectionPtr& con, const ConnectionObserverPtr& old)
{
    return _connections.getObserver(_metrics->getMatching("Connection", ConnectionHelper(con)), old.get());
}

ObjectObserverPtr 
ObserverResolverI::getThreadObserver(const string&, const string&, const ObjectObserverPtr&)
{
    return 0;
}

ThreadPoolThreadObserverPtr 
ObserverResolverI::getThreadPoolThreadObserver(const string&, const string&, const ThreadPoolThreadObserverPtr&)
{
    return 0;
}

RequestObserverPtr 
ObserverResolverI::getInvocationObserver(const ObjectPrx&, const string&)
{
    return 0;
}

RequestObserverPtr 
ObserverResolverI::getDispatchObserver(const ObjectPtr&, const Current&)
{
    return 0;
}
