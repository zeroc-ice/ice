// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_OBJECT_ADAPTER_H
#define ICE_OBJECT_ADAPTER_H

#include <Ice/ObjectAdapterF.h>
#include <Ice/InstanceF.h>
#include <Ice/EndpointF.h>
#include <Ice/CollectorF.h>
#include <Ice/SkeletonF.h>
#include <Ice/LocalException.h>
#include <Ice/Shared.h>
#include <map>

namespace Ice
{

class CommunicatorI;

class ICE_API ObjectAdapterI : virtual public ::__Ice::Shared, public JTCMutex
{
public:

    void activate();
    void hold();
    void deactivate();

    void add(const ::IceServant::Ice::Object&, const std::string&);

    ::__Ice::Instance __instance() const;
    ::IceServant::Ice::Object __findServant(const std::string&) const;

private:

    ObjectAdapterI(const ::__Ice::Instance&,
		   const std::vector< ::__Ice::Endpoint>&);
    virtual ~ObjectAdapterI();
    friend CommunicatorI; // May create ObjectAdapterIs

    ObjectAdapterI(const ObjectAdapterI&);
    void operator=(const ObjectAdapterI&);

    ::__Ice::Instance instance_;
    std::vector< __Ice::CollectorFactory> factories_;
    std::map<std::string, ::IceServant::Ice::Object> servants_;
};

}

#endif
