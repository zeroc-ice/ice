// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_ENDPOINT_H
#define ICE_ENDPOINT_H

#include <Ice/EndpointF.h>
#include <Ice/InstanceF.h>
#include <Ice/EndpointDataF.h>
#include <Ice/CollectorF.h>
#include <Ice/LocalException.h>
#include <Ice/Shared.h>

namespace Ice
{

class ICE_API EndpointClosedException : public LocalException
{
public:    

    virtual std::string toString() const;
    virtual LocalException* clone() const;
};

class ICE_API EndpointI : virtual public ::_Ice::Shared, public JTCMutex
{
public:

    EndpointI(const ::_Ice::Instance&, const _Ice::EndpointData&);
    virtual ~EndpointI();

    void run();
    void hold();
    void close();

private:

    EndpointI(const EndpointI&);
    void operator=(const EndpointI&);

    ::_Ice::Instance instance_;
    ::_Ice::EndpointData endpointData_;
    ::_Ice::CollectorFactory collectorFactory_;
};

}

#endif
