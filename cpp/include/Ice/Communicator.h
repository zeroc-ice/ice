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
#include <Ice/Stub.h>

namespace Ice
{

class ICE_API CommunicatorI : public ::_Ice::SimpleShared
{
public:
    
    void destroy();

    Object referenceFromString(const std::string&);
    std::string referenceToString(const Object&);

private:

    CommunicatorI(const CommunicatorI&);
    void operator=(const CommunicatorI&);

    CommunicatorI();
    virtual ~CommunicatorI();
    // May create Communicators
    friend ICE_API Communicator initialize(int&, char*[]);

    ::_Ice::Instance instance_;
};

ICE_API Communicator initialize(int&, char*[]);

}

#endif
