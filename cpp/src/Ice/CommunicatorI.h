// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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

class CommunicatorI : public Communicator, public ::IceUtil::RecMutex
{
public:
    
    virtual void destroy();
    virtual void shutdown();
    virtual void waitForShutdown();

    virtual ObjectPrx stringToProxy(const std::string&) const;
    virtual std::string proxyToString(const ObjectPrx&) const;

    virtual ObjectAdapterPtr createObjectAdapter(const std::string&);
    virtual ObjectAdapterPtr createObjectAdapterWithEndpoints(const std::string&, const std::string&);

    virtual void addObjectFactory(const ObjectFactoryPtr&, const std::string&);
    virtual void removeObjectFactory(const std::string&);
    virtual ObjectFactoryPtr findObjectFactory(const std::string&) const;

    virtual void setDefaultContext(const Context&);
    virtual Context getDefaultContext() const;

    virtual PropertiesPtr getProperties() const;

    virtual LoggerPtr getLogger() const;
    virtual void setLogger(const LoggerPtr&);

    virtual StatsPtr getStats() const;
    virtual void setStats(const StatsPtr&);

    virtual RouterPrx getDefaultRouter() const;
    virtual void setDefaultRouter(const RouterPrx&);

    virtual LocatorPrx getDefaultLocator() const;
    virtual void setDefaultLocator(const LocatorPrx&);

    virtual PluginManagerPtr getPluginManager() const;

    virtual void flushBatchRequests();

private:

    CommunicatorI(const PropertiesPtr&);
    virtual ~CommunicatorI();

    //
    // Certain initialization tasks need to be completed after the
    // constructor.
    //
    void finishSetup(int&, char*[]);

    friend ICE_API CommunicatorPtr initialize(int&, char*[], Int);
    friend ICE_API CommunicatorPtr initializeWithProperties(int&, char*[], const PropertiesPtr&, Int);
    friend ICE_API ::IceInternal::InstancePtr IceInternal::getInstance(const ::Ice::CommunicatorPtr&);

    bool _destroyed;
    ::IceInternal::InstancePtr _instance;
    ::Ice::Context _dfltContext;

    //
    // We don't want the dynamic libraries to be unloaded until the
    // Communicator's destructor is invoked.
    //
    ::IceInternal::DynamicLibraryListPtr _dynamicLibraryList;
};

}

#endif
