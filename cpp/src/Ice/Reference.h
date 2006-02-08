// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_REFERENCE_H
#define ICE_REFERENCE_H

#include <IceUtil/Shared.h>
#include <IceUtil/Mutex.h>
#include <Ice/ReferenceF.h>
#include <Ice/ReferenceFactoryF.h>
#include <Ice/EndpointIF.h>
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

    Mode getMode() const { return _mode; }
    const Ice::Identity& getIdentity() const { return _identity; }
    const std::string& getFacet() const { return _facet; }
    const InstancePtr& getInstance() const { return _instance; }
    const Ice::Context& getContext() const;

    ReferencePtr defaultContext() const;

    Ice::CommunicatorPtr getCommunicator() const;

    virtual bool getSecure() const = 0;
    virtual std::string getAdapterId() const = 0;
    virtual std::vector<EndpointIPtr> getEndpoints() const = 0;
    virtual bool getCollocationOptimization() const = 0;
    virtual int getLocatorCacheTimeout() const = 0;

    //
    // The change* methods (here and in derived classes) create
    // a new reference based on the existing one, with the
    // corresponding value changed.
    //
    ReferencePtr changeContext(const Ice::Context&) const;
    ReferencePtr changeMode(Mode) const;
    ReferencePtr changeIdentity(const Ice::Identity&) const;
    ReferencePtr changeFacet(const std::string&) const;

    virtual ReferencePtr changeSecure(bool) const = 0;
    virtual ReferencePtr changeRouter(const Ice::RouterPrx&) const = 0;
    virtual ReferencePtr changeLocator(const Ice::LocatorPrx&) const = 0;
    virtual ReferencePtr changeCompress(bool) const = 0;
    virtual ReferencePtr changeTimeout(int) const = 0;
    virtual ReferencePtr changeConnectionId(const std::string&) const = 0;
    virtual ReferencePtr changeCollocationOptimization(bool) const = 0;
    virtual ReferencePtr changeAdapterId(const std::string&) const = 0;
    virtual ReferencePtr changeEndpoints(const std::vector<EndpointIPtr>&) const = 0;
    virtual ReferencePtr changeLocatorCacheTimeout(int) const = 0;
    
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
    virtual Ice::ConnectionIPtr getConnection(bool&) const = 0;

    virtual bool operator==(const Reference&) const = 0;
    virtual bool operator!=(const Reference&) const = 0;
    virtual bool operator<(const Reference&) const = 0;

    virtual ReferencePtr clone() const = 0;

protected:

    Reference(const InstancePtr&, const Ice::CommunicatorPtr&, const Ice::Identity&, const Ice::Context&,
	      const std::string&, Mode);
    Reference(const Reference&);

private:

    const InstancePtr _instance;
    const Ice::CommunicatorPtr _communicator;

    Mode _mode;
    Ice::Identity _identity;
    Ice::Context _context;
    std::string _facet;
    static std::string _emptyAdapterId;

    IceUtil::Mutex _hashMutex; // For lazy initialization of hash value.
    mutable Ice::Int _hashValue;
    mutable bool _hashInitialized;
};

class FixedReference : public Reference
{
public:

    FixedReference(const InstancePtr&, const Ice::CommunicatorPtr&, const Ice::Identity&, const Ice::Context&, 
		   const std::string&, Mode, const std::vector<Ice::ConnectionIPtr>&);

    const std::vector<Ice::ConnectionIPtr>& getFixedConnections() const;

    virtual bool getSecure() const;
    virtual int getLocatorCacheTimeout() const;
    virtual std::string getAdapterId() const;
    virtual std::vector<EndpointIPtr> getEndpoints() const;
    virtual bool getCollocationOptimization() const;

    virtual ReferencePtr changeSecure(bool) const;
    virtual ReferencePtr changeRouter(const Ice::RouterPrx&) const;
    virtual ReferencePtr changeLocator(const Ice::LocatorPrx&) const;
    virtual ReferencePtr changeCollocationOptimization(bool) const;
    virtual ReferencePtr changeCompress(bool) const;
    virtual ReferencePtr changeTimeout(int) const;
    virtual ReferencePtr changeConnectionId(const std::string&) const;
    virtual ReferencePtr changeLocatorCacheTimeout(int) const;
    virtual ReferencePtr changeAdapterId(const std::string&) const;
    virtual ReferencePtr changeEndpoints(const std::vector<EndpointIPtr>&) const;

