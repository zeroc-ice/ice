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
#include <Ice/EndpointF.h>
#include <Ice/LoggerF.h>
#include <Ice/Shared.h>

namespace Ice
{

class ICE_API CommunicatorI : public ::__Ice::Shared, public JTCMutex
{
public:
    
    void destroy();

    void waitForShutdown();

    Object referenceFromString(const std::string&);
    std::string referenceToString(const Object&);

    Endpoint createEndpoint(); // TODO: arguments

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
};

ICE_API Communicator initialize(int&, char*[]);

}

#endif
