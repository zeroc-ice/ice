// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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
#include <Ice/LocatorF.h>
#include <Ice/RouterInfoF.h>
#include <Ice/LocatorInfoF.h>
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
    const Ice::Context context;
    const Ice::FacetPath facet;
    const Mode mode;
    const bool secure;
    const std::string adapterId;
    const std::vector<EndpointPtr> endpoints;
    const RouterInfoPtr routerInfo; // Null if no router is used.
    const LocatorInfoPtr locatorInfo; // Null if no locator is used.
    const Ice::ObjectAdapterPtr reverseAdapter; // For reverse communications using the adapter's incoming connections.
    const bool collocationOptimization;
    const Ice::Int hashValue;

    //
    // Get a new reference, based on the existing one, overwriting
    // certain values.
    //
    ReferencePtr changeIdentity(const Ice::Identity&) const;
    ReferencePtr changeContext(const Ice::Context&) const;
    ReferencePtr changeFacet(const Ice::FacetPath&) const;
    ReferencePtr changeTimeout(int) const;
    ReferencePtr changeMode(Mode) const;
    ReferencePtr changeSecure(bool) const;
    ReferencePtr changeCompress(bool) const;
    ReferencePtr changeAdapterId(const std::string&) const;
    ReferencePtr changeEndpoints(const std::vector<EndpointPtr>&) const;
    ReferencePtr changeRouter(const ::Ice::RouterPrx&) const;
    ReferencePtr changeLocator(const ::Ice::LocatorPrx&) const;
    ReferencePtr changeCollocationOptimization(bool) const;
    ReferencePtr changeDefault() const;
 
private:

    Reference(const InstancePtr&, const Ice::Identity&, const Ice::Context&, const Ice::FacetPath&, Mode, bool,
	      const std::string&, const std::vector<EndpointPtr>&,
	      const RouterInfoPtr&, const LocatorInfoPtr&, const Ice::ObjectAdapterPtr&, bool);
    friend class ReferenceFactory;
};

}

#endif
