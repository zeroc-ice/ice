// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_COMMUNICATOR_I_H
#define ICE_COMMUNICATOR_I_H

#include <IceUtil/RecMutex.h>

#include <Ice/ThreadPoolF.h>
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

    virtual ObjectPrx stringToProxy(const std::string&);
    virtual std::string proxyToString(const ObjectPrx&);

    virtual ObjectAdapterPtr createObjectAdapter(const std::string&);
    virtual ObjectAdapterPtr createObjectAdapterFromProperty(const std::string&, const std::string&);
    virtual ObjectAdapterPtr createObjectAdapterWithEndpoints(const std::string&, const std::string&);

    virtual void addObjectFactory(const ObjectFactoryPtr&, const std::string&);
    virtual void removeObjectFactory(const std::string&);
    virtual ObjectFactoryPtr findObjectFactory(const std::string&);

    virtual void addUserExceptionFactory(const UserExceptionFactoryPtr&, const std::string&);
    virtual void removeUserExceptionFactory(const std::string&);
    virtual UserExceptionFactoryPtr findUserExceptionFactory(const std::string&);

    virtual PropertiesPtr getProperties();

    virtual LoggerPtr getLogger();
    virtual void setLogger(const LoggerPtr&);

    virtual void setDefaultRouter(const RouterPrx&);

    virtual void setDefaultLocator(const LocatorPrx&);

    virtual PluginManagerPtr getPluginManager();

private:

    CommunicatorI(int&, char*[], const PropertiesPtr&);
    virtual ~CommunicatorI();

    //
    // Certain initialization tasks need to be completed after the
    // constructor.
    //
    void finishSetup(int&, char*[]);

    friend ICE_API CommunicatorPtr initialize(int&, char*[], Int);
    friend ICE_API CommunicatorPtr initializeWithProperties(int&, char*[], const PropertiesPtr&, Int);
    friend ICE_API ::IceInternal::InstancePtr IceInternal::getInstance(const ::Ice::CommunicatorPtr&);

    ::IceInternal::InstancePtr _instance;

    //
    // We need _serverThreadPool directly in CommunicatorI. That's
    // because the shutdown() operation is signal-safe, and thus must
    // not access any mutex locks or _instance. It may only access
    // _serverThreadPool->initiateShutdown(), which is signal-safe as
    // well.
    //
    ::IceInternal::ThreadPoolPtr _serverThreadPool;
};

}

#endif
