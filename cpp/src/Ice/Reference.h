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

namespace IceInternal
{

class Stream;

class Reference : public Shared
{
public:

    Reference(const InstancePtr&, const std::string&,
	      const std::vector<EndpointPtr>&, const std::vector<EndpointPtr>&);
    Reference(const InstancePtr&, const std::string&);
    Reference(Stream*);

    //
    // Marshal the reference
    //
    void streamWrite(Stream*) const;

    //
    // Convert the reference to its string form
    //
    std::string toString() const;

    //
    // All  members are const, because References are immutable.
    //
    const InstancePtr instance;
    const std::string identity;

    enum Mode
    {
	ModeTwoway,
	ModeOneway,
	ModeSecure,
	ModeDatagram
    };
    const Mode mode;

    const std::vector<EndpointPtr> origEndpoints; // Original endpoints
    const std::vector<EndpointPtr> endpoints; // Actual endpoints (set by a location forward)

    //
    // Get a new reference, based on the existing one, overwriting
    // certain values.
    //
    ReferencePtr changeIdentity(const std::string&) const;
    ReferencePtr changeTimeout(int) const;
    ReferencePtr changeMode(Mode) const;
    ReferencePtr changeEndpoints(const std::vector<EndpointPtr>&) const;
 
    bool operator==(const Reference&) const;
    bool operator!=(const Reference&) const;
    bool operator<(const Reference&) const;
};

}

#endif
