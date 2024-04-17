//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_OBJECT_ADAPTER_I_H
#define ICE_OBJECT_ADAPTER_I_H

#include "ACM.h"
#include "ConnectionFactoryF.h"
#include "EndpointIF.h"
#include "Ice/BuiltinSequences.h"
#include "Ice/CommunicatorF.h"
#include "Ice/Exception.h"
#include "Ice/InstanceF.h"
#include "Ice/ObjectAdapter.h"
#include "Ice/ObjectF.h"
#include "Ice/Proxy.h"
#include "LocatorInfoF.h"
#include "ObjectAdapterFactoryF.h"
#include "RouterInfoF.h"
#include "ServantManagerF.h"
#include "ThreadPoolF.h"

#ifdef ICE_SWIFT
#    include <dispatch/dispatch.h>
#endif

#include <list>
#include <mutex>

namespace IceInternal
{
    class CommunicatorFlushBatchAsync;
    using CommunicatorFlushBatchAsyncPtr = std::shared_ptr<CommunicatorFlushBatchAsync>;
}

namespace Ice
{
    class ObjectAdapterI final : public ObjectAdapter, public std::enable_shared_from_this<ObjectAdapterI>
    {
    public:
        std::string getName() const noexcept final;

        CommunicatorPtr getCommunicator() const noexcept final;

        void activate() final;
        void hold() final;
        void waitForHold() final;
        void deactivate() noexcept;
        void waitForDeactivate() noexcept final;
        bool isDeactivated() const noexcept final;
        void destroy() noexcept final;

        ObjectPrx add(const ObjectPtr&, const Identity&) final;
        ObjectPrx addFacet(const ObjectPtr&, const Identity&, const std::string&) final;
        ObjectPrx addWithUUID(const ObjectPtr&) final;
        ObjectPrx addFacetWithUUID(const ObjectPtr&, const std::string&) final;
        void addDefaultServant(const ObjectPtr&, const std::string&) final;
        ObjectPtr remove(const Identity&) final;
        ObjectPtr removeFacet(const Identity&, const std::string&) final;
        FacetMap removeAllFacets(const Identity&) final;
        ObjectPtr removeDefaultServant(const std::string&) final;
        ObjectPtr find(const Identity&) const final;
        ObjectPtr findFacet(const Identity&, const std::string&) const final;
        FacetMap findAllFacets(const Identity&) const final;
        ObjectPtr findByProxy(const ObjectPrx&) const final;
        ObjectPtr findDefaultServant(const std::string&) const final;
        void addServantLocator(const ServantLocatorPtr&, const std::string&) final;
        ServantLocatorPtr removeServantLocator(const std::string&) final;
        ServantLocatorPtr findServantLocator(const std::string&) const final;
        ObjectPtr dispatcher() const noexcept final;

        ObjectPrx createProxy(const Identity&) const final;
        ObjectPrx createDirectProxy(const Identity&) const final;
        ObjectPrx createIndirectProxy(const Identity&) const final;

        void setLocator(const std::optional<LocatorPrx>&) final;
        std::optional<LocatorPrx> getLocator() const noexcept;
        EndpointSeq getEndpoints() const noexcept;

        void refreshPublishedEndpoints() final;
        EndpointSeq getPublishedEndpoints() const noexcept;
        void setPublishedEndpoints(const EndpointSeq&) final;

#ifdef ICE_SWIFT
        dispatch_queue_t getDispatchQueue() const final;
#endif

        bool isLocal(const IceInternal::ReferencePtr&) const;

        void flushAsyncBatchRequests(const IceInternal::CommunicatorFlushBatchAsyncPtr&, CompressBatch);

        void updateConnectionObservers();
        void updateThreadObservers();

        void incDirectCount();
        void decDirectCount();

        IceInternal::ThreadPoolPtr getThreadPool() const;
        IceInternal::ACMConfig getACM() const;
        void setAdapterOnConnection(const Ice::ConnectionIPtr&);
        size_t messageSizeMax() const { return _messageSizeMax; }

        // The dispatch pipeline is the dispatcher plus the logger and observer middleware. They are installed in the
        // dispatch pipeline only when the communicator configuration enables them.
        const Ice::ObjectPtr& dispatchPipeline() const noexcept { return _dispatchPipeline; }

        ObjectAdapterI(
            const IceInternal::InstancePtr&,
            const CommunicatorPtr&,
            const IceInternal::ObjectAdapterFactoryPtr&,
            const std::string&,
            bool);
        virtual ~ObjectAdapterI();

    private:
        void initialize(std::optional<RouterPrx>);
        friend class IceInternal::ObjectAdapterFactory;

        ObjectPrx newProxy(const Identity&, const std::string&) const;
        ObjectPrx newDirectProxy(const Identity&, const std::string&) const;
        ObjectPrx newIndirectProxy(const Identity&, const std::string&, const std::string&) const;
        void checkForDeactivation() const;
        std::vector<IceInternal::EndpointIPtr> parseEndpoints(const std::string&, bool) const;
        std::vector<IceInternal::EndpointIPtr> computePublishedEndpoints();
        void updateLocatorRegistry(const IceInternal::LocatorInfoPtr&, const std::optional<Ice::ObjectPrx>&);
        bool filterProperties(Ice::StringSeq&);

        enum State
        {
            StateUninitialized,
            StateHeld,
            StateActivating,
            StateActive,
            StateDeactivating,
            StateDeactivated,
            StateDestroying,
            StateDestroyed
        };
        State _state;
        IceInternal::InstancePtr _instance;
        CommunicatorPtr _communicator;
        IceInternal::ObjectAdapterFactoryPtr _objectAdapterFactory;
        IceInternal::ThreadPoolPtr _threadPool;
        IceInternal::ACMConfig _acm;
        const IceInternal::ServantManagerPtr _servantManager;

        // There is no need to clear _dispatchPipeline during destroy because _dispatchPipeline does not hold onto this
        // object adapter directly. It can hold onto a communicator that holds onto this object adapter, but the
        // communicator will release this refcount when it is destroyed or when the object adapter is destroyed.
        const ObjectPtr _dispatchPipeline; // must be declared after _servantManager
        const std::string _name;
        const std::string _id;
        const std::string _replicaGroupId;
        IceInternal::ReferencePtr _reference;
        std::vector<IceInternal::IncomingConnectionFactoryPtr> _incomingConnectionFactories;
        IceInternal::RouterInfoPtr _routerInfo;
        std::vector<IceInternal::EndpointIPtr> _publishedEndpoints;
        IceInternal::LocatorInfoPtr _locatorInfo;
        int _directCount; // The number of direct proxies dispatching on this object adapter.
        bool _noConfig;
        size_t _messageSizeMax;
        mutable std::recursive_mutex _mutex;
        std::condition_variable_any _conditionVariable;

#if defined(_WIN32)
        CredHandle _sslServerContext;
        std::function<bool(CtxtHandle context)> _sslClientCertificateValidationCallback;
#elif defined(__APPLE__)
#else
#endif
    };
}

#endif
