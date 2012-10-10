// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_INSTRUMENTATION_I_H
#define ICE_INSTRUMENTATION_I_H

#include <Ice/MetricsObserverI.h>

namespace IceMX
{

class ConnectionObserverI : public Ice::Instrumentation::ConnectionObserver, public ObserverT<ConnectionMetrics>
{
public:

    virtual void sentBytes(Ice::Int);
    virtual void receivedBytes(Ice::Int);
};

class ThreadObserverI : public Ice::Instrumentation::ThreadObserver, public ObserverT<ThreadMetrics>
{
public:

    virtual void stateChanged(Ice::Instrumentation::ThreadState, Ice::Instrumentation::ThreadState);
};

class DispatchObserverI : public Ice::Instrumentation::DispatchObserver, public ObserverT<DispatchMetrics>
{
public:

    virtual void userException();
};

class InvocationObserverI : public Ice::Instrumentation::InvocationObserver, public ObserverT<InvocationMetrics>
{
public:

    virtual void retried();

    virtual void userException();

    virtual Ice::Instrumentation::ObserverPtr getRemoteObserver(const Ice::ConnectionInfoPtr&, const Ice::EndpointPtr&);
};

class ICE_API CommunicatorObserverI : public Ice::Instrumentation::CommunicatorObserver
{
public:

    CommunicatorObserverI(const IceInternal::MetricsAdminIPtr&);

    virtual void setObserverUpdater(const Ice::Instrumentation::ObserverUpdaterPtr&);
 
    virtual Ice::Instrumentation::ObserverPtr getConnectionEstablishmentObserver(const Ice::EndpointPtr&,
                                                                                 const std::string&);
 
    virtual Ice::Instrumentation::ObserverPtr getEndpointLookupObserver(const Ice::EndpointPtr&);
    
    virtual Ice::Instrumentation::ConnectionObserverPtr 
    getConnectionObserver(const Ice::ConnectionInfoPtr&, 
                          const Ice::EndpointPtr&,
                          Ice::Instrumentation::ConnectionState, 
                          const Ice::Instrumentation::ConnectionObserverPtr&);

    virtual Ice::Instrumentation::ThreadObserverPtr getThreadObserver(const std::string&, const std::string&,
                                                                      Ice::Instrumentation::ThreadState,
                                                                      const Ice::Instrumentation::ThreadObserverPtr&);

    virtual Ice::Instrumentation::InvocationObserverPtr getInvocationObserver(const Ice::ObjectPrx&, 
                                                                              const std::string&,
                                                                              const Ice::Context&);

    virtual Ice::Instrumentation::DispatchObserverPtr getDispatchObserver(const Ice::Current&);

    const IceInternal::MetricsAdminIPtr& getMetricsAdmin() const;

private:

    const IceInternal::MetricsAdminIPtr _metrics;

    ObserverFactoryT<ConnectionObserverI> _connections;
    ObserverFactoryT<DispatchObserverI> _dispatch;
    ObserverFactoryT<InvocationObserverI> _invocations;
    ObserverFactoryT<ThreadObserverI> _threads;
    ObserverFactoryT<ObserverI> _connects;
    ObserverFactoryT<ObserverI> _endpointLookups;
};
typedef IceUtil::Handle<CommunicatorObserverI> CommunicatorObserverIPtr;

};

#endif
