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

#include <IceUtil/Shared.h>
#include <Ice/ReferenceF.h>
#include <Ice/EndpointF.h>
#include <Ice/InstanceF.h>

namespace IceInternal
{

class Stream;

class Reference : public ::IceUtil::Shared
{
public:

    enum Mode
    {
	ModeTwoway,
	ModeOneway,
	ModeBatchOneway,
	ModeDatagram,
	ModeBatchDatagram
    };

    Reference(const InstancePtr&, const std::string&, Mode, bool,
	      const std::vector<EndpointPtr>&, const std::vector<EndpointPtr>&);
    Reference(const InstancePtr&, const std::string&);
    Reference(const std::string&, Stream*);

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
    const Mode mode;
    const bool secure;
    const std::vector<EndpointPtr> origEndpoints; // Original endpoints
    const std::vector<EndpointPtr> endpoints; // Actual endpoints (set by a location forward)

    //
    // Get a new reference, based on the existing one, overwriting
    // certain values.
    //
    ReferencePtr changeIdentity(const std::string&) const;
    ReferencePtr changeTimeout(int) const;
    ReferencePtr changeMode(Mode) const;
    ReferencePtr changeSecure(bool) const;
    ReferencePtr changeEndpoints(const std::vector<EndpointPtr>&) const;
 
    bool operator==(const Reference&) const;
    bool operator<(const Reference&) const;
};

}

#endif
