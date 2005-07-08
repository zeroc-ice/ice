// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_REFERENCE_H
#define ICEE_REFERENCE_H

#include <IceE/ReferenceF.h>
#include <IceE/EndpointF.h>
#include <IceE/InstanceF.h>
#ifdef ICEE_HAS_ROUTER
#  include <IceE/RouterInfoF.h>
#  include <IceE/RouterF.h>
#endif
#ifdef ICEE_HAS_LOCATOR
#  include <IceE/LocatorInfoF.h>
#  include <IceE/LocatorF.h>
#endif
#include <IceE/ConnectionF.h>
#include <IceE/Shared.h>
#include <IceE/Mutex.h>
#include <IceE/Identity.h>

namespace IceInternal
{

class BasicStream;

class Reference : public Ice::Shared
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

    Mode getMode() const { return _mode; }
    const Ice::Identity& getIdentity() const { return _identity; }
    const std::string& getFacet() const { return _facet; }
    const InstancePtr& getInstance() const { return _instance; }
    const Ice::Context& getContext() const;

    ReferencePtr defaultContext() const;

    virtual std::vector<EndpointPtr> getEndpoints() const = 0;

    //
    // The change* methods (here and in derived classes) create
    // a new reference based on the existing one, with the
    // corresponding value changed.
    //
    ReferencePtr changeContext(const Ice::Context&) const;
    ReferencePtr changeMode(Mode) const;
    ReferencePtr changeIdentity(const Ice::Identity&) const;
    bool hasContext() const { return _hasContext; }
    ReferencePtr changeFacet(const std::string&) const;

    //
    // Return a reference in the default configuration.
    //
    virtual ReferencePtr changeDefault() const;

#ifdef ICEE_HAS_ROUTER
    virtual ReferencePtr changeRouter(const Ice::RouterPrx&) const = 0;
#endif
#ifdef ICEE_HAS_LOCATOR
    virtual ReferencePtr changeLocator(const Ice::LocatorPrx&) const = 0;
#endif
    virtual ReferencePtr changeTimeout(int) const = 0;

    int hash() const; // Conceptually const.

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
    virtual Ice::ConnectionPtr getConnection() const = 0;

    virtual bool operator==(const Reference&) const = 0;
    virtual bool operator!=(const Reference&) const = 0;
    virtual bool operator<(const Reference&) const = 0;

    virtual ReferencePtr clone() const = 0;

protected:

    Reference(const InstancePtr&, const Ice::Identity&, const Ice::Context&, const std::string&, Mode);
    Reference(const Reference&);

private:

    InstancePtr _instance;

    Mode _mode;
    Ice::Identity _identity;
    bool _hasContext;
    Ice::Context _context;
    std::string _facet;

    Ice::Mutex _hashMutex; // For lazy initialization of hash value.
    mutable Ice::Int _hashValue;
    mutable bool _hashInitialized;
};

class FixedReference : public Reference
{
public:

    FixedReference(const InstancePtr&, const Ice::Identity&, const Ice::Context&, const std::string&, Mode,
	           const std::vector<Ice::ConnectionPtr>&);

    const std::vector<Ice::ConnectionPtr>& getFixedConnections() const;

    virtual std::vector<EndpointPtr> getEndpoints() const;

#ifdef ICEE_HAS_ROUTER
    virtual ReferencePtr changeRouter(const Ice::RouterPrx&) const;
#endif
#ifdef ICEE_HAS_LOCATOR
    virtual ReferencePtr changeLocator(const Ice::LocatorPrx&) const;
#endif
    virtual ReferencePtr changeTimeout(int) const;

    virtual void streamWrite(BasicStream*) const;

    virtual Ice::ConnectionPtr getConnection() const;

    virtual bool operator==(const Reference&) const;
    virtual bool operator!=(const Reference&) const;
    virtual bool operator<(const Reference&) const;

