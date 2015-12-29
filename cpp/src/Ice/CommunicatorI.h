// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_COMMUNICATOR_I_H
#define ICE_COMMUNICATOR_I_H

#include <IceUtil/RecMutex.h>

#include <Ice/DynamicLibraryF.h>
#include <Ice/Initialize.h>
#include <Ice/Communicator.h>
#include <Ice/CommunicatorAsync.h>

namespace Ice
{

class CommunicatorI;
ICE_DEFINE_PTR(CommunicatorIPtr, CommunicatorI);

class CommunicatorI : public EnableSharedFromThis<CommunicatorI>,
                      public Communicator
{
public:

    virtual void destroy();
    virtual void shutdown();
    virtual void waitForShutdown();
    virtual bool isShutdown() const;

    virtual ObjectPrxPtr stringToProxy(const std::string&) const;
    virtual std::string proxyToString(const ObjectPrxPtr&) const;

    virtual ObjectPrxPtr propertyToProxy(const std::string&) const;
    virtual PropertyDict proxyToProperty(const ObjectPrxPtr&, const std::string&) const;

    virtual Identity stringToIdentity(const std::string&) const;
    virtual std::string identityToString(const Identity&) const;

    virtual ObjectAdapterPtr createObjectAdapter(const std::string&);
    virtual ObjectAdapterPtr createObjectAdapterWithEndpoints(const std::string&, const std::string&);
    virtual ObjectAdapterPtr createObjectAdapterWithRouter(const std::string&, const RouterPrxPtr&);

    virtual void addObjectFactory(const ObjectFactoryPtr&, const std::string&);
    virtual ObjectFactoryPtr findObjectFactory(const std::string&) const;

#ifdef ICE_CPP11_MAPPING
    virtual void addValueFactory(std::function<std::shared_ptr<Ice::Value> (std::string)>, const ::std::string&);
    virtual std::function<::std::shared_ptr<Ice::Value> (const std::string&)> findValueFactory(const std::string&) const;
#else
    virtual void addValueFactory(const ValueFactoryPtr&, const std::string&);
    virtual ValueFactoryPtr findValueFactory(const std::string&) const;
#endif

    virtual ImplicitContextPtr getImplicitContext() const;

    virtual PropertiesPtr getProperties() const;
    virtual LoggerPtr getLogger() const;
    virtual Ice::Instrumentation::CommunicatorObserverPtr getObserver() const;

    virtual RouterPrxPtr getDefaultRouter() const;
    virtual void setDefaultRouter(const RouterPrxPtr&);

    virtual LocatorPrxPtr getDefaultLocator() const;
    virtual void setDefaultLocator(const LocatorPrxPtr&);

    virtual PluginManagerPtr getPluginManager() const;

    virtual void flushBatchRequests();

#ifdef ICE_CPP11_MAPPING
    virtual ::std::function<void ()>
    flushBatchRequests_async(::std::function<void (::std::exception_ptr)> exception,
                             ::std::function<void (bool)> sent);
#else
    virtual AsyncResultPtr begin_flushBatchRequests();
    virtual AsyncResultPtr begin_flushBatchRequests(const CallbackPtr&, const LocalObjectPtr& = 0);
    virtual AsyncResultPtr begin_flushBatchRequests(const Callback_Communicator_flushBatchRequestsPtr&,
                                                    const LocalObjectPtr& = 0);

    virtual void end_flushBatchRequests(const AsyncResultPtr&);
#endif

    virtual ObjectPrxPtr createAdmin(const ObjectAdapterPtr&, const Identity&);
    virtual ObjectPrxPtr getAdmin() const;
    virtual void addAdminFacet(const ObjectPtr&, const std::string&);
    virtual ObjectPtr removeAdminFacet(const std::string&);
    virtual ObjectPtr findAdminFacet(const std::string&);
    virtual FacetMap findAllAdminFacets();

    virtual ~CommunicatorI();

private:

    static CommunicatorIPtr
    create(const InitializationData&);

    //
    // Certain initialization tasks need to be completed after the
    // constructor.
    //
    void finishSetup(int&, char*[]);

    friend ICE_API CommunicatorPtr initialize(int&, char*[], const InitializationData&, Int);
    friend ICE_API CommunicatorPtr initialize(StringSeq&, const InitializationData&, Int);
    friend ICE_API CommunicatorPtr initialize(const InitializationData&, Int);
    friend ICE_API ::IceInternal::InstancePtr IceInternal::getInstance(const ::Ice::CommunicatorPtr&);
    friend ICE_API ::IceUtil::TimerPtr IceInternal::getInstanceTimer(const ::Ice::CommunicatorPtr&);

    AsyncResultPtr __begin_flushBatchRequests(const IceInternal::CallbackBasePtr&, const LocalObjectPtr&);

    const ::IceInternal::InstancePtr _instance;

    //
    // We don't want the dynamic libraries to be unloaded until the
    // Communicator's destructor is invoked.
    //
    const ::IceInternal::DynamicLibraryListPtr _dynamicLibraryList;
};

}

#endif