    virtual void streamWrite(BasicStream*) const;
    virtual std::string toString() const;

    virtual Ice::ConnectionIPtr getConnection(bool&) const;

    virtual bool operator==(const Reference&) const;
    virtual bool operator!=(const Reference&) const;
    virtual bool operator<(const Reference&) const;

    virtual ReferencePtr clone() const;

protected:

    FixedReference(const FixedReference&);

private:

    std::vector<Ice::ConnectionIPtr> _fixedConnections;
};

class RoutableReference : public Reference
{
public:

    const RouterInfoPtr& getRouterInfo() const { return _routerInfo; }
    std::vector<EndpointIPtr> getRoutedEndpoints() const;

    virtual bool getSecure() const;
    virtual bool getCollocationOptimization() const;

    virtual ReferencePtr changeSecure(bool) const;
    virtual ReferencePtr changeRouter(const Ice::RouterPrx&) const;
    virtual ReferencePtr changeCollocationOptimization(bool) const;

    virtual Ice::ConnectionIPtr getConnection(bool&) const = 0;

    virtual bool operator==(const Reference&) const = 0;
    virtual bool operator!=(const Reference&) const = 0;
    virtual bool operator<(const Reference&) const = 0;

    virtual ReferencePtr clone() const = 0;

protected:

    RoutableReference(const InstancePtr&, const Ice::CommunicatorPtr&, const Ice::Identity&, const Ice::Context&,
		      const std::string&, Mode, bool, const RouterInfoPtr&, bool);
    RoutableReference(const RoutableReference&);


private:

    bool _secure;
    RouterInfoPtr _routerInfo; // Null if no router is used.
    bool _collocationOptimization;
};

class DirectReference : public RoutableReference
{
public:

    DirectReference(const InstancePtr&, const Ice::CommunicatorPtr&, const Ice::Identity&, const Ice::Context&,
		    const std::string&, Mode, bool, const std::vector<EndpointIPtr>&, const RouterInfoPtr&, bool);

    virtual int getLocatorCacheTimeout() const;
    virtual std::string getAdapterId() const;
    virtual std::vector<EndpointIPtr> getEndpoints() const;

    virtual ReferencePtr changeLocator(const Ice::LocatorPrx&) const;
    virtual ReferencePtr changeCompress(bool) const;
    virtual ReferencePtr changeTimeout(int) const;
    virtual ReferencePtr changeConnectionId(const std::string&) const;
    virtual ReferencePtr changeLocatorCacheTimeout(int) const;
    virtual ReferencePtr changeAdapterId(const std::string&) const;
    virtual ReferencePtr changeEndpoints(const std::vector<EndpointIPtr>&) const;

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

    std::vector<EndpointIPtr> _endpoints;
};

class IndirectReference : public RoutableReference
{
public:

    IndirectReference(const InstancePtr&, const Ice::CommunicatorPtr&, const Ice::Identity&, const Ice::Context&, 
		      const std::string&, Mode, bool, const std::string&, const RouterInfoPtr&, const LocatorInfoPtr&,
		      bool, int);

    const LocatorInfoPtr& getLocatorInfo() const { return _locatorInfo; }

    virtual int getLocatorCacheTimeout() const;
    virtual std::string getAdapterId() const;
    virtual std::vector<EndpointIPtr> getEndpoints() const;

    virtual ReferencePtr changeLocator(const Ice::LocatorPrx&) const;
    virtual ReferencePtr changeCompress(bool) const;
    virtual ReferencePtr changeTimeout(int) const;
    virtual ReferencePtr changeConnectionId(const std::string&) const;
    virtual ReferencePtr changeLocatorCacheTimeout(int) const;
    virtual ReferencePtr changeAdapterId(const std::string&) const;
    virtual ReferencePtr changeEndpoints(const std::vector<EndpointIPtr>&) const;

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

    std::string _adapterId;
    std::string _connectionId;
    LocatorInfoPtr _locatorInfo;
    int _locatorCacheTimeout;
};

std::vector<EndpointIPtr> filterEndpoints(const std::vector<EndpointIPtr>&, Reference::Mode, bool);
std::vector<Ice::ConnectionIPtr> filterConnections(const std::vector<Ice::ConnectionIPtr>&, Reference::Mode, bool);

}

#endif