    virtual ReferencePtr clone() const;

protected:

    FixedReference(const FixedReference&);

private:

    std::vector<Ice::ConnectionPtr> _fixedConnections;
};

class RoutableReference : public Reference
{
public:

#ifdef ICEE_HAS_ROUTER
    const RouterInfoPtr& getRouterInfo() const { return _routerInfo; }
    std::vector<EndpointPtr> getRoutedEndpoints() const;

    virtual ReferencePtr changeDefault() const;

    virtual ReferencePtr changeRouter(const Ice::RouterPrx&) const;
#endif

    virtual Ice::ConnectionPtr getConnection() const = 0;

    virtual bool operator==(const Reference&) const = 0;
    virtual bool operator!=(const Reference&) const = 0;
    virtual bool operator<(const Reference&) const = 0;

    virtual ReferencePtr clone() const = 0;

protected:

    RoutableReference(const InstancePtr&, const Ice::Identity&, const Ice::Context&, const std::string&, Mode
#ifdef ICEE_HAS_ROUTER
		      , const RouterInfoPtr&
#endif
		      );
    RoutableReference(const RoutableReference&);


private:

#ifdef ICEE_HAS_ROUTER
    RouterInfoPtr _routerInfo; // Null if no router is used.
#endif
};

class DirectReference : public RoutableReference
{
public:

    DirectReference(const InstancePtr&, const Ice::Identity&, const Ice::Context&, const std::string&, Mode,
	            const std::vector<EndpointPtr>&
#ifdef ICEE_HAS_ROUTER
		    , const RouterInfoPtr&
#endif
		    );

    virtual std::vector<EndpointPtr> getEndpoints() const;

    DirectReferencePtr changeEndpoints(const std::vector<EndpointPtr>&) const;

    virtual ReferencePtr changeDefault() const;

#ifdef ICEE_HAS_LOCATOR
    virtual ReferencePtr changeLocator(const Ice::LocatorPrx&) const;
#endif
    virtual ReferencePtr changeTimeout(int) const;

    virtual void streamWrite(BasicStream*) const;
    virtual std::string toString() const;
    virtual Ice::ConnectionPtr getConnection() const;

    virtual bool operator==(const Reference&) const;
    virtual bool operator!=(const Reference&) const;
    virtual bool operator<(const Reference&) const;

    virtual ReferencePtr clone() const;

protected:

    DirectReference(const DirectReference&);

private:

    std::vector<EndpointPtr> _endpoints;
};

#ifdef ICEE_HAS_LOCATOR

class IndirectReference : public RoutableReference
{
public:

    IndirectReference(const InstancePtr&, const Ice::Identity&, const Ice::Context&, const std::string&,
                      Mode, const std::string&
#ifdef ICEE_HAS_ROUTER
		      , const RouterInfoPtr&
#endif
		      , const LocatorInfoPtr&);

    const std::string& getAdapterId() const { return _adapterId; }
    const LocatorInfoPtr& getLocatorInfo() const { return _locatorInfo; }

    virtual std::vector<EndpointPtr> getEndpoints() const;

    virtual ReferencePtr changeDefault() const;

    virtual ReferencePtr changeLocator(const Ice::LocatorPrx&) const;
    virtual ReferencePtr changeTimeout(int) const;

    virtual void streamWrite(BasicStream*) const;
    virtual std::string toString() const;
    virtual Ice::ConnectionPtr getConnection() const;

    virtual bool operator==(const Reference&) const;
    virtual bool operator!=(const Reference&) const;
    virtual bool operator<(const Reference&) const;

    virtual ReferencePtr clone() const;

protected:

    IndirectReference(const IndirectReference&);

private:

    std::string _adapterId;
    LocatorInfoPtr _locatorInfo;
};

#endif // ICEE_HAS_LOCATOR

std::vector<EndpointPtr> filterEndpoints(const std::vector<EndpointPtr>&);

}

#endif
