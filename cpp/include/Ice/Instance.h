// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_INSTANCE_H
#define ICE_INSTANCE_H

#include <Ice/InstanceF.h>
#include <Ice/CommunicatorF.h>
#include <Ice/ObjectFactoryF.h>
#include <Ice/Stub.h>

namespace _Ice
{

class ICE_API InstanceI : public SimpleShared
{
public:
    
private:

    InstanceI(const InstanceI&);
    void operator=(const InstanceI&);

    InstanceI();
    virtual ~InstanceI();
    void destroy();
    friend class ::Ice::CommunicatorI; // May create and destroy Instances

    ObjectFactory objectFactory_;
};

}

#endif
