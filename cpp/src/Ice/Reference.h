// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_REFERENCE_H
#define ICE_REFERENCE_H

#include <IceUtil/Shared.h>
#include <IceUtil/RecMutex.h>
#include <Ice/ReferenceF.h>
#include <Ice/ReferenceFactoryF.h>
#include <Ice/EndpointIF.h>
#include <Ice/InstanceF.h>
#include <Ice/RouterF.h>
#include <Ice/LocatorF.h>
#include <Ice/RouterInfoF.h>
#include <Ice/LocatorInfoF.h>
#include <Ice/ConnectionIF.h>
#include <Ice/SharedContext.h>
#include <Ice/Identity.h>

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
    const SharedContextPtr& getContext() const { return _context; }

    ReferencePtr defaultContext() const;

    Ice::CommunicatorPtr getCommunicator() const;

    virtual RouterInfoPtr getRouterInfo() const { return 0; }
    virtual LocatorInfoPtr getLocatorInfo() const { return 0; }

    virtual Type getType() const = 0;
    virtual bool getSecure() const = 0;
    virtual bool getPreferSecure() const = 0;
    virtual std::string getAdapterId() const = 0;
    virtual std::vector<EndpointIPtr> getEndpoints() const = 0;
    virtual bool getCollocationOptimization() const = 0;
    virtual int getLocatorCacheTimeout() const = 0;
    virtual bool getCacheConnection() const = 0;
    virtual Ice::EndpointSelectionType getEndpointSelection() const = 0;
    virtual bool getThreadPerConnection() const = 0;

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
    virtual ReferencePtr changePreferSecure(bool) const = 0;
    virtual ReferencePtr changeRouter(const Ice::RouterPrx&) const = 0;
    virtual ReferencePtr changeLocator(const Ice::LocatorPrx&) const = 0;
    virtual ReferencePtr changeCompress(bool) const = 0;
    virtual ReferencePtr changeTimeout(int) const = 0;
    virtual ReferencePtr changeConnectionId(const std::string&) const = 0;
    virtual ReferencePtr changeCollocationOptimization(bool) const = 0;
    virtual ReferencePtr changeAdapterId(const std::string&) const = 0;
    virtual ReferencePtr changeEndpoints(const std::vector<EndpointIPtr>&) const = 0;
    virtual ReferencePtr changeLocatorCacheTimeout(int) const = 0;
    virtual ReferencePtr changeCacheConnection(bool) const = 0;
    virtual ReferencePtr changeEndpointSelection(Ice::EndpointSelectionType) const = 0;
    virtual ReferencePtr changeThreadPerConnection(bool) const = 0;
    
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

    Reference(const InstancePtr&, const Ice::CommunicatorPtr&, const Ice::Identity&, const SharedContextPtr&,
              const std::string&, Mode);
    Reference(const Reference&);

    IceUtil::RecMutex _hashMutex; // For lazy initialization of hash value.
    mutable Ice::Int _hashValue;
    mutable bool _hashInitialized;

private:

    const InstancePtr _instance;
    const Ice::CommunicatorPtr _communicator;

    Mode _mode;
    Ice::Identity _identity;
    SharedContextPtr _context;
    std::string _facet;
};

class FixedReference : public Reference
{
public:

    FixedReference(const InstancePtr&, const Ice::CommunicatorPtr&, const Ice::Identity&, const SharedContextPtr&, 
                   const std::string&, Mode, const std::vector<Ice::ConnectionIPtr>&);

    virtual Type getType() const;
    virtual bool getSecure() const;
    virtual bool getPreferSecure() const;
    virtual std::string getAdapterId() const;
    virtual std::vector<EndpointIPtr> getEndpoints() const;
    virtual bool getCollocationOptimization() const;
    virtual int getLocatorCacheTimeout() const;
    virtual bool getCacheConnection() const;
    virtual Ice::EndpointSelectionType getEndpointSelection() const;
    virtual bool getThreadPerConnection() const;

    virtual ReferencePtr changeSecure(bool) const;
    virtual ReferencePtr changePreferSecure(bool) const;
    virtual ReferencePtr changeRouter(const Ice::RouterPrx&) const;
    virtual ReferencePtr changeLocator(const Ice::LocatorPrx&) const;
    virtual ReferencePtr changeCollocationOptimization(bool) const;
    virtual ReferencePtr changeCompress(bool) const;
    virtual ReferencePtr changeTimeout(int) const;
    virtual ReferencePtr changeConnectionId(const std::string&) const;
    virtual ReferencePtr changeLocatorCacheTimeout(int) const;
    virtual ReferencePtr changeAdapterId(const std::string&) const;
    virtual ReferencePtr changeEndpoints(const std::vector<EndpointIPtr>&) const;
    virtual ReferencePtr changeCacheConnection(bool) const;
    virtual ReferencePtr changeEndpointSelection(Ice::EndpointSelectionType) const;
    virtual ReferencePtr changeThreadPerConnection(bool) const;

    virtual void streamWrite(BasicStream*) const;
    virtual std::string toString() const;

