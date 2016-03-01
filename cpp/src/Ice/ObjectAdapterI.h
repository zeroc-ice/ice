// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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

class ObjectAdapterI;
typedef IceUtil::Handle<ObjectAdapterI> ObjectAdapterIPtr;

class ObjectAdapterI : public ObjectAdapter, public IceUtil::Monitor<IceUtil::RecMutex>
{
public:

    virtual std::string getName() const;

    virtual CommunicatorPtr getCommunicator() const;

    virtual void activate();
    virtual void hold();
    virtual void waitForHold();
    virtual void deactivate();
    virtual void waitForDeactivate();
    virtual bool isDeactivated() const;
    virtual void destroy();

    virtual ObjectPrx add(const ObjectPtr&, const Identity&);
    virtual ObjectPrx addFacet(const ObjectPtr&, const Identity&, const std::string&);
    virtual ObjectPrx addWithUUID(const ObjectPtr&);
    virtual ObjectPrx addFacetWithUUID(const ObjectPtr&, const std::string&);
    virtual void addDefaultServant(const ObjectPtr&, const std::string&);
    virtual ObjectPtr remove(const Identity&);
    virtual ObjectPtr removeFacet(const Identity&, const std::string&);
    virtual FacetMap removeAllFacets(const Identity&);
    virtual ObjectPtr removeDefaultServant(const std::string&);
    virtual ObjectPtr find(const Identity&) const;
    virtual ObjectPtr findFacet(const Identity&, const std::string&) const;
    virtual FacetMap findAllFacets(const Identity&) const;
    virtual ObjectPtr findByProxy(const ObjectPrx&) const;
    virtual ObjectPtr findDefaultServant(const std::string&) const;


    virtual void addServantLocator(const ServantLocatorPtr&, const std::string&);
    virtual ServantLocatorPtr removeServantLocator(const std::string&);
    virtual ServantLocatorPtr findServantLocator(const std::string&) const;

    virtual ObjectPrx createProxy(const Identity&) const;
    virtual ObjectPrx createDirectProxy(const Identity&) const;
    virtual ObjectPrx createIndirectProxy(const Identity&) const;

    virtual void setLocator(const LocatorPrx&);
    virtual Ice::LocatorPrx getLocator() const;
    virtual void refreshPublishedEndpoints();

    virtual EndpointSeq getEndpoints() const;
    virtual EndpointSeq getPublishedEndpoints() const;

    bool isLocal(const ObjectPrx&) const;

    void flushAsyncBatchRequests(const IceInternal::CommunicatorFlushBatchAsyncPtr&);

    void updateConnectionObservers();
    void updateThreadObservers();

    void incDirectCount();
    void decDirectCount();

    IceInternal::ThreadPoolPtr getThreadPool() const;
    IceInternal::ServantManagerPtr getServantManager() const;
    IceInternal::ACMConfig getACM() const;
    size_t messageSizeMax() const { return _messageSizeMax; }

private:

    ObjectAdapterI(const IceInternal::InstancePtr&, const CommunicatorPtr&,
                   const IceInternal::ObjectAdapterFactoryPtr&, const std::string&, bool);
    virtual ~ObjectAdapterI();
    void initialize(const RouterPrx&);
    friend class IceInternal::ObjectAdapterFactory;

    ObjectPrx newProxy(const Identity&, const std::string&) const;
    ObjectPrx newDirectProxy(const Identity&, const std::string&) const;
    ObjectPrx newIndirectProxy(const Identity&, const std::string&, const std::string&) const;
    void checkForDeactivation() const;
    std::vector<IceInternal::EndpointIPtr> parseEndpoints(const std::string&, bool) const;
    std::vector<IceInternal::EndpointIPtr> parsePublishedEndpoints();
    void updateLocatorRegistry(const IceInternal::LocatorInfoPtr&, const Ice::ObjectPrx&);
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
    std::vector<IceInternal::EndpointIPtr> _routerEndpoints;
    IceInternal::RouterInfoPtr _routerInfo;
    std::vector<IceInternal::EndpointIPtr> _publishedEndpoints;
    IceInternal::LocatorInfoPtr _locatorInfo;
    int _directCount; // The number of direct proxies dispatching on this object adapter.
    bool _noConfig;
    size_t _messageSizeMax;
};

}

#endif
