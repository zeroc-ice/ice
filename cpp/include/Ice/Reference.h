// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_REFERENCE_H
#define ICE_REFERENCE_H

#include <Ice/ReferenceF.h>
#include <Ice/EndpointF.h>
#include <Ice/InstanceF.h>
#include <Ice/Shared.h>

namespace __Ice
{

class ICE_API ReferenceI : virtual public Shared
{
public:

    ReferenceI(const Instance&, const std::string&, const Endpoint&);
    virtual ~ReferenceI();

    bool operator==(const ReferenceI&) const;
    bool operator!=(const ReferenceI&) const;

    //
    // All  members are const, because References are immutable.
    //
    const Instance instance;
    const std::string identity;
    const Endpoint endpoint;
 
private:

    ReferenceI(const ReferenceI&);
    void operator=(const ReferenceI&);
};

}

#endif
