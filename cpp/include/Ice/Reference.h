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

class Stream;

class Reference : public Shared
{
public:

    Reference(const Instance_ptr&, const std::string&,
	       const std::vector<Endpoint_ptr>&);
    Reference(const Instance_ptr&, const std::string&);
    Reference(Stream*);

    void streamWrite(Stream*) const;

    //
    // All  members are const, because References are immutable.
    //
    const Instance_ptr instance;
    const std::string identity;

    enum Mode
    {
	ModeTwoway,
	ModeOneway,
	ModeDatagram
    };
    const Mode mode;

    const std::vector<Endpoint_ptr> endpoints;

    //
    // Get a new reference, based on the existing one, overwriting
    // certain values.
    //
    Reference_ptr changeTimeout(int) const;
    Reference_ptr changeMode(Mode) const;
 
    bool operator==(const Reference&) const;
    bool operator!=(const Reference&) const;
    bool operator<(const Reference&) const;
};

}

#endif
