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
#include <IceUtil/Mutex.h>
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

class Reference : public IceUtil::Shared
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

    Mode getMode() const { return mode; }
    const Ice::Identity& getIdentity() const { return identity; }
    const Ice::Context& getContext() const { return context; }
    const std::string& getFacet() const { return facet; }
    bool getSecure() const { return secure; }
    const InstancePtr& getInstance() const { return instance; }

    virtual std::vector<EndpointPtr> getEndpoints() const = 0;
    virtual bool getCollocationOptimization() const = 0;

    //
    // The change* methods (here and in derived classes) create
    // a new reference based on the existing one, with the
    // corresponding value changed.
    //
    ReferencePtr changeMode(Mode) const;
    ReferencePtr changeIdentity(const Ice::Identity&) const;
    ReferencePtr changeContext(const Ice::Context&) const;
    ReferencePtr changeFacet(const std::string&) const;
    ReferencePtr changeSecure(bool) const;

    virtual ReferencePtr changeRouter(const Ice::RouterPrx&) const = 0;
    virtual ReferencePtr changeLocator(const Ice::LocatorPrx&) const = 0;
    virtual ReferencePtr changeDefault() const = 0;
    virtual ReferencePtr changeCompress(bool) const = 0;
    virtual ReferencePtr changeTimeout(int) const = 0;
    virtual ReferencePtr changeCollocationOptimization(bool) const = 0;

    virtual int hash() const; // Conceptually const.

    //
    // Marshal the reference.
    //
    virtual void streamWrite(BasicStream*) const;

    //
    // Convert the reference to its string form.
    //
    virtual std::string toString() const;

    //
    // Get a suitable connection for this reference.
    //
    virtual Ice::ConnectionIPtr getConnection(bool&) const = 0;

    virtual bool operator==(const Reference&) const = 0;
    virtual bool operator!=(const Reference&) const = 0;
    virtual bool operator<(const Reference&) const = 0;

    virtual ReferencePtr clone() const = 0;

protected:

    Reference(const InstancePtr&, const Ice::Identity&, const Ice::Context&, const std::string&, Mode, bool);
    Reference(const Reference&);

private:

    InstancePtr instance;

    Mode mode;
    Ice::Identity identity;
    Ice::Context context;
    std::string facet;
    bool secure;

    IceUtil::Mutex hashMutex; // For lazy initialization of hash value.
    mutable Ice::Int hashValue;
    mutable bool hashInitialized;
};

class FixedReference : public Reference
{
public:

    FixedReference(const InstancePtr&, const Ice::Identity&, const Ice::Context&, const std::string&, Mode, bool,
	           const std::vector<Ice::ConnectionIPtr>&);

    const std::vector<Ice::ConnectionIPtr>& getFixedConnections() const { return fixedConnections; }

    virtual std::vector<EndpointPtr> getEndpoints() const;

    virtual bool getCollocationOptimization() const { return false; }

    virtual void streamWrite(BasicStream*) const;
    virtual Ice::ConnectionIPtr getConnection(bool&) const;

    virtual ReferencePtr changeRouter(const Ice::RouterPrx&) const;
    virtual ReferencePtr changeLocator(const Ice::LocatorPrx&) const;
    virtual ReferencePtr changeDefault() const;
    virtual ReferencePtr changeCollocationOptimization(bool) const;
    virtual ReferencePtr changeCompress(bool) const;
    virtual ReferencePtr changeTimeout(int) const;

    virtual bool operator==(const Reference&) const;
    virtual bool operator!=(const Reference&) const;
    virtual bool operator<(const Reference&) const;

    virtual ReferencePtr clone() const;

protected:

    FixedReference(const FixedReference&);

private:

    std::vector<Ice::ConnectionIPtr> fixedConnections;
};

class RoutableReference : public Reference
{
public:

    const RouterInfoPtr& getRouterInfo() const { return routerInfo; }
    std::vector<EndpointPtr> getRoutedEndpoints() const;
    virtual bool getCollocationOptimization() const { return collocationOptimization; }

    virtual ReferencePtr changeRouter(const Ice::RouterPrx&) const;
    virtual ReferencePtr changeDefault() const;
    virtual ReferencePtr changeCollocationOptimization(bool) const;

    virtual Ice::ConnectionIPtr getConnection(bool&) const = 0;

    virtual bool operator==(const Reference&) const = 0;
    virtual bool operator!=(const Reference&) const = 0;
    virtual bool operator<(const Reference&) const = 0;

    virtual ReferencePtr clone() const = 0;

protected:

    RoutableReference(const InstancePtr&, const Ice::Identity&, const Ice::Context&, const std::string&,
                      Mode, bool, const RouterInfoPtr&, bool);
    RoutableReference(const RoutableReference&);


private:

    bool collocationOptimization;
    RouterInfoPtr routerInfo; // Null if no router is used.
};

class DirectReference : public RoutableReference
{
public:

    DirectReference(const InstancePtr&, const Ice::Identity&, const Ice::Context&, const std::string&, Mode, bool,
	            const std::vector<EndpointPtr>&, const RouterInfoPtr&, bool);

    virtual std::vector<EndpointPtr> getEndpoints() const;

    DirectReferencePtr changeEndpoints(const std::vector<EndpointPtr>&) const;

    virtual ReferencePtr changeLocator(const Ice::LocatorPrx&) const;
    virtual ReferencePtr changeDefault() const;
    virtual ReferencePtr changeCompress(bool) const;
    virtual ReferencePtr changeTimeout(int) const;

    virtual void streamWrite(BasicStream*) const;
    virtual std::string toString() const;
    virtual Ice::ConnectionIPtr getConnection(bool&) const;

    virtual bool operator==(const Reference&) const;
    virtual bool operator!=(const Reference&) const;
    virtual bool operator<(const Reference&) const;

    virtual ReferencePtr clone() const;

protected:

    DirectReference(const DirectReference&);

private:

    std::vector<EndpointPtr> endpoints;
};

class IndirectReference : public RoutableReference
{
public:

    IndirectReference(const InstancePtr&, const Ice::Identity&, const Ice::Context&, const std::string&,
                      Mode, bool, const std::string&, const RouterInfoPtr&, const LocatorInfoPtr&, bool);

    const std::string& getAdapterId() const { return adapterId; }
    const LocatorInfoPtr& getLocatorInfo() const { return locatorInfo; }

    virtual std::vector<EndpointPtr> getEndpoints() const;

    virtual ReferencePtr changeLocator(const Ice::LocatorPrx&) const;
    virtual ReferencePtr changeDefault() const;
    virtual ReferencePtr changeCompress(bool) const;
    virtual ReferencePtr changeTimeout(int) const;

    virtual void streamWrite(BasicStream*) const;
    virtual std::string toString() const;
    virtual Ice::ConnectionIPtr getConnection(bool&) const;

    virtual bool operator==(const Reference&) const;
    virtual bool operator!=(const Reference&) const;
    virtual bool operator<(const Reference&) const;

    virtual ReferencePtr clone() const;

protected:

    IndirectReference(const IndirectReference&);

private:

    std::string adapterId;
    LocatorInfoPtr locatorInfo; // Null if no locator is used.
};

std::vector<EndpointPtr> filterEndpoints(const std::vector<EndpointPtr>&, Reference::Mode, bool);
std::vector<Ice::ConnectionIPtr> filterConnections(const std::vector<Ice::ConnectionIPtr>&, Reference::Mode, bool);

}

#endif
