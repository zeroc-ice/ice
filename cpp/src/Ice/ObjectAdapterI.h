// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
#include <Ice/Exception.h>
#include <Ice/Process.h>
#include <Ice/BuiltinSequences.h>
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
    virtual ObjectPtr remove(const Identity&);
    virtual ObjectPtr removeFacet(const Identity&, const std::string&);
    virtual FacetMap removeAllFacets(const Identity&);
    virtual ObjectPtr find(const Identity&) const;
    virtual ObjectPtr findFacet(const Identity&, const std::string&) const;
    virtual FacetMap findAllFacets(const Identity&) const;
    virtual ObjectPtr findByProxy(const ObjectPrx&) const;

    virtual void addServantLocator(const ServantLocatorPtr&, const std::string&);
    virtual ServantLocatorPtr findServantLocator(const std::string&) const;

    virtual ObjectPrx createProxy(const Identity&) const;
    virtual ObjectPrx createDirectProxy(const Identity&) const;
    virtual ObjectPrx createIndirectProxy(const Identity&) const;
    virtual ObjectPrx createReverseProxy(const Identity&) const;

    virtual void setLocator(const LocatorPrx&);
    virtual void refreshPublishedEndpoints();
    
    bool isLocal(const ObjectPrx&) const;

    void flushBatchRequests();

    void incDirectCount();
    void decDirectCount();

    IceInternal::ThreadPoolPtr getThreadPool() const;
    IceInternal::ServantManagerPtr getServantManager() const;
    bool getThreadPerConnection() const;
    size_t getThreadPerConnectionStackSize() const;

private:

    ObjectAdapterI(const IceInternal::InstancePtr&, const CommunicatorPtr&, 
                   const IceInternal::ObjectAdapterFactoryPtr&, const std::string&, const std::string&,
                   const RouterPrx&, bool);
    virtual ~ObjectAdapterI();
    friend class IceInternal::ObjectAdapterFactory;
    
    ObjectPrx newProxy(const Identity&, const std::string&) const;
    ObjectPrx newDirectProxy(const Identity&, const std::string&) const;
    ObjectPrx newIndirectProxy(const Identity&, const std::string&, const std::string&) const;
    void checkForDeactivation() const;
    static void checkIdentity(const Identity&);
    std::vector<IceInternal::EndpointIPtr> parseEndpoints(const std::string&) const;
    std::vector<IceInternal::EndpointIPtr> parsePublishedEndpoints();
    void updateLocatorRegistry(const IceInternal::LocatorInfoPtr&, const Ice::ObjectPrx&, bool);
    bool filterProperties(Ice::StringSeq&);

    bool _deactivated;
    IceInternal::InstancePtr _instance;
    CommunicatorPtr _communicator;
    IceInternal::ObjectAdapterFactoryPtr _objectAdapterFactory;
    IceInternal::ThreadPoolPtr _threadPool;
    IceInternal::ServantManagerPtr _servantManager;
    bool _activateOneOffDone;
    const std::string _name;
    const std::string _id;
    const std::string _replicaGroupId;
    std::vector<IceInternal::IncomingConnectionFactoryPtr> _incomingConnectionFactories;
    std::vector<IceInternal::EndpointIPtr> _routerEndpoints;
    IceInternal::RouterInfoPtr _routerInfo;
    std::vector<IceInternal::EndpointIPtr> _publishedEndpoints;
    IceInternal::LocatorInfoPtr _locatorInfo;
    int _directCount; // The number of direct proxies dispatching on this object adapter.
    bool _waitForActivate;
    bool _destroying;
    bool _destroyed;
    bool _noConfig;
    bool _threadPerConnection;
    size_t _threadPerConnectionStackSize;

    class ProcessI : public Process
    {
    public:

        ProcessI(const CommunicatorPtr&);

        virtual void shutdown(const Current&);
        virtual void writeMessage(const std::string&, Int, const Current&);

    private:

        const CommunicatorPtr _communicator;
    };
};

}

#endif