    virtual Ice::ConnectionIPtr getConnection(bool&) const;

    virtual bool operator==(const Reference&) const;
    virtual bool operator!=(const Reference&) const;
    virtual bool operator<(const Reference&) const;

    virtual ReferencePtr clone() const;

protected:

    FixedReference(const FixedReference&);

    std::vector<Ice::ConnectionIPtr> filterConnections(const std::vector<Ice::ConnectionIPtr>&) const;

private:

    std::vector<Ice::ConnectionIPtr> _fixedConnections;
};

class RoutableReference : public Reference
{
public:

    virtual RouterInfoPtr getRouterInfo() const { return _routerInfo; }
    std::vector<EndpointIPtr> getRoutedEndpoints() const;

    virtual bool getSecure() const;
    virtual bool getPreferSecure() const;
    virtual bool getCollocationOptimization() const;
    virtual bool getCacheConnection() const;
    virtual Ice::EndpointSelectionType getEndpointSelection() const;
    virtual bool getThreadPerConnection() const;

    virtual ReferencePtr changeSecure(bool) const;
    virtual ReferencePtr changePreferSecure(bool) const;
    virtual ReferencePtr changeRouter(const Ice::RouterPrx&) const;
    virtual ReferencePtr changeCollocationOptimization(bool) const;
    virtual ReferencePtr changeCompress(bool) const;
    virtual ReferencePtr changeTimeout(int) const;
    virtual ReferencePtr changeConnectionId(const std::string&) const;
    virtual ReferencePtr changeCacheConnection(bool) const;
    virtual ReferencePtr changeEndpointSelection(Ice::EndpointSelectionType) const;
    virtual ReferencePtr changeThreadPerConnection(bool) const;

    virtual Ice::ConnectionIPtr getConnection(bool&) const = 0;

    virtual int hash() const;

    virtual bool operator==(const Reference&) const = 0;
    virtual bool operator!=(const Reference&) const = 0;
    virtual bool operator<(const Reference&) const = 0;

    virtual ReferencePtr clone() const = 0;

protected:

    RoutableReference(const InstancePtr&, const Ice::CommunicatorPtr&, const Ice::Identity&, const SharedContextPtr&,
                      const std::string&, Mode, bool, bool, const RouterInfoPtr&, bool, bool,
                      Ice::EndpointSelectionType, bool);
    RoutableReference(const RoutableReference&);

    Ice::ConnectionIPtr createConnection(const std::vector<EndpointIPtr>&, bool&) const;
    void applyOverrides(std::vector<EndpointIPtr>&) const;

private:

    bool _secure;
    bool _preferSecure;
    RouterInfoPtr _routerInfo; // Null if no router is used.
    bool _collocationOptimization;
    bool _cacheConnection;
    Ice::EndpointSelectionType _endpointSelection;

    std::string _connectionId;
    bool _overrideCompress;
    bool _compress; // Only used if _overrideCompress == true
    bool _overrideTimeout;
    int _timeout; // Only used if _overrideTimeout == true
    bool _threadPerConnection;
};

class DirectReference : public RoutableReference
{
public:

    DirectReference(const InstancePtr&, const Ice::CommunicatorPtr&, const Ice::Identity&, const SharedContextPtr&,
                    const std::string&, Mode, bool, bool, const std::vector<EndpointIPtr>&, const RouterInfoPtr&, bool,
                    bool, Ice::EndpointSelectionType, bool);

    virtual Type getType() const;
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

    IndirectReference(const InstancePtr&, const Ice::CommunicatorPtr&, const Ice::Identity&, const SharedContextPtr&, 
                      const std::string&, Mode, bool, bool, const std::string&, const RouterInfoPtr&,
                      const LocatorInfoPtr&, bool, bool, Ice::EndpointSelectionType, bool, int);

    virtual LocatorInfoPtr getLocatorInfo() const { return _locatorInfo; }

    virtual Type getType() const;
    virtual int getLocatorCacheTimeout() const;
    virtual std::string getAdapterId() const;
    virtual std::vector<EndpointIPtr> getEndpoints() const;

    virtual ReferencePtr changeLocator(const Ice::LocatorPrx&) const;
    virtual ReferencePtr changeLocatorCacheTimeout(int) const;
    virtual ReferencePtr changeAdapterId(const std::string&) const;
    virtual ReferencePtr changeEndpoints(const std::vector<EndpointIPtr>&) const;

    virtual void streamWrite(BasicStream*) const;
    virtual std::string toString() const;
    virtual Ice::ConnectionIPtr getConnection(bool&) const;

    virtual int hash() const; // Conceptually const.

    virtual bool operator==(const Reference&) const;
    virtual bool operator!=(const Reference&) const;
    virtual bool operator<(const Reference&) const;

    virtual ReferencePtr clone() const;

protected:

    IndirectReference(const IndirectReference&);

private:

    std::string _adapterId;
    LocatorInfoPtr _locatorInfo;
    int _locatorCacheTimeout;
};

}

#endif
