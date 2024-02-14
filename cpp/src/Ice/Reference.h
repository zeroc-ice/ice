//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_REFERENCE_H
#define ICE_REFERENCE_H

#include <Ice/ReferenceF.h>
#include <Ice/ReferenceFactoryF.h>
#include <Ice/EndpointIF.h>
#include <Ice/InstanceF.h>
#include <Ice/RouterF.h>
#include <Ice/LocatorF.h>
#include <Ice/RouterInfoF.h>
#include <Ice/LocatorInfoF.h>
#include <Ice/ConnectionIF.h>
#include <Ice/RequestHandlerF.h>
#include <Ice/BatchRequestQueue.h>
#include <Ice/SharedContext.h>
#include <Ice/Identity.h>
#include <Ice/Protocol.h>
#include <Ice/Properties.h>

#include <mutex>

namespace Ice
{

class OutputStream;

}

namespace IceInternal
{

class Reference : public std::enable_shared_from_this<Reference>
{
public:

    virtual ~Reference() = default;

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
    bool isBatch() const { return _mode == ModeBatchOneway || _mode == ModeBatchDatagram; }
    bool isTwoway() const { return _mode == ModeTwoway; }
    bool getSecure() const { return _secure; }
    const Ice::ProtocolVersion& getProtocol() const { return _protocol; }
    const Ice::EncodingVersion& getEncoding() const { return _encoding; }
    const Ice::Identity& getIdentity() const { return _identity; }
    const std::string& getFacet() const { return _facet; }
    const InstancePtr& getInstance() const { return _instance; }
    const SharedContextPtr& getContext() const { return _context; }
    int getInvocationTimeout() const { return _invocationTimeout; }
    std::optional<bool> getCompress() const
    {
        return _overrideCompress ? std::optional<bool>(_compress) : std::nullopt;
    }

    Ice::CommunicatorPtr getCommunicator() const;

    virtual std::vector<EndpointIPtr> getEndpoints() const = 0;
    virtual std::string getAdapterId() const = 0;
    virtual LocatorInfoPtr getLocatorInfo() const { return 0; }
    virtual RouterInfoPtr getRouterInfo() const { return 0; }
    virtual bool getCollocationOptimized() const = 0;
    virtual bool getCacheConnection() const = 0;
    virtual bool getPreferSecure() const = 0;
    virtual Ice::EndpointSelectionType getEndpointSelection() const = 0;
    virtual int getLocatorCacheTimeout() const = 0;
    virtual std::string getConnectionId() const = 0;
    virtual std::optional<int> getTimeout() const = 0;

    //
    // The change* methods (here and in derived classes) create
    // a new reference based on the existing one, with the
    // corresponding value changed.
    //
    ReferencePtr changeContext(const Ice::Context&) const;
    ReferencePtr changeMode(Mode) const;
    ReferencePtr changeSecure(bool) const;
    ReferencePtr changeIdentity(const Ice::Identity&) const;
    ReferencePtr changeFacet(const std::string&) const;
    ReferencePtr changeInvocationTimeout(int) const;
    virtual ReferencePtr changeEncoding(const Ice::EncodingVersion&) const;
    virtual ReferencePtr changeCompress(bool) const;

    virtual ReferencePtr changeEndpoints(const std::vector<EndpointIPtr>&) const = 0;
    virtual ReferencePtr changeAdapterId(const std::string&) const = 0;
    virtual ReferencePtr changeLocator(const Ice::LocatorPrxPtr&) const = 0;
    virtual ReferencePtr changeRouter(const Ice::RouterPrxPtr&) const = 0;
    virtual ReferencePtr changeCollocationOptimized(bool) const = 0;
    virtual ReferencePtr changeLocatorCacheTimeout(int) const = 0;
    virtual ReferencePtr changeCacheConnection(bool) const = 0;
    virtual ReferencePtr changePreferSecure(bool) const = 0;
    virtual ReferencePtr changeEndpointSelection(Ice::EndpointSelectionType) const = 0;

    virtual ReferencePtr changeTimeout(int) const = 0;
    virtual ReferencePtr changeConnectionId(const std::string&) const = 0;
    virtual ReferencePtr changeConnection(const Ice::ConnectionIPtr&) const = 0;

