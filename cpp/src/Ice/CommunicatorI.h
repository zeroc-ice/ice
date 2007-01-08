// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_COMMUNICATOR_I_H
#define ICE_COMMUNICATOR_I_H

#include <IceUtil/RecMutex.h>
#include <IceUtil/StaticMutex.h>

#include <Ice/DynamicLibraryF.h>
#include <Ice/Initialize.h>
#include <Ice/Communicator.h>

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

    virtual Identity stringToIdentity(const std::string&) const;
    virtual std::string identityToString(const Identity&) const;

    virtual ObjectAdapterPtr createObjectAdapter(const std::string&);
    virtual ObjectAdapterPtr createObjectAdapterWithEndpoints(const std::string&, const std::string&);
    virtual ObjectAdapterPtr createObjectAdapterWithRouter(const std::string&, const RouterPrx&);

    virtual void addObjectFactory(const ObjectFactoryPtr&, const std::string&);
    virtual ObjectFactoryPtr findObjectFactory(const std::string&) const;

    virtual Context getDefaultContext() const;
    virtual void setDefaultContext(const Context&);

    virtual ImplicitContextPtr getImplicitContext() const;

    virtual PropertiesPtr getProperties() const;
    virtual LoggerPtr getLogger() const;
    virtual StatsPtr getStats() const;

    virtual RouterPrx getDefaultRouter() const;
    virtual void setDefaultRouter(const RouterPrx&);

    virtual LocatorPrx getDefaultLocator() const;
    virtual void setDefaultLocator(const LocatorPrx&);

    virtual PluginManagerPtr getPluginManager() const;

    virtual void flushBatchRequests();

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

    const ::IceInternal::InstancePtr _instance;

    //
    // We don't want the dynamic libraries to be unloaded until the
    // Communicator's destructor is invoked.
    //
    const ::IceInternal::DynamicLibraryListPtr _dynamicLibraryList;
};

}

#endif
