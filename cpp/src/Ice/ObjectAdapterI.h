//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_OBJECT_ADAPTER_I_H
#define ICE_OBJECT_ADAPTER_I_H

#include "ConnectionFactoryF.h"
#include "ConnectionI.h"
#include "EndpointIF.h"
#include "Ice/BuiltinSequences.h"
#include "Ice/CommunicatorF.h"
#include "Ice/InstanceF.h"
#include "Ice/ObjectAdapter.h"
#include "Ice/ObjectF.h"
#include "Ice/Proxy.h"
#include "Ice/SSL/ServerAuthenticationOptions.h"
#include "LocatorInfoF.h"
#include "ObjectAdapterFactoryF.h"
#include "RouterInfoF.h"
#include "ServantManagerF.h"
#include "ThreadPoolF.h"

#include <list>
#include <mutex>
#include <optional>
#include <stack>

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

        ObjectAdapterPtr use(std::function<ObjectPtr(ObjectPtr)> middlewareFactory) final;

        ObjectPrx _add(ObjectPtr, Identity) final;
        ObjectPrx _addFacet(ObjectPtr, Identity, std::string) final;
        ObjectPrx _addWithUUID(ObjectPtr) final;
        ObjectPrx _addFacetWithUUID(ObjectPtr, std::string) final;
        void addDefaultServant(ObjectPtr, std::string) final;
        ObjectPtr remove(const Identity&) final;
        ObjectPtr removeFacet(const Identity&, std::string_view) final;
        FacetMap removeAllFacets(const Identity&) final;
        ObjectPtr removeDefaultServant(std::string_view) final;
        ObjectPtr find(const Identity&) const final;
        ObjectPtr findFacet(const Identity&, std::string_view) const final;
        FacetMap findAllFacets(const Identity&) const final;
        ObjectPtr findByProxy(const ObjectPrx&) const final;
        ObjectPtr findDefaultServant(std::string_view) const final;
        void addServantLocator(ServantLocatorPtr, std::string) final;
        ServantLocatorPtr removeServantLocator(std::string_view) final;
        ServantLocatorPtr findServantLocator(std::string_view) const final;

        const ObjectPtr& dispatchPipeline() const noexcept final;

        ObjectPrx _createProxy(Identity) const final;
        ObjectPrx _createDirectProxy(Identity) const final;
        ObjectPrx _createIndirectProxy(Identity) const final;

        void setLocator(std::optional<LocatorPrx>) final;
        std::optional<LocatorPrx> getLocator() const noexcept;
        EndpointSeq getEndpoints() const noexcept;

        EndpointSeq getPublishedEndpoints() const noexcept;
        void setPublishedEndpoints(EndpointSeq) final;

        bool isLocal(const IceInternal::ReferencePtr&) const;

        void flushAsyncBatchRequests(const IceInternal::CommunicatorFlushBatchAsyncPtr&, CompressBatch);

        void updateConnectionObservers();
        void updateThreadObservers();

        void incDirectCount();
        void decDirectCount();

        IceInternal::ThreadPoolPtr getThreadPool() const;
        void setAdapterOnConnection(const ConnectionIPtr&);
        size_t messageSizeMax() const { return _messageSizeMax; }

        ObjectAdapterI(
            IceInternal::InstancePtr,
            CommunicatorPtr,
            IceInternal::ObjectAdapterFactoryPtr,
            std::string name,
            bool,
            std::optional<SSL::ServerAuthenticationOptions>);
        virtual ~ObjectAdapterI();

        const std::optional<SSL::ServerAuthenticationOptions>& serverAuthenticationOptions() const noexcept
        {
            return _serverAuthenticationOptions;
        }

    private:
        void initialize(std::optional<RouterPrx>);
        friend class IceInternal::ObjectAdapterFactory;

        ObjectPrx newProxy(Identity, std::string) const;
        ObjectPrx newDirectProxy(Identity, std::string) const;
        ObjectPrx newIndirectProxy(Identity, std::string, std::string) const;
        void checkForDeactivation() const;
        void checkForDestruction() const;
        std::vector<IceInternal::EndpointIPtr> parseEndpoints(std::string_view, bool) const;
        std::vector<IceInternal::EndpointIPtr> computePublishedEndpoints();
        void updateLocatorRegistry(const IceInternal::LocatorInfoPtr&, const std::optional<ObjectPrx>&);

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
        const IceInternal::ServantManagerPtr _servantManager;

        mutable ObjectPtr _dispatchPipeline;
        mutable std::stack<std::function<ObjectPtr(ObjectPtr)>> _middlewareFactoryStack;

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
        const std::optional<SSL::ServerAuthenticationOptions> _serverAuthenticationOptions;
    };
}

#endif
