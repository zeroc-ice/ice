// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
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
#include <Ice/Exception.h>
#include <Ice/EndpointF.h>
#include <Ice/LocatorInfoF.h>
#include <Ice/ThreadPoolF.h>
#include <Ice/Process.h>
#include <list>

namespace Ice
{

class ObjectAdapterI;
typedef IceUtil::Handle<ObjectAdapterI> ObjectAdapterIPtr;

class ObjectAdapterI : public ObjectAdapter, public IceUtil::Monitor<IceUtil::RecMutex>
{
public:

    virtual std::string getName();

    virtual CommunicatorPtr getCommunicator();

    virtual void activate();
    virtual void hold();
    virtual void waitForHold();
    virtual void deactivate();
    virtual void waitForDeactivate();

    virtual ObjectPrx add(const ObjectPtr&, const Identity&);
    virtual ObjectPrx addFacet(const ObjectPtr&, const Identity&, const std::string&);
    virtual ObjectPrx addWithUUID(const ObjectPtr&);
    virtual ObjectPrx addFacetWithUUID(const ObjectPtr&, const std::string&);
    virtual void remove(const Identity&);
    virtual void removeFacet(const Identity&, const std::string&);
    virtual FacetMap removeAllFacets(const Identity&);
    virtual ObjectPtr find(const Identity&);
    virtual ObjectPtr findFacet(const Identity&, const std::string&);
    virtual FacetMap findAllFacets(const Identity&);
    virtual ObjectPtr findByProxy(const ObjectPrx&);

    virtual void addServantLocator(const ServantLocatorPtr&, const std::string&);
    virtual ServantLocatorPtr findServantLocator(const std::string&);

    virtual ObjectPrx createProxy(const Identity&);
    virtual ObjectPrx createDirectProxy(const Identity&);
    virtual ObjectPrx createReverseProxy(const Identity&);

    virtual void addRouter(const RouterPrx&);

    virtual void setLocator(const LocatorPrx&);
    virtual LocatorPrx getLocator();
    
    bool isLocal(const ObjectPrx&) const;

    std::list<IceInternal::ConnectionPtr> getIncomingConnections() const;

    void flushBatchRequests();

    void incDirectCount();
    void decDirectCount();

    IceInternal::ThreadPoolPtr getThreadPool() const;
    IceInternal::ServantManagerPtr getServantManager() const;

private:

    ObjectAdapterI(const IceInternal::InstancePtr&, const CommunicatorPtr&, const std::string&);
    virtual ~ObjectAdapterI();
    friend class IceInternal::ObjectAdapterFactory;
    
    ObjectPrx newProxy(const Identity&) const;
    ObjectPrx newDirectProxy(const Identity&) const;
    void checkForDeactivation() const;
    static void checkIdentity(const Identity&);

    bool _deactivated;
    IceInternal::InstancePtr _instance;
    CommunicatorPtr _communicator;
    IceInternal::ThreadPoolPtr _threadPool;
    IceInternal::ServantManagerPtr _servantManager;
    bool _printAdapterReadyDone;
    const std::string _name;
    const std::string _id;
    std::vector<IceInternal::IncomingConnectionFactoryPtr> _incomingConnectionFactories;
    std::vector<IceInternal::EndpointPtr> _routerEndpoints;
    IceInternal::LocatorInfoPtr _locatorInfo;
    int _directCount; // The number of direct proxies dispatching on this object adapter.
    bool _waitForDeactivate;

    class ProcessI : public Process
    {
    public:

        ProcessI(const CommunicatorPtr&);

        virtual void shutdown(const Current&);

    private:

        CommunicatorPtr _communicator;
    };
};

}

#endif
