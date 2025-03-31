// Copyright (c) ZeroC, Inc.

#ifndef ICE_INSTRUMENTATION_I_H
#define ICE_INSTRUMENTATION_I_H

#include "Ice/Connection.h"
#include "MetricsObserverI.h"

namespace IceInternal
{
    template<typename T, typename O> class ObserverWithDelegateT : public IceMX::ObserverT<T>, public virtual O
    {
    public:
        using ObserverType = O;
        using ObserverPtrType = typename std::shared_ptr<O>;
        void attach() override
        {
            IceMX::ObserverT<T>::attach();
            if (_delegate)
            {
                _delegate->attach();
            }
        }

        void detach() override
        {
            IceMX::ObserverT<T>::detach();
            if (_delegate)
            {
                _delegate->detach();
            }
        }

        void failed(const std::string& exceptionName) override
        {
            IceMX::ObserverT<T>::failed(exceptionName);
            if (_delegate)
            {
                _delegate->failed(exceptionName);
            }
        }

        [[nodiscard]] ObserverPtrType getDelegate() const { return _delegate; }

        void setDelegate(ObserverPtrType delegate) { _delegate = std::move(delegate); }

        template<typename ObserverImpl, typename ObserverMetricsType, typename ObserverPtrType>
        ObserverPtrType getObserverWithDelegate(
            const std::string& mapName,
            const IceMX::MetricsHelperT<ObserverMetricsType>& helper,
            const ObserverPtrType& del)
        {
            std::shared_ptr<ObserverImpl> obsv =
                IceMX::ObserverT<T>::template getObserver<ObserverImpl>(mapName, helper);
            if (obsv)
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
        ObserverFactoryWithDelegateT(const IceInternal::MetricsAdminIPtr& metrics, const std::string& name)
            : IceMX::ObserverFactoryT<T>(metrics, name)
        {
        }

        template<typename ObserverMetricsType, typename ObserverPtrType>
        ObserverPtrType
        getObserverWithDelegate(const IceMX::MetricsHelperT<ObserverMetricsType>& helper, const ObserverPtrType& del)
        {
            std::shared_ptr<T> obsv = IceMX::ObserverFactoryT<T>::getObserver(helper);
            if (obsv)
            {
                obsv->setDelegate(del);
                return obsv;
            }
            return del;
        }

        template<typename ObserverMetricsType, typename ObserverPtrType>
        ObserverPtrType getObserverWithDelegate(
            const IceMX::MetricsHelperT<ObserverMetricsType>& helper,
            const ObserverPtrType& del,
            const ObserverPtrType& old)
        {
            std::shared_ptr<T> obsv = IceMX::ObserverFactoryT<T>::getObserver(helper, old);
            if (obsv)
            {
                obsv->setDelegate(del);
                return obsv;
            }
            return del;
        }
    };

    template<typename Helper> void addEndpointAttributes(typename Helper::Attributes& attrs)
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

    template<typename Helper> void addConnectionAttributes(typename Helper::Attributes& attrs)
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

    class ConnectionObserverI
        : public ObserverWithDelegateT<IceMX::ConnectionMetrics, Ice::Instrumentation::ConnectionObserver>
    {
    public:
        void sentBytes(std::int32_t) override;
        void receivedBytes(std::int32_t) override;
    };

    class ThreadObserverI : public ObserverWithDelegateT<IceMX::ThreadMetrics, Ice::Instrumentation::ThreadObserver>
    {
    public:
        void stateChanged(Ice::Instrumentation::ThreadState, Ice::Instrumentation::ThreadState) override;
    };

    class DispatchObserverI
        : public ObserverWithDelegateT<IceMX::DispatchMetrics, Ice::Instrumentation::DispatchObserver>
    {
    public:
        void userException() override;

        void reply(std::int32_t) override;
    };

    class RemoteObserverI : public ObserverWithDelegateT<IceMX::RemoteMetrics, Ice::Instrumentation::RemoteObserver>
    {
    public:
        void reply(std::int32_t) override;
    };

    class CollocatedObserverI
        : public ObserverWithDelegateT<IceMX::CollocatedMetrics, Ice::Instrumentation::CollocatedObserver>
    {
    public:
        void reply(std::int32_t) override;
    };

    class InvocationObserverI
        : public ObserverWithDelegateT<IceMX::InvocationMetrics, Ice::Instrumentation::InvocationObserver>
    {
    public:
        void retried() override;

        void userException() override;

        Ice::Instrumentation::RemoteObserverPtr
        getRemoteObserver(const Ice::ConnectionInfoPtr&, const Ice::EndpointPtr&, std::int32_t, std::int32_t) override;

        Ice::Instrumentation::CollocatedObserverPtr
        getCollocatedObserver(const Ice::ObjectAdapterPtr&, std::int32_t, std::int32_t) override;
    };

    using ObserverI = ObserverWithDelegateT<IceMX::Metrics, Ice::Instrumentation::Observer>;

    class ICE_API CommunicatorObserverI : public Ice::Instrumentation::CommunicatorObserver
    {
    public:
        CommunicatorObserverI(const Ice::InitializationData&);

        void setObserverUpdater(const Ice::Instrumentation::ObserverUpdaterPtr&) override;

        Ice::Instrumentation::ObserverPtr
        getConnectionEstablishmentObserver(const Ice::EndpointPtr&, const std::string&) override;

        Ice::Instrumentation::ObserverPtr getEndpointLookupObserver(const Ice::EndpointPtr&) override;

        Ice::Instrumentation::ConnectionObserverPtr getConnectionObserver(
            const Ice::ConnectionInfoPtr&,
            const Ice::EndpointPtr&,
            Ice::Instrumentation::ConnectionState,
            const Ice::Instrumentation::ConnectionObserverPtr&) override;

        Ice::Instrumentation::ThreadObserverPtr getThreadObserver(
            const std::string&,
            const std::string&,
            Ice::Instrumentation::ThreadState,
            const Ice::Instrumentation::ThreadObserverPtr&) override;

        Ice::Instrumentation::InvocationObserverPtr
        getInvocationObserver(const std::optional<Ice::ObjectPrx>&, std::string_view, const Ice::Context&) override;

        Ice::Instrumentation::DispatchObserverPtr getDispatchObserver(const Ice::Current&, std::int32_t) override;

        [[nodiscard]] const IceInternal::MetricsAdminIPtr& getFacet() const;

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
    using CommunicatorObserverIPtr = std::shared_ptr<CommunicatorObserverI>;

};

#endif
