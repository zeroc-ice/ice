// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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

template<typename T, typename O> class ObserverWithDelegateT : public IceMX::ObserverT<T>, virtual public O
{
public:

    typedef O ObserverType;
    typedef typename IceInternal::Handle<O> ObserverPtrType;

    virtual void 
    attach()
    {
        IceMX::ObserverT<T>::attach();
        if(_delegate)
        {
            _delegate->attach();
        }
    }

    virtual void 
    detach()
    {
        IceMX::ObserverT<T>::detach();
        if(_delegate)
        {
            _delegate->detach();
        }
    }

    virtual void
    failed(const std::string& exceptionName)
    {
        IceMX::ObserverT<T>::failed(exceptionName);
        if(_delegate)
        {
            _delegate->failed(exceptionName);
        }
    }

    ObserverPtrType
    getDelegate() const
    {
        return _delegate;
    }

    void
    setDelegate(ObserverPtrType delegate)
    {
        _delegate = delegate;
    }

    template<typename ObserverImpl, typename ObserverMetricsType, typename ObserverPtrType> ObserverPtrType 
    getObserverWithDelegate(const std::string& mapName, const IceMX::MetricsHelperT<ObserverMetricsType>& helper, 
                            const ObserverPtrType& del)
    {
        IceInternal::Handle<ObserverImpl> obsv = IceMX::ObserverT<T>::template getObserver<ObserverImpl>(mapName, 
                                                                                                         helper);
        if(obsv)
        {
            obsv->setDelegate(del);
            return obsv;
        }
        return del;
    }

protected:

    ObserverPtrType _delegate;
};

template<typename T> class ObserverFactoryWithDelegateT : public IceMX::ObserverFactoryT<T>
{
public:

    ObserverFactoryWithDelegateT(const IceInternal::MetricsAdminIPtr& metrics, const std::string& name) : 
        IceMX::ObserverFactoryT<T>(metrics, name)
    {
    }

    template<typename ObserverMetricsType, typename ObserverPtrType> ObserverPtrType 
    getObserverWithDelegate(const IceMX::MetricsHelperT<ObserverMetricsType>& helper, const ObserverPtrType& del)
    {
        IceInternal::Handle<T> obsv = IceMX::ObserverFactoryT<T>::getObserver(helper);
        if(obsv)
        {
            obsv->setDelegate(del);
            return obsv;
        }
        return del;
    }

    template<typename ObserverMetricsType, typename ObserverPtrType> ObserverPtrType 
    getObserverWithDelegate(const IceMX::MetricsHelperT<ObserverMetricsType>& helper, const ObserverPtrType& del, 
                            const ObserverPtrType& old)
    {
        IceInternal::Handle<T> obsv = IceMX::ObserverFactoryT<T>::getObserver(helper, old);
        if(obsv)
        {
            obsv->setDelegate(del);
            return obsv;
        }
        return del;
    }
};

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

class ConnectionObserverI : public ObserverWithDelegateT<IceMX::ConnectionMetrics, 
                                                         Ice::Instrumentation::ConnectionObserver>
{
public:

    virtual void sentBytes(Ice::Int);
    virtual void receivedBytes(Ice::Int);
};

class ThreadObserverI : public ObserverWithDelegateT<IceMX::ThreadMetrics, Ice::Instrumentation::ThreadObserver>
{
public:

    virtual void stateChanged(Ice::Instrumentation::ThreadState, Ice::Instrumentation::ThreadState);
};

class DispatchObserverI : public ObserverWithDelegateT<IceMX::DispatchMetrics, Ice::Instrumentation::DispatchObserver>
{
public:

    virtual void userException();

    virtual void reply(Ice::Int);
};

class RemoteObserverI : public ObserverWithDelegateT<IceMX::RemoteMetrics, Ice::Instrumentation::RemoteObserver>
{
public:

    virtual void reply(Ice::Int);
};

class CollocatedObserverI : public ObserverWithDelegateT<IceMX::CollocatedMetrics, 
                                                         Ice::Instrumentation::CollocatedObserver>
{
public:

    virtual void reply(Ice::Int);
};

class InvocationObserverI : public ObserverWithDelegateT<IceMX::InvocationMetrics, 
                                                         Ice::Instrumentation::InvocationObserver>
{
public:

    virtual void retried();

    virtual void userException();

    virtual Ice::Instrumentation::RemoteObserverPtr 
    getRemoteObserver(const Ice::ConnectionInfoPtr&, const Ice::EndpointPtr&, Ice::Int, Ice::Int);

    virtual Ice::Instrumentation::CollocatedObserverPtr
    getCollocatedObserver(const Ice::ObjectAdapterPtr&, Ice::Int, Ice::Int);
};

typedef ObserverWithDelegateT<IceMX::Metrics, Ice::Instrumentation::Observer> ObserverI;

class ICE_API CommunicatorObserverI : public Ice::Instrumentation::CommunicatorObserver
{
public:

    CommunicatorObserverI(const Ice::InitializationData&);

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

    const IceInternal::MetricsAdminIPtr& getFacet() const;

    void destroy();

private:

    IceInternal::MetricsAdminIPtr _metrics;
    const Ice::Instrumentation::CommunicatorObserverPtr _delegate;

    ObserverFactoryWithDelegateT<ConnectionObserverI> _connections;
    ObserverFactoryWithDelegateT<DispatchObserverI> _dispatch;
    ObserverFactoryWithDelegateT<InvocationObserverI> _invocations;
    ObserverFactoryWithDelegateT<ThreadObserverI> _threads;
    ObserverFactoryWithDelegateT<ObserverI> _connects;
    ObserverFactoryWithDelegateT<ObserverI> _endpointLookups;
};
typedef IceUtil::Handle<CommunicatorObserverI> CommunicatorObserverIPtr;

};

#endif
