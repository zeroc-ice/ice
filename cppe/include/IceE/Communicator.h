// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_COMMUNICATOR_H
#define ICEE_COMMUNICATOR_H

#include <IceE/CommunicatorF.h>
#include <IceE/LoggerF.h>
#include <IceE/PropertiesF.h>
#include <IceE/InstanceF.h>

#include <IceE/RecMutex.h>
#include <IceE/Initialize.h> // For the friend declarations.

namespace Ice
{

class ICE_API Communicator : public ::IceUtil::Shared
{
public:
    
    void destroy();
    void shutdown();
    void waitForShutdown();

    ObjectPrx stringToProxy(const std::string&) const;
    std::string proxyToString(const ObjectPrx&) const;

    ObjectAdapterPtr createObjectAdapter(const std::string&);
    ObjectAdapterPtr createObjectAdapterWithEndpoints(const std::string&, const std::string&);

    void setDefaultContext(const Context&);
    Context getDefaultContext() const;

    PropertiesPtr getProperties() const;

    LoggerPtr getLogger() const;
    void setLogger(const LoggerPtr&);

#ifdef ICEE_HAS_ROUTER
    RouterPrx getDefaultRouter() const;
    void setDefaultRouter(const RouterPrx&);
#endif

#ifdef ICEE_HAS_LOCATOR
    LocatorPrx getDefaultLocator() const;
    void setDefaultLocator(const LocatorPrx&);
#endif

#ifdef ICEE_HAS_BATCH
    void flushBatchRequests();
#endif

private:

    Communicator(const PropertiesPtr&);
    ~Communicator();

    //
    // Certain initialization tasks need to be completed after the
    // constructor.
    //
    void finishSetup(int&, char*[]);

    friend ICE_API CommunicatorPtr initialize(int&, char*[], Int);
    friend ICE_API CommunicatorPtr initializeWithProperties(int&, char*[], const PropertiesPtr&, Int);
    friend ICE_API ::IceInternal::InstancePtr IceInternal::getInstance(const ::Ice::CommunicatorPtr&);

    const ::IceInternal::InstancePtr _instance;
    const ::Ice::Context _dfltContext;
};

}

#endif
