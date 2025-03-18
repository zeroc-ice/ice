// Copyright (c) ZeroC, Inc.

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
        [[nodiscard]] const std::string& getName() const noexcept final;

        [[nodiscard]] CommunicatorPtr getCommunicator() const noexcept final;

        void activate() final;
        void hold() final;
        void waitForHold() final;
        void deactivate() noexcept override;
        void waitForDeactivate() noexcept final;
        [[nodiscard]] bool isDeactivated() const noexcept final;
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
        [[nodiscard]] ObjectPtr find(const Identity&) const final;
        [[nodiscard]] ObjectPtr findFacet(const Identity&, std::string_view) const final;
        [[nodiscard]] FacetMap findAllFacets(const Identity&) const final;
        [[nodiscard]] ObjectPtr findByProxy(const ObjectPrx&) const final;
        [[nodiscard]] ObjectPtr findDefaultServant(std::string_view) const final;
        void addServantLocator(ServantLocatorPtr, std::string) final;
        ServantLocatorPtr removeServantLocator(std::string_view) final;
        [[nodiscard]] ServantLocatorPtr findServantLocator(std::string_view) const final;

        [[nodiscard]] const ObjectPtr& dispatchPipeline() const noexcept final;

        [[nodiscard]] ObjectPrx _createProxy(Identity) const final;
        [[nodiscard]] ObjectPrx _createDirectProxy(Identity) const final;
        [[nodiscard]] ObjectPrx _createIndirectProxy(Identity) const final;

        void setLocator(std::optional<LocatorPrx>) final;
        [[nodiscard]] std::optional<LocatorPrx> getLocator() const noexcept override;
        [[nodiscard]] EndpointSeq getEndpoints() const override;

        [[nodiscard]] EndpointSeq getPublishedEndpoints() const override;
        void setPublishedEndpoints(EndpointSeq) final;

        [[nodiscard]] bool isLocal(const IceInternal::ReferencePtr&) const;

        void flushAsyncBatchRequests(const IceInternal::CommunicatorFlushBatchAsyncPtr&, CompressBatch);

        void updateConnectionObservers();
        void updateThreadObservers();

        void incDirectCount();
        void decDirectCount();

        [[nodiscard]] IceInternal::ThreadPoolPtr getThreadPool() const;
        void setAdapterOnConnection(const ConnectionIPtr&);
        [[nodiscard]] size_t messageSizeMax() const { return _messageSizeMax; }

        ObjectAdapterI(
            IceInternal::InstancePtr,
            CommunicatorPtr,
            IceInternal::ObjectAdapterFactoryPtr,
            std::string name,
            bool,
            std::optional<SSL::ServerAuthenticationOptions>);
        ~ObjectAdapterI() override;

        [[nodiscard]] const std::optional<SSL::ServerAuthenticationOptions>&
        serverAuthenticationOptions() const noexcept
        {
            return _serverAuthenticationOptions;
        }

    private:
        void initialize(std::optional<RouterPrx>);
        friend class IceInternal::ObjectAdapterFactory;

        [[nodiscard]] ObjectPrx newProxy(Identity, std::string) const;
        [[nodiscard]] ObjectPrx newDirectProxy(Identity, std::string) const;
        [[nodiscard]] ObjectPrx newIndirectProxy(Identity, std::string, std::string) const;
        void checkForDeactivation() const;
        void checkForDestruction() const;
        [[nodiscard]] std::vector<IceInternal::EndpointIPtr> parseEndpoints(std::string_view, bool) const;
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
        State _state{StateUninitialized};
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
        int _directCount{0}; // The number of direct proxies dispatching on this object adapter.
        bool _noConfig;
        size_t _messageSizeMax{0};
        mutable std::recursive_mutex _mutex;
        std::condition_variable_any _conditionVariable;
        const std::optional<SSL::ServerAuthenticationOptions> _serverAuthenticationOptions;
    };
}

#endif
