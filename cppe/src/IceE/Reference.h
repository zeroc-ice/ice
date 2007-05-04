// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_REFERENCE_H
#define ICEE_REFERENCE_H

#include <IceE/ReferenceF.h>
#include <IceE/EndpointF.h>
#include <IceE/InstanceF.h>
#include <IceE/CommunicatorF.h>
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
#include <IceE/RecMutex.h>
#include <IceE/Identity.h>

namespace IceInternal
{

class BasicStream;

class Reference : public IceUtil::Shared
{
public:

    enum Type
    {
	TypeDirect,
	TypeIndirect,
	TypeFixed
    };

    //
    // The reference mode in Ice-E is defined in ReferenceF.h to allow the proxy
    // to inline methods such as ice_twoway, ice_isTwoway, etc.
    //
//     enum Mode
//     {
// 	ModeTwoway,
// 	ModeOneway,
// 	ModeBatchOneway,
// 	ModeDatagram,
// 	ModeBatchDatagram,
// 	ModeLast = ModeBatchDatagram
//     };

    ReferenceMode getMode() const { return _mode; }
    bool getSecure() const { return _secure; };
    const Ice::Identity& getIdentity() const { return _identity; }
    const std::string& getFacet() const { return _facet; }
    const InstancePtr& getInstance() const { return _instance; }
    const Ice::Context* getContext() const { return &_context; }

    Ice::CommunicatorPtr getCommunicator() const;

    virtual Type getType() const = 0;
    virtual std::vector<EndpointPtr> getEndpoints() const = 0;

#ifdef ICEE_HAS_ROUTER
    virtual RouterInfoPtr getRouterInfo() const { return 0; }
#endif
#ifdef ICEE_HAS_LOCATOR
    virtual std::string getAdapterId() const = 0;
    virtual LocatorInfoPtr getLocatorInfo() const { return 0; }
#endif

    //
    // The change* methods (here and in derived classes) create
    // a new reference based on the existing one, with the
    // corresponding value changed.
    //
    ReferencePtr changeContext(const Ice::Context&) const;
    ReferencePtr changeMode(ReferenceMode) const;
    ReferencePtr changeSecure(bool) const;
    ReferencePtr changeIdentity(const Ice::Identity&) const;
    ReferencePtr changeFacet(const std::string&) const;

#ifdef ICEE_HAS_ROUTER
    virtual ReferencePtr changeRouter(const Ice::RouterPrx&) const = 0;
#endif
#ifdef ICEE_HAS_LOCATOR
    virtual ReferencePtr changeAdapterId(const std::string&) const = 0;
    virtual ReferencePtr changeLocator(const Ice::LocatorPrx&) const = 0;
#endif
    virtual ReferencePtr changeTimeout(int) const;

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
    virtual bool operator<(const Reference&) const = 0;

    virtual ReferencePtr clone() const = 0;

protected:

    Reference(const InstancePtr&, const Ice::CommunicatorPtr&, const Ice::Identity&, const Ice::Context&,
	      const std::string&, ReferenceMode, bool);
    Reference(const Reference&);

    void applyOverrides(std::vector<EndpointPtr>&) const;

    IceUtil::RecMutex _hashMutex; // For lazy initialization of hash value.
    mutable Ice::Int _hashValue;
    mutable bool _hashInitialized;

private:

    const InstancePtr _instance;
    const Ice::CommunicatorPtr _communicator;

    ReferenceMode _mode;
    bool _secure;
    Ice::Identity _identity;
    Ice::Context _context;
    std::string _facet;

    //
    // NOTE: The override timeout should theoritically be in
    // RoutableReference. However, since RoutableReference is only
    // defined if the ICEE_HAS_ROUTER macro is defined, it would also
    // have to be conditionally defined here. To simplify, we just 
    // define it here.
    //
    bool _overrideTimeout;
    int _timeout; // Only used if _overrideTimeout == true
};

class FixedReference : public Reference
{
public:

    FixedReference(const InstancePtr&, const Ice::CommunicatorPtr&, const Ice::Identity&, const Ice::Context&,
		   const std::string&, ReferenceMode, const std::vector<Ice::ConnectionPtr>&);

    virtual Type getType() const;
    virtual std::vector<EndpointPtr> getEndpoints() const;

#ifdef ICEE_HAS_ROUTER
    virtual ReferencePtr changeRouter(const Ice::RouterPrx&) const;
#endif
#ifdef ICEE_HAS_LOCATOR
    virtual std::string getAdapterId() const;
    virtual ReferencePtr changeAdapterId(const std::string&) const;
    virtual ReferencePtr changeLocator(const Ice::LocatorPrx&) const;
#endif
    virtual ReferencePtr changeTimeout(int) const;

