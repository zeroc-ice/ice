// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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

class CommunicatorI : public Communicator
{
public:
    
    virtual void destroy();
    virtual void shutdown();
    virtual void waitForShutdown();
    virtual bool isShutdown() const;

    virtual ObjectPrx stringToProxy(const std::string&) const;
    virtual std::string proxyToString(const ObjectPrx&) const;

    virtual ObjectPrx propertyToProxy(const std::string&) const;
    virtual PropertyDict proxyToProperty(const ObjectPrx&, const std::string&) const;

    virtual Identity stringToIdentity(const std::string&) const;
    virtual std::string identityToString(const Identity&) const;

    virtual ObjectAdapterPtr createObjectAdapter(const std::string&);
    virtual ObjectAdapterPtr createObjectAdapterWithEndpoints(const std::string&, const std::string&);
    virtual ObjectAdapterPtr createObjectAdapterWithRouter(const std::string&, const RouterPrx&);

    virtual void addObjectFactory(const ObjectFactoryPtr&, const std::string&);
    virtual ObjectFactoryPtr findObjectFactory(const std::string&) const;

    virtual ImplicitContextPtr getImplicitContext() const;

    virtual PropertiesPtr getProperties() const;
    virtual LoggerPtr getLogger() const;
    virtual Ice::Instrumentation::CommunicatorObserverPtr getObserver() const;

    virtual RouterPrx getDefaultRouter() const;
    virtual void setDefaultRouter(const RouterPrx&);

    virtual LocatorPrx getDefaultLocator() const;
    virtual void setDefaultLocator(const LocatorPrx&);

    virtual PluginManagerPtr getPluginManager() const;

    virtual void flushBatchRequests();

    virtual AsyncResultPtr begin_flushBatchRequests();
    virtual AsyncResultPtr begin_flushBatchRequests(const CallbackPtr&, const LocalObjectPtr& = 0);
    virtual AsyncResultPtr begin_flushBatchRequests(const Callback_Communicator_flushBatchRequestsPtr&,
                                                    const LocalObjectPtr& = 0);

    virtual AsyncResultPtr begin_flushBatchRequests(
        const IceInternal::Function<void (const Exception&)>&,
        const IceInternal::Function<void (bool)>& = IceInternal::Function<void (bool)>());

    virtual void end_flushBatchRequests(const AsyncResultPtr&);

    virtual ObjectPrx createAdmin(const ObjectAdapterPtr&, const Identity&);
    virtual ObjectPrx getAdmin() const;
    virtual void addAdminFacet(const ObjectPtr&, const std::string&);
    virtual ObjectPtr removeAdminFacet(const std::string&);
    virtual ObjectPtr findAdminFacet(const std::string&);
    virtual FacetMap findAllAdminFacets();

private:

    CommunicatorI(const InitializationData&);
    virtual ~CommunicatorI();

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
