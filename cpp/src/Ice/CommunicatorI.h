// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_COMMUNICATOR_I_H
#define ICE_COMMUNICATOR_I_H

#include <Ice/ThreadPoolF.h>
#include <Ice/Initialize.h>
#include <Ice/Communicator.h>

namespace Ice
{

class CommunicatorI : public Communicator, public JTCRecursiveMutex
{
public:
    
    virtual void destroy();
    virtual void shutdown();
    virtual void waitForShutdown();

    virtual ObjectPrx stringToProxy(const std::string&);
    virtual std::string proxyToString(const ObjectPrx&);

    virtual ObjectAdapterPtr createObjectAdapter(const std::string&);
    virtual ObjectAdapterPtr createObjectAdapterWithEndpoints(const std::string&, const std::string&);

    virtual void addServantFactory(const ServantFactoryPtr&, const std::string&);
    virtual void removeServantFactory(const std::string&);
    virtual ServantFactoryPtr findServantFactory(const std::string&);

    virtual void addUserExceptionFactory(const UserExceptionFactoryPtr&, const std::string&);
    virtual void removeUserExceptionFactory(const std::string&);
    virtual UserExceptionFactoryPtr findUserExceptionFactory(const std::string&);

    virtual PropertiesPtr getProperties();

    virtual LoggerPtr getLogger();
    virtual void setLogger(const LoggerPtr&);

    virtual StreamPtr createStream();

private:

    CommunicatorI(const PropertiesPtr&);
    virtual ~CommunicatorI();

    friend ICE_API CommunicatorPtr initialize(int&, char*[], Int);
    friend ICE_API CommunicatorPtr initializeWithProperties(const PropertiesPtr&, Int);
    friend ICE_API ::IceInternal::InstancePtr IceInternal::getInstance(const ::Ice::CommunicatorPtr&);

    ::IceInternal::InstancePtr _instance;

    //
    // We need _threadPool directly in CommunicatorI, and it must
    // never be set to null. That's because the shutdown() operation
    // is signal-safe, and thus must not access any mutex locks or
    // _instance. It may only access _threadPool->initiateShutdown(),
    // which is signal-safe as well.
    //
    ::IceInternal::ThreadPoolPtr _threadPool;
};

}

#endif
