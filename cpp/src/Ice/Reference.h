// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distirbution.
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
#include <Ice/ConnectionIF.h>
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
    // Marshal the reference.
    //
    void streamWrite(BasicStream*) const;

    //
    // Convert the reference to its string form.
    //
    std::string toString() const;

    //
    // All  members are const, because References are immutable.
    //
    const InstancePtr instance;
    const Ice::Identity identity;
    const Ice::Context context;
    const std::string facet;
    const Mode mode;
    const bool secure;
    const std::string adapterId;
    const std::vector<EndpointPtr> endpoints;
    const RouterInfoPtr routerInfo; // Null if no router is used.
    const LocatorInfoPtr locatorInfo; // Null if no locator is used.
    const std::vector<Ice::ConnectionIPtr> fixedConnections; // For using fixed connections, otherwise empty.
    const bool collocationOptimization;
    const Ice::Int hashValue;

    //
    // Get a new reference, based on the existing one, overwriting
    // certain values.
    //
    ReferencePtr changeIdentity(const Ice::Identity&) const;
    ReferencePtr changeContext(const Ice::Context&) const;
    ReferencePtr changeFacet(const std::string&) const;
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
 
    //
    // Get a suitable connection for this reference.
    //
    Ice::ConnectionIPtr getConnection(bool&) const;

    //
    // Filter endpoints or connections based on criteria from this reference.
    //
    std::vector<EndpointPtr> filterEndpoints(const std::vector<EndpointPtr>&) const;
    std::vector<Ice::ConnectionIPtr> filterConnections(const std::vector<Ice::ConnectionIPtr>&) const;

private:

    Reference(const InstancePtr&, const Ice::Identity&, const Ice::Context&, const std::string&, Mode, bool,
	      const std::string&, const std::vector<EndpointPtr>&,
	      const RouterInfoPtr&, const LocatorInfoPtr&, const std::vector<Ice::ConnectionIPtr>&, bool);
    friend class ReferenceFactory;
};

}

#endif
