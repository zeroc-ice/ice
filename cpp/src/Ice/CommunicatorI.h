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

#include <Ice/InstanceF.h>
#include <Ice/Communicator.h>
#include <Ice/Shared.h>

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

    virtual void installValueFactory(const ValueFactoryPtr&, const std::string&);

    virtual PropertiesPtr getProperties();

    virtual LoggerPtr getLogger();
    virtual void setLogger(const LoggerPtr&);

    virtual PicklerPtr getPickler();

private:

    CommunicatorI(const PropertiesPtr&);
    virtual ~CommunicatorI();

    friend ICE_API CommunicatorPtr initialize(int&, char*[], Int);
    friend ICE_API CommunicatorPtr initializeWithProperties(const PropertiesPtr&, Int);

    ::IceInternal::InstancePtr _instance;
};

}

#endif
