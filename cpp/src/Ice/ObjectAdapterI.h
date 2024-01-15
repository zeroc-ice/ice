//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_OBJECT_ADAPTER_I_H
#define ICE_OBJECT_ADAPTER_I_H

#include <IceUtil/Shared.h>
#include <IceUtil/RecMutex.h>
#include <IceUtil/Monitor.h>
#include <Ice/ObjectAdapter.h>
#include <Ice/InstanceF.h>
#include <Ice/ObjectAdapterFactoryF.h>
#include <Ice/CommunicatorF.h>
#include <Ice/ConnectionFactoryF.h>
#include <Ice/ServantManagerF.h>
#include <Ice/ProxyF.h>
#include <Ice/ObjectF.h>
#include <Ice/RouterInfoF.h>
#include <Ice/EndpointIF.h>
#include <Ice/LocatorInfoF.h>
#include <Ice/ThreadPoolF.h>
#include <Ice/OutgoingAsyncF.h>
#include <Ice/Exception.h>
#include <Ice/BuiltinSequences.h>
#include <Ice/Proxy.h>
#include <Ice/ACM.h>
#include <list>

namespace Ice
{
class ObjectAdapterI final : public ObjectAdapter,
                             public IceUtil::Monitor<IceUtil::RecMutex>,
                             public std::enable_shared_from_this<ObjectAdapterI>

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

    ObjectPrxPtr add(const std::shared_ptr<Object>&, const Identity&) final;
    ObjectPrxPtr addFacet(const std::shared_ptr<Object>&, const Identity&, const std::string&) final;
    ObjectPrxPtr addWithUUID(const std::shared_ptr<Object>&) final;
    ObjectPrxPtr addFacetWithUUID(const std::shared_ptr<Object>&, const std::string&) final;
    void addDefaultServant(const std::shared_ptr<Object>&, const std::string&) final;
    std::shared_ptr<Object> remove(const Identity&) final;
    std::shared_ptr<Object> removeFacet(const Identity&, const std::string&) final;
    FacetMap removeAllFacets(const Identity&) final;
    std::shared_ptr<Object> removeDefaultServant(const std::string&) final;
    std::shared_ptr<Object> find(const Identity&) const final;
    std::shared_ptr<Object> findFacet(const Identity&, const std::string&) const final;
    FacetMap findAllFacets(const Identity&) const final;
    std::shared_ptr<Object> findByProxy(const ObjectPrxPtr&) const final;
    std::shared_ptr<Object> findDefaultServant(const std::string&) const final;

    void addServantLocator(const std::shared_ptr<ServantLocator>&, const std::string&) final;
    std::shared_ptr<ServantLocator> removeServantLocator(const std::string&) final;
    std::shared_ptr<ServantLocator> findServantLocator(const std::string&) const final;

    ObjectPrxPtr createProxy(const Identity&) const final;
    ObjectPrxPtr createDirectProxy(const Identity&) const final;
    ObjectPrxPtr createIndirectProxy(const Identity&) const final;

    void setLocator(const LocatorPrxPtr&) final;
    LocatorPrxPtr getLocator() const noexcept;
    EndpointSeq getEndpoints() const noexcept;

    void refreshPublishedEndpoints() final;
    EndpointSeq getPublishedEndpoints() const noexcept;
    void setPublishedEndpoints(const EndpointSeq&) final;

#ifdef ICE_SWIFT
    dispatch_queue_t getDispatchQueue() const final;
#endif

    bool isLocal(const ObjectPrxPtr&) const;

    void flushAsyncBatchRequests(const IceInternal::CommunicatorFlushBatchAsyncPtr&, CompressBatch);

    void updateConnectionObservers();
    void updateThreadObservers();

    void incDirectCount();
    void decDirectCount();

    IceInternal::ThreadPoolPtr getThreadPool() const;
    IceInternal::ServantManagerPtr getServantManager() const;
    IceInternal::ACMConfig getACM() const;
    void setAdapterOnConnection(const Ice::ConnectionIPtr&);
    size_t messageSizeMax() const { return _messageSizeMax; }

    ObjectAdapterI(const IceInternal::InstancePtr&, const CommunicatorPtr&,
                   const IceInternal::ObjectAdapterFactoryPtr&, const std::string&, bool);
    virtual ~ObjectAdapterI();

private:

    void initialize(const RouterPrxPtr&);
    friend class IceInternal::ObjectAdapterFactory;

    ObjectPrxPtr newProxy(const Identity&, const std::string&) const;
    ObjectPrxPtr newDirectProxy(const Identity&, const std::string&) const;
    ObjectPrxPtr newIndirectProxy(const Identity&, const std::string&, const std::string&) const;
    void checkForDeactivation() const;
    std::vector<IceInternal::EndpointIPtr> parseEndpoints(const std::string&, bool) const;
    std::vector<IceInternal::EndpointIPtr> computePublishedEndpoints();
    void updateLocatorRegistry(const IceInternal::LocatorInfoPtr&, const Ice::ObjectPrxPtr&);
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
    IceInternal::ServantManagerPtr _servantManager;
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
};

}

#endif