    int hash() const; // Conceptually const.

    bool getCompressOverride(bool&) const;

    //
    // Utility methods.
    //
    virtual bool isIndirect() const = 0;
    virtual bool isWellKnown() const = 0;

    //
    // Marshal the reference.
    //
    virtual void streamWrite(Ice::OutputStream*) const;

    //
    // Convert the reference to its string form.
    //
    virtual std::string toString() const;

    //
    // Convert the reference to its property form.
    //
    virtual Ice::PropertyDict toProperty(const std::string&) const = 0;

    //
    // Get a suitable connection for this reference.
    //
    virtual RequestHandlerPtr getRequestHandler() const = 0;
    virtual BatchRequestQueuePtr getBatchRequestQueue() const = 0;

    virtual bool operator==(const Reference&) const;
    virtual bool operator<(const Reference&) const;

    virtual ReferencePtr clone() const = 0;

protected:

    Reference(const InstancePtr&, const Ice::CommunicatorPtr&, const Ice::Identity&, const std::string&, Mode, bool,
              const Ice::ProtocolVersion&, const Ice::EncodingVersion&, int, const Ice::Context& ctx);
    Reference(const Reference&);

    virtual Ice::Int hashInit() const;

    const InstancePtr _instance;
    bool _overrideCompress;
    bool _compress; // Only used if _overrideCompress == true

private:

    const Ice::CommunicatorPtr _communicator;

    Mode _mode;
    bool _secure;
    Ice::Identity _identity;
    SharedContextPtr _context;
    std::string _facet;
    mutable Ice::Int _hashValue;
    mutable bool _hashInitialized;
    mutable std::mutex _hashMutex;
    Ice::ProtocolVersion _protocol;
    Ice::EncodingVersion _encoding;
    int _invocationTimeout;
};

class FixedReference : public Reference
{
public:

    FixedReference(const InstancePtr&, const Ice::CommunicatorPtr&, const Ice::Identity&, const std::string&, Mode,
                   bool, const Ice::ProtocolVersion&, const Ice::EncodingVersion&, const Ice::ConnectionIPtr&,
                   int, const Ice::Context&, const std::optional<bool>&);

    FixedReference(const FixedReference&);

    virtual std::vector<EndpointIPtr> getEndpoints() const;
    virtual std::string getAdapterId() const;
    virtual bool getCollocationOptimized() const;
    virtual bool getCacheConnection() const;
    virtual bool getPreferSecure() const;
    virtual Ice::EndpointSelectionType getEndpointSelection() const;
    virtual int getLocatorCacheTimeout() const;
    virtual std::string getConnectionId() const;
    virtual std::optional<int> getTimeout() const;

    virtual ReferencePtr changeEndpoints(const std::vector<EndpointIPtr>&) const;
    virtual ReferencePtr changeAdapterId(const std::string&) const;
    virtual ReferencePtr changeLocator(const Ice::LocatorPrxPtr&) const;
    virtual ReferencePtr changeRouter(const Ice::RouterPrxPtr&) const;
    virtual ReferencePtr changeCollocationOptimized(bool) const;
    virtual ReferencePtr changeCacheConnection(bool) const;
    virtual ReferencePtr changePreferSecure(bool) const;
    virtual ReferencePtr changeEndpointSelection(Ice::EndpointSelectionType) const;
    virtual ReferencePtr changeLocatorCacheTimeout(int) const;

    virtual ReferencePtr changeTimeout(int) const;
    virtual ReferencePtr changeConnectionId(const std::string&) const;
    virtual ReferencePtr changeConnection(const Ice::ConnectionIPtr&) const;

    virtual bool isIndirect() const;
    virtual bool isWellKnown() const;

    virtual void streamWrite(Ice::OutputStream*) const;
    virtual Ice::PropertyDict toProperty(const std::string&) const;

    virtual RequestHandlerPtr getRequestHandler() const;
    virtual BatchRequestQueuePtr getBatchRequestQueue() const;

    virtual bool operator==(const Reference&) const;
    virtual bool operator<(const Reference&) const;

    virtual ReferencePtr clone() const;

private:

    Ice::ConnectionIPtr _fixedConnection;
};

using FixedReferencePtr = std::shared_ptr<FixedReference>;

class RoutableReference : public Reference
{
public:

