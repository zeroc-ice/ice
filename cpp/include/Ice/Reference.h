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

    ReferenceI(const Instance&, const std::string&,
	       const std::vector<Endpoint>&);
    virtual ~ReferenceI();

    //
    // All  members are const, because References are immutable.
    //
    const Instance instance;
    const std::string identity;
    const std::vector<Endpoint> endpoints;

    enum Mode
    {
	ModeTwoway,
	ModeOneway,
	ModeDatagram
    };
    const Mode mode;

    //
    // Get a new reference, based on the existing one, overwriting
    // certain values.
    //
    Reference changeMode(Mode) const;
    Reference changeTimeout(int) const;
 
    bool operator==(const ReferenceI&) const;
    bool operator!=(const ReferenceI&) const;
    bool operator<(const ReferenceI&) const;

private:

    ReferenceI(const ReferenceI&);
    void operator=(const ReferenceI&);
};

}

#endif
