// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_COMMUNICATOR_H
#define ICE_COMMUNICATOR_H

#include <Ice/CommunicatorF.h>
#include <Ice/InstanceF.h>
#include <Ice/StubF.h>
#include <Ice/ObjectAdapterF.h>
#include <Ice/LoggerF.h>
#include <Ice/Shared.h>

namespace Ice
{

class ICE_API CommunicatorI : public ::__Ice::Shared, public JTCRecursiveMutex
{
public:
    
    void destroy();
    void shutdown();
    void waitForShutdown();

    Object referenceFromString(const std::string&);

    ObjectAdapter createObjectAdapter(const std::string&);

    Logger logger();
    void logger(const Logger&);

private:

    CommunicatorI(const CommunicatorI&);
    void operator=(const CommunicatorI&);

    CommunicatorI();
    virtual ~CommunicatorI();
    // May create CommunicatorIs
    friend ICE_API Communicator initialize(int&, char*[]);

    ::__Ice::Instance instance_;
    std::vector<ObjectAdapter> adapters_;
};

ICE_API Communicator initialize(int&, char*[]);

}

#endif