    RoutableReference(const InstancePtr&, const Ice::CommunicatorPtr&, const Ice::Identity&, const std::string&, Mode,
                      bool, const Ice::ProtocolVersion&, const Ice::EncodingVersion&, const std::vector<EndpointIPtr>&,
                      const std::string&, const LocatorInfoPtr&, const RouterInfoPtr&, bool, bool, bool,
                      Ice::EndpointSelectionType, int, int, const Ice::Context&);

    RoutableReference(const RoutableReference&);

    virtual std::vector<EndpointIPtr> getEndpoints() const;
    virtual std::string getAdapterId() const;
    virtual LocatorInfoPtr getLocatorInfo() const;
    virtual RouterInfoPtr getRouterInfo() const;
    virtual bool getCollocationOptimized() const;
    virtual bool getCacheConnection() const;
    virtual bool getPreferSecure() const;
    virtual Ice::EndpointSelectionType getEndpointSelection() const;
    virtual int getLocatorCacheTimeout() const;
    virtual std::string getConnectionId() const;
    virtual std::optional<int> getTimeout() const;

    virtual ReferencePtr changeEncoding(const Ice::EncodingVersion&) const;
    virtual ReferencePtr changeCompress(bool) const;
    virtual ReferencePtr changeEndpoints(const std::vector<EndpointIPtr>&) const;
    virtual ReferencePtr changeAdapterId(const std::string&) const;
    virtual ReferencePtr changeLocator(const Ice::LocatorPrxPtr&) const;
    virtual ReferencePtr changeRouter(const Ice::RouterPrxPtr&) const;
    virtual ReferencePtr changeCollocationOptimized(bool) const;
    virtual ReferencePtr changeCacheConnection(bool) const;
    virtual ReferencePtr changePreferSecure(bool) const;
    virtual ReferencePtr changeEndpointSelection(Ice::EndpointSelectionType) const;
    virtual ReferencePtr changeLocatorCacheTimeout(int) const;

    virtual ReferencePtr changeTimeout(int) const;
    virtual ReferencePtr changeConnectionId(const std::string&) const;
    virtual ReferencePtr changeConnection(const Ice::ConnectionIPtr&) const;

    virtual bool isIndirect() const;
    virtual bool isWellKnown() const;

    virtual void streamWrite(Ice::OutputStream*) const;
    virtual std::string toString() const;
    virtual Ice::PropertyDict toProperty(const std::string&) const;

    virtual bool operator==(const Reference&) const;
    virtual bool operator<(const Reference&) const;

    virtual ReferencePtr clone() const;

    virtual RequestHandlerPtr getRequestHandler() const;
    virtual BatchRequestQueuePtr getBatchRequestQueue() const;

    void getConnectionAsync(
        std::function<void(Ice::ConnectionIPtr, bool)> response,
        std::function<void(std::exception_ptr)> exception) const;

    void applyOverrides(std::vector<EndpointIPtr>&) const;

protected:

    std::vector<EndpointIPtr> filterEndpoints(const std::vector<EndpointIPtr>&) const;

    virtual int hashInit() const;

private:

    void createConnectionAsync(
        const std::vector<EndpointIPtr>&,
        std::function<void(Ice::ConnectionIPtr, bool)> response,
        std::function<void(std::exception_ptr)> exception) const;

    void getConnectionNoRouterInfoAsync(
        std::function<void(Ice::ConnectionIPtr, bool)> response,
        std::function<void(std::exception_ptr)> exception) const;

    std::vector<EndpointIPtr> _endpoints; // Empty if indirect proxy.
    std::string _adapterId; // Empty if direct proxy.

    LocatorInfoPtr _locatorInfo; // Null if no locator is used.
    RouterInfoPtr _routerInfo; // Null if no router is used.
    bool _collocationOptimized;
    bool _cacheConnection;
    bool _preferSecure;
    Ice::EndpointSelectionType _endpointSelection;
    int _locatorCacheTimeout;

    bool _overrideTimeout;
    int _timeout; // Only used if _overrideTimeout == true
    std::string _connectionId;
};

using RoutableReferencePtr = std::shared_ptr<RoutableReference>;

}

#endif
