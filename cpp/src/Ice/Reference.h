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
#include <Ice/ReferenceFactoryF.h>
#include <Ice/EndpointF.h>
#include <Ice/InstanceF.h>
#include <Ice/RouterF.h>
#include <Ice/RouterInfoF.h>
#include <Ice/ObjectAdapterF.h>
#include <Ice/Identity.h>

namespace IceInternal
{

class BasicStream;

class Reference : public ::IceUtil::Shared
{
public:

    enum Mode
    {
	ModeTwoway,
	ModeOneway,
	ModeBatchOneway,
	ModeDatagram,
	ModeBatchDatagram,
	ModeLast = ModeBatchDatagram
    };

    bool operator==(const Reference&) const;
    bool operator!=(const Reference&) const;
    bool operator<(const Reference&) const;

    //
    // Marshal the reference
    //
    void streamWrite(BasicStream*) const;

    //
    // Convert the reference to its string form
    //
    std::string toString() const;

    //
    // All  members are const, because References are immutable.
    //
    const InstancePtr instance;
    const Ice::Identity identity;
    const std::string facet;
    const Mode mode;
    const bool secure;
    const bool compress;
    const std::vector<EndpointPtr> origEndpoints; // Original endpoints.
    const std::vector<EndpointPtr> endpoints; // Actual endpoints, changed by a location forwards.
    const RouterInfoPtr routerInfo; // Null if no router is used.
    const Ice::ObjectAdapterPtr reverseAdapter; // For reverse communications using the adapter's incoming connections.
    const Ice::Int hashValue;

    //
    // Get a new reference, based on the existing one, overwriting
    // certain values.
    //
    ReferencePtr changeIdentity(const Ice::Identity&) const;
    ReferencePtr changeFacet(const std::string&) const;
    ReferencePtr changeTimeout(int) const;
    ReferencePtr changeMode(Mode) const;
    ReferencePtr changeSecure(bool) const;
    ReferencePtr changeCompress(bool) const;
    ReferencePtr changeEndpoints(const std::vector<EndpointPtr>&) const;
    ReferencePtr changeRouter(const ::Ice::RouterPrx&) const;
    ReferencePtr changeDefault() const;
 
private:

    Reference(const InstancePtr&, const Ice::Identity&, const std::string&, Mode, bool, bool,
	      const std::vector<EndpointPtr>&, const std::vector<EndpointPtr>&,
	      const RouterInfoPtr&, const Ice::ObjectAdapterPtr&);
    friend class ReferenceFactory;
};

}

#endif