    virtual void streamWrite(BasicStream*) const;
    virtual std::string toString() const;

    virtual Ice::ConnectionPtr getConnection() const;

    virtual bool operator==(const Reference&) const;
    virtual bool operator<(const Reference&) const;

    virtual ReferencePtr clone() const;

protected:

    FixedReference(const FixedReference&);

private:

    std::vector<Ice::ConnectionPtr> _fixedConnections;
};

#ifdef ICEE_HAS_ROUTER
class RoutableReference : public Reference
{
public:

    virtual RouterInfoPtr getRouterInfo() const { return _routerInfo; }
    std::vector<EndpointPtr> getRoutedEndpoints() const;

    virtual ReferencePtr changeRouter(const Ice::RouterPrx&) const;

    virtual Ice::ConnectionPtr getConnection() const = 0;

    int hash() const; // Conceptually const.

    virtual bool operator==(const Reference&) const = 0;
    virtual bool operator<(const Reference&) const = 0;

    virtual ReferencePtr clone() const = 0;

protected:

    RoutableReference(const InstancePtr&, const Ice::CommunicatorPtr&, const Ice::Identity&, const Ice::Context&,
		      const std::string&, ReferenceMode, bool, const RouterInfoPtr&);
    RoutableReference(const RoutableReference&);


private:

    RouterInfoPtr _routerInfo; // Null if no router is used.
};
#endif

class DirectReference :
#ifdef ICEE_HAS_ROUTER
    public RoutableReference
#else
    public Reference
#endif
{
public:

    DirectReference(const InstancePtr&, const Ice::CommunicatorPtr&, const Ice::Identity&, const Ice::Context&,
		    const std::string&, ReferenceMode, bool, const std::vector<EndpointPtr>&
#ifdef ICEE_HAS_ROUTER
		    , const RouterInfoPtr&
#endif
		    );

    virtual Type getType() const;
    virtual std::vector<EndpointPtr> getEndpoints() const;

#ifdef ICEE_HAS_LOCATOR
    virtual std::string getAdapterId() const;
    virtual ReferencePtr changeAdapterId(const std::string&) const;
    virtual ReferencePtr changeLocator(const Ice::LocatorPrx&) const;
#endif
    virtual ReferencePtr changeTimeout(int) const;

    virtual void streamWrite(BasicStream*) const;
    virtual std::string toString() const;
    virtual Ice::ConnectionPtr getConnection() const;

    virtual bool operator==(const Reference&) const;
    virtual bool operator<(const Reference&) const;

    virtual ReferencePtr clone() const;

protected:

    DirectReference(const DirectReference&);

private:

    std::vector<EndpointPtr> _endpoints;

#ifdef ICEE_HAS_ROUTER
    typedef RoutableReference Parent;
#else
    typedef Reference Parent;
#endif
};

#ifdef ICEE_HAS_LOCATOR

class IndirectReference :
#ifdef ICEE_HAS_ROUTER
    public RoutableReference
#else
    public Reference
#endif
{
public:

    IndirectReference(const InstancePtr&, const Ice::CommunicatorPtr&, const Ice::Identity&, const Ice::Context&,
		      const std::string&, ReferenceMode, bool, const std::string&
#ifdef ICEE_HAS_ROUTER
		      , const RouterInfoPtr&
#endif
		      , const LocatorInfoPtr&);

    virtual LocatorInfoPtr getLocatorInfo() const { return _locatorInfo; }

    virtual Type getType() const;
    virtual std::vector<EndpointPtr> getEndpoints() const;
    virtual std::string getAdapterId() const;

    virtual ReferencePtr changeAdapterId(const std::string&) const;
    virtual ReferencePtr changeLocator(const Ice::LocatorPrx&) const;

    virtual void streamWrite(BasicStream*) const;
    virtual std::string toString() const;
    virtual Ice::ConnectionPtr getConnection() const;

    int hash() const; // Conceptually const.

    virtual bool operator==(const Reference&) const;
    virtual bool operator<(const Reference&) const;

    virtual ReferencePtr clone() const;

protected:

    IndirectReference(const IndirectReference&);

private:

    std::string _adapterId;
    LocatorInfoPtr _locatorInfo;
#ifdef ICEE_HAS_ROUTER
    typedef RoutableReference Parent;
#else
    typedef Reference Parent;
#endif
};

#endif // ICEE_HAS_LOCATOR

std::vector<EndpointPtr> filterEndpoints(const std::vector<EndpointPtr>&, ReferenceMode, bool);

}

#endif
