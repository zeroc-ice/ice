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
#include <Ice/Connection.h>

namespace IceInternal
{

template<typename Helper>
void addEndpointAttributes(typename Helper::Attributes& attrs)
{
    attrs.add("endpoint", &Helper::getEndpoint);

    attrs.add("endpointType", &Helper::getEndpointInfo, &Ice::EndpointInfo::type);
    attrs.add("endpointIsDatagram", &Helper::getEndpointInfo, &Ice::EndpointInfo::datagram);
    attrs.add("endpointIsSecure", &Helper::getEndpointInfo, &Ice::EndpointInfo::secure);
    attrs.add("endpointTimeout", &Helper::getEndpointInfo, &Ice::EndpointInfo::timeout);
    attrs.add("endpointCompress", &Helper::getEndpointInfo, &Ice::EndpointInfo::compress);
    
    attrs.add("endpointHost", &Helper::getEndpointInfo, &Ice::IPEndpointInfo::host);
    attrs.add("endpointPort", &Helper::getEndpointInfo, &Ice::IPEndpointInfo::port);
}

template<typename Helper>
void addConnectionAttributes(typename Helper::Attributes& attrs)
{
    attrs.add("incoming", &Helper::getConnectionInfo, &Ice::ConnectionInfo::incoming);
    attrs.add("adapterName", &Helper::getConnectionInfo, &Ice::ConnectionInfo::adapterName);
    attrs.add("connectionId", &Helper::getConnectionInfo, &Ice::ConnectionInfo::connectionId);
    
    attrs.add("localHost", &Helper::getConnectionInfo, &Ice::IPConnectionInfo::localAddress);
    attrs.add("localPort", &Helper::getConnectionInfo, &Ice::IPConnectionInfo::localPort);
    attrs.add("remoteHost", &Helper::getConnectionInfo, &Ice::IPConnectionInfo::remoteAddress);
    attrs.add("remotePort", &Helper::getConnectionInfo, &Ice::IPConnectionInfo::remotePort);
            
    attrs.add("mcastHost", &Helper::getConnectionInfo, &Ice::UDPConnectionInfo::mcastAddress);
    attrs.add("mcastPort", &Helper::getConnectionInfo, &Ice::UDPConnectionInfo::mcastPort);
    
    addEndpointAttributes<Helper>(attrs);
}

class ConnectionObserverI : public Ice::Instrumentation::ConnectionObserver, 
                            public IceMX::ObserverT<IceMX::ConnectionMetrics>
{
public:

    virtual void sentBytes(Ice::Int);
    virtual void receivedBytes(Ice::Int);
};

class ThreadObserverI : public Ice::Instrumentation::ThreadObserver, 
                        public IceMX::ObserverT<IceMX::ThreadMetrics>
{
public:

    virtual void stateChanged(Ice::Instrumentation::ThreadState, Ice::Instrumentation::ThreadState);
};

class DispatchObserverI : public Ice::Instrumentation::DispatchObserver, 
                          public IceMX::ObserverT<IceMX::DispatchMetrics>
{
public:

    virtual void userException();

    virtual void reply(Ice::Int);
};

class RemoteObserverI : public Ice::Instrumentation::RemoteObserver, 
                        public IceMX::ObserverT<IceMX::RemoteMetrics>
{
    virtual void reply(Ice::Int);
};

class InvocationObserverI : public Ice::Instrumentation::InvocationObserver, 
                            public IceMX::ObserverT<IceMX::InvocationMetrics>
{
public:

    virtual void retried();

    virtual void userException();

    virtual Ice::Instrumentation::RemoteObserverPtr getRemoteObserver(const Ice::ConnectionInfoPtr&, 
                                                                      const Ice::EndpointPtr&, Ice::Int, Ice::Int);
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

    virtual Ice::Instrumentation::DispatchObserverPtr getDispatchObserver(const Ice::Current&, Ice::Int);

    const IceInternal::MetricsAdminIPtr& getMetricsAdmin() const;

private:

    const IceInternal::MetricsAdminIPtr _metrics;

    IceMX::ObserverFactoryT<ConnectionObserverI> _connections;
    IceMX::ObserverFactoryT<DispatchObserverI> _dispatch;
    IceMX::ObserverFactoryT<InvocationObserverI> _invocations;
    IceMX::ObserverFactoryT<ThreadObserverI> _threads;
    IceMX::ObserverFactoryT<IceMX::ObserverI> _connects;
    IceMX::ObserverFactoryT<IceMX::ObserverI> _endpointLookups;
};
typedef IceUtil::Handle<CommunicatorObserverI> CommunicatorObserverIPtr;

};

#endif
