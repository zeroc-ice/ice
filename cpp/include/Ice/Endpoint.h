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
#include <Ice/SkeletonF.h>
#include <Ice/LocalException.h>
#include <Ice/Shared.h>
#include <map>

namespace Ice
{

class ICE_API EndpointClosedException : public LocalException
{
public:    

    virtual std::string toString() const;
    virtual LocalException* clone() const;
};

class CommunicatorI;

class ICE_API EndpointI : virtual public ::__Ice::Shared, public JTCMutex
{
public:

    void activate();
    void hold();
    void close();

    void add(const ::IceServant::Ice::Object&, const std::string&);

    ::__Ice::Instance __instance() const;
    ::__Ice::EndpointData __endpointData() const;
    ::IceServant::Ice::Object __findServant(const std::string&) const;

private:

    EndpointI(const ::__Ice::Instance&, const __Ice::EndpointData&);
    virtual ~EndpointI();
    friend CommunicatorI; // May create EndpointIs

    EndpointI(const EndpointI&);
    void operator=(const EndpointI&);

    ::__Ice::Instance instance_;
    ::__Ice::EndpointData endpointData_;
    ::__Ice::CollectorFactory collectorFactory_;
    std::map<std::string, ::IceServant::Ice::Object> servants_;
};

}

#endif
