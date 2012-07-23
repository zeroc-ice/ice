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

using namespace std;
using namespace Ice;
using namespace IceMetrics;

namespace 
{

class ConnectionMetricsHelper : public MetricsHelper
{
public:
    
    ConnectionMetricsHelper(const ConnectionPtr& con) : _connection(con)
    {
    }

    virtual string operator()(const string& attribute) const
    {
        return ""; // TODO: return attribute value
    }
    
    virtual MetricsObjectPtr newMetricsObject() const
    {
        return new ConnectionMetricsObject();
    }
        
private:
    
    Ice::ConnectionPtr _connection;
};

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
            ++v->total;
            ++v->current;
            ++v->initializing;
        }
    };
    forEach(Detach());
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

        void operator()(const ConnectionMetricsObjectPtr& v)
        {
            --(v.get()->*getConnectionStateMetric(oldState));
            ++(v.get()->*getConnectionStateMetric(newState));
        }

        int ConnectionMetricsObject::*
        getConnectionStateMetric(ConnectionState s)
        {
            switch(s)
            {
            case ConnectionStateInitializing:
                return &ConnectionMetricsObject::initializing;
            case ConnectionStateActive:
                return &ConnectionMetricsObject::active;
            case ConnectionStateHolding:
                return &ConnectionMetricsObject::holding;
            case ConnectionStateClosing:
                return &ConnectionMetricsObject::closing;
            case ConnectionStateClosed:
                return &ConnectionMetricsObject::closed;
            }
        }    

        ConnectionState oldState;
        ConnectionState newState;
    }
    forEach(StateChanged(oldState, newState));
}

void 
ConnectionObserverI::sentBytes(Int num, Long duration)
{
    forEach(aggregate(applyOnMember(&ConnectionMetricsObject::sentBytes, Add<Int>(num)),
                      applyOnMember(&ConnectionMetricsObject::sentTime, Add<Long>(duration))));
}

void 
ConnectionObserverI::receivedBytes(Int num, Long duration)
{
    forEach(aggregate(applyOnMember(&ConnectionMetricsObject::receivedBytes, Add<Int>(num)),
                      applyOnMember(&ConnectionMetricsObject::receivedTime, Add<Long>(duration))));
}

ObserverResolverI::ObserverResolverI(const MetricsAdminIPtr& metrics) : _metrics(metrics)
{
}

void
ObserverResolverI::setObserverUpdater(const ObserverUpdaterPtr& updater)
{
    class Updater : public ObjectObserverUpdater
    {
    public:

        Updater(const ObserverUpdaterPtr& updater, void (ObserverUpdater::*fn)()) : 
            _updater(updater), _fn(fn)
        {
        }
        
        virtual void update()
        {
            (_updater.get()->*_fn)();
        }

    private: 

        const ObserverUpdaterPtr _updater;
        void (ObserverUpdater::*_fn)();
    };
    _metrics->addUpdater("Connection", new Updater(updater, &ObserverUpdater::updateConnectionObservers));
    _metrics->addUpdater("Thread", new Updater(updater, &ObserverUpdater::updateThreadObservers));
    _metrics->addUpdater("ThreadPoolThread", new Updater(updater, &ObserverUpdater::updateThreadPoolThreadObservers));
}

ConnectionObserverPtr 
ObserverResolverI::getConnectionObserver(const ConnectionObserverPtr& old, const ConnectionPtr& con)
{
    return _connections.getObserver(_metrics->getMatching("Connection", ConnectionMetricsHelper(con)), old.get());
}

ObjectObserverPtr 
ObserverResolverI::getThreadObserver(const ObjectObserverPtr&, const string&, const string&)
{
    return 0;
}

ThreadPoolThreadObserverPtr 
ObserverResolverI::getThreadPoolThreadObserver(const ThreadPoolThreadObserverPtr&, const string&, const string&)
{
    return 0;
}

RequestObserverPtr 
ObserverResolverI::getInvocationObserver(const RequestObserverPtr&, const ObjectPrx&, const string&)
{
    return 0;
}

RequestObserverPtr 
ObserverResolverI::getDispatchObserver(const RequestObserverPtr&, const ObjectPtr&, const Current&)
{
    return 0;
}
