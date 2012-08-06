// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_OBSERVER_I_H
#define ICE_OBSERVER_I_H

#include <Ice/MetricsObserverI.h>

namespace IceMX
{

class ConnectionObserverI : public Ice::Instrumentation::ConnectionObserver, public ObserverT<ConnectionMetrics>
{
public:

    virtual void detach();

    virtual void stateChanged(Ice::Instrumentation::ConnectionState, Ice::Instrumentation::ConnectionState);
    virtual void sentBytes(Ice::Int, Ice::Long);
    virtual void receivedBytes(Ice::Int, Ice::Long);
};

class ThreadObserverI : public Ice::Instrumentation::ThreadObserver, public ObserverT<ThreadMetrics>
{
    virtual void stateChanged(Ice::Instrumentation::ThreadState, Ice::Instrumentation::ThreadState);
};

class ObserverResolverI : public Ice::Instrumentation::ObserverResolver
{
public:

    ObserverResolverI(const MetricsAdminIPtr&);

    virtual void setObserverUpdater(const Ice::Instrumentation::ObserverUpdaterPtr&);

    virtual Ice::Instrumentation::ObserverPtr getLocatorQueryObserver(const std::string&);
 
    virtual Ice::Instrumentation::ObserverPtr getConnectObserver(const Ice::EndpointInfoPtr&, 
                                                                 const std::string&);
 
    virtual Ice::Instrumentation::ObserverPtr getEndpointResolveObserver(const Ice::EndpointInfoPtr&, 
                                                                         const std::string&);
 
    virtual Ice::Instrumentation::ConnectionObserverPtr 
    getConnectionObserver(const Ice::ConnectionInfoPtr&, 
                          const Ice::EndpointInfoPtr&,
                          Ice::Instrumentation::ConnectionState, 
                          const Ice::Instrumentation::ConnectionObserverPtr&);

    virtual Ice::Instrumentation::ThreadObserverPtr getThreadObserver(const std::string&, const std::string&,
                                                                      Ice::Instrumentation::ThreadState,
                                                                      const Ice::Instrumentation::ThreadObserverPtr&);

    virtual Ice::Instrumentation::ObserverPtr getInvocationObserver(const Ice::ObjectPrx&, 
                                                                    const std::string&, 
                                                                    const Ice::Context&, 
                                                                    const Ice::ConnectionPtr&);

    virtual Ice::Instrumentation::ObserverPtr getDispatchObserver(const Ice::Current&);

private:

    const MetricsAdminIPtr _metrics;

    ObserverResolverT<ConnectionObserverI> _connections;
    ObserverResolverT<ObserverI> _requests;
    ObserverResolverT<ThreadObserverI> _threads;
    ObserverResolverT<ObserverI> _locatorQueries;
    ObserverResolverT<ObserverI> _connects;
    ObserverResolverT<ObserverI>  _endpointResolves;

};

};

#endif
