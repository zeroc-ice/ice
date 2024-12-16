//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_REFERENCE_H
#define ICE_REFERENCE_H

#include "BatchRequestQueue.h"
#include "EndpointIF.h"
#include "Ice/ConnectionIF.h"
#include "Ice/Identity.h"
#include "Ice/InstanceF.h"
#include "Ice/Locator.h"
#include "Ice/Properties.h"
#include "Ice/ReferenceF.h"
#include "Ice/RequestHandlerF.h"
#include "Ice/Router.h"
#include "Ice/VersionFunctions.h"
#include "LocatorInfoF.h"
#include "ReferenceFactoryF.h"
#include "RouterInfoF.h"
#include "SharedContext.h"

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
            ModeTwoway = 0,
            ModeOneway = 1,
            ModeBatchOneway = 2,
            ModeDatagram = 3,
            ModeBatchDatagram = 4,
            ModeLast = ModeBatchDatagram
        };

        Mode getMode() const noexcept { return _mode; }
        bool isBatch() const noexcept { return _mode == ModeBatchOneway || _mode == ModeBatchDatagram; }
        bool isTwoway() const noexcept { return _mode == ModeTwoway; }
        bool getSecure() const noexcept { return _secure; }
        const Ice::ProtocolVersion& getProtocol() const noexcept { return _protocol; }
        const Ice::EncodingVersion& getEncoding() const noexcept { return _encoding; }
        const Ice::Identity& getIdentity() const noexcept { return _identity; }
        const std::string& getFacet() const noexcept { return _facet; }
        const InstancePtr& getInstance() const noexcept { return _instance; }
        const SharedContextPtr& getContext() const noexcept { return _context; }
        std::chrono::milliseconds getInvocationTimeout() const noexcept { return _invocationTimeout; }
        std::optional<bool> getCompress() const noexcept { return _compress; }

        Ice::CommunicatorPtr getCommunicator() const noexcept;

        virtual std::vector<EndpointIPtr> getEndpoints() const = 0;
        virtual std::string getAdapterId() const = 0;
        virtual LocatorInfoPtr getLocatorInfo() const noexcept { return nullptr; }
        virtual RouterInfoPtr getRouterInfo() const noexcept { return nullptr; }
        virtual bool getCollocationOptimized() const noexcept = 0;
        virtual bool getCacheConnection() const noexcept = 0;
        virtual bool getPreferSecure() const noexcept = 0;
        virtual Ice::EndpointSelectionType getEndpointSelection() const noexcept = 0;
        virtual std::chrono::milliseconds getLocatorCacheTimeout() const noexcept = 0;
        virtual std::string getConnectionId() const = 0;

        //
        // The change* methods (here and in derived classes) create
        // a new reference based on the existing one, with the
        // corresponding value changed.
        //
        ReferencePtr changeContext(Ice::Context) const;
        ReferencePtr changeSecure(bool) const;
        ReferencePtr changeIdentity(Ice::Identity) const;
        ReferencePtr changeFacet(std::string) const;
        ReferencePtr changeInvocationTimeout(std::chrono::milliseconds) const;

        virtual ReferencePtr changeEncoding(Ice::EncodingVersion) const;
        virtual ReferencePtr changeCompress(bool) const;
        virtual ReferencePtr changeMode(Mode) const;
        virtual ReferencePtr changeEndpoints(std::vector<EndpointIPtr>) const = 0;
        virtual ReferencePtr changeAdapterId(std::string) const = 0;
        virtual ReferencePtr changeLocator(std::optional<Ice::LocatorPrx>) const = 0;
        virtual ReferencePtr changeRouter(std::optional<Ice::RouterPrx>) const = 0;
        virtual ReferencePtr changeCollocationOptimized(bool) const = 0;
        virtual ReferencePtr changeLocatorCacheTimeout(std::chrono::milliseconds) const = 0;
        virtual ReferencePtr changeCacheConnection(bool) const = 0;
        virtual ReferencePtr changePreferSecure(bool) const = 0;
        virtual ReferencePtr changeEndpointSelection(Ice::EndpointSelectionType) const = 0;

        virtual ReferencePtr changeConnectionId(std::string) const = 0;
        virtual ReferencePtr changeConnection(Ice::ConnectionIPtr) const = 0;

        virtual std::size_t hash() const noexcept;

        // Gets the effective compression setting, taking into account the override.
        std::optional<bool> getCompressOverride() const noexcept;

        //
        // Utility methods.
        //
        virtual bool isIndirect() const noexcept = 0;
        virtual bool isWellKnown() const noexcept = 0;

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
        virtual Ice::PropertyDict toProperty(std::string) const = 0;

        //
        // Get a suitable connection for this reference.
        //
        virtual RequestHandlerPtr getRequestHandler() const = 0;

        //
        // Get batch request queue.
        //
        virtual const BatchRequestQueuePtr& getBatchRequestQueue() const noexcept = 0;

        virtual bool operator==(const Reference&) const noexcept;
        virtual bool operator<(const Reference&) const noexcept;

        virtual ReferencePtr clone() const = 0;

    protected:
        Reference(
            InstancePtr,
            Ice::CommunicatorPtr,
            Ice::Identity,
            std::string,
            Mode,
            bool,
            std::optional<bool>,
            Ice::ProtocolVersion,
            Ice::EncodingVersion,
            std::chrono::milliseconds,
            Ice::Context ctx);

        Reference(const Reference&);

        const InstancePtr _instance;

    private:
        const Ice::CommunicatorPtr _communicator;

        Mode _mode;
        bool _secure;
        std::optional<bool> _compress;
        Ice::Identity _identity;
        SharedContextPtr _context;
        std::string _facet;
        Ice::ProtocolVersion _protocol;
        Ice::EncodingVersion _encoding;
        std::chrono::milliseconds _invocationTimeout;
    };

    class FixedReference final : public Reference
    {
    public:
        FixedReference(
            InstancePtr,
            Ice::CommunicatorPtr,
            Ice::Identity,
            std::string,
            Mode,
            bool,
            std::optional<bool>,
            Ice::ProtocolVersion,
            Ice::EncodingVersion,
            Ice::ConnectionIPtr,
            std::chrono::milliseconds,
            Ice::Context);

        FixedReference(const FixedReference&);

        std::vector<EndpointIPtr> getEndpoints() const final;
        std::string getAdapterId() const final;
        bool getCollocationOptimized() const noexcept final;
        bool getCacheConnection() const noexcept final;
        bool getPreferSecure() const noexcept final;
        Ice::EndpointSelectionType getEndpointSelection() const noexcept final;
        std::chrono::milliseconds getLocatorCacheTimeout() const noexcept final;
        std::string getConnectionId() const final;

        ReferencePtr changeEndpoints(std::vector<EndpointIPtr>) const final;
        ReferencePtr changeAdapterId(std::string) const final;
        ReferencePtr changeLocator(std::optional<Ice::LocatorPrx>) const final;
        ReferencePtr changeRouter(std::optional<Ice::RouterPrx>) const final;
        ReferencePtr changeCollocationOptimized(bool) const final;
        ReferencePtr changeCacheConnection(bool) const final;
        ReferencePtr changePreferSecure(bool) const final;
        ReferencePtr changeEndpointSelection(Ice::EndpointSelectionType) const final;
        ReferencePtr changeLocatorCacheTimeout(std::chrono::milliseconds) const final;

        ReferencePtr changeConnectionId(std::string) const final;
        ReferencePtr changeConnection(Ice::ConnectionIPtr) const final;

        bool isIndirect() const noexcept final;
        bool isWellKnown() const noexcept final;

        void streamWrite(Ice::OutputStream*) const final;
        Ice::PropertyDict toProperty(std::string) const final;

        RequestHandlerPtr getRequestHandler() const final;
        const BatchRequestQueuePtr& getBatchRequestQueue() const noexcept final;

        bool operator==(const Reference&) const noexcept final;
        bool operator<(const Reference&) const noexcept final;

        ReferencePtr clone() const final;

    private:
        Ice::ConnectionIPtr _fixedConnection;
    };

    using FixedReferencePtr = std::shared_ptr<FixedReference>;

    class RoutableReference final : public Reference
    {
    public:
        RoutableReference(
            InstancePtr,
            Ice::CommunicatorPtr,
            Ice::Identity,
            std::string,
            Mode,
            bool,
            std::optional<bool>,
            Ice::ProtocolVersion,
            Ice::EncodingVersion,
            std::vector<EndpointIPtr>,
            std::string,
            LocatorInfoPtr,
            RouterInfoPtr,
            bool,
            bool,
            bool,
            Ice::EndpointSelectionType,
            std::chrono::milliseconds,
            std::chrono::milliseconds,
            Ice::Context);

        RoutableReference(const RoutableReference&);

        std::vector<EndpointIPtr> getEndpoints() const final;
        std::string getAdapterId() const final;
        LocatorInfoPtr getLocatorInfo() const noexcept final;
        RouterInfoPtr getRouterInfo() const noexcept final;
        bool getCollocationOptimized() const noexcept final;
        bool getCacheConnection() const noexcept final;
        bool getPreferSecure() const noexcept final;
        Ice::EndpointSelectionType getEndpointSelection() const noexcept final;
        std::chrono::milliseconds getLocatorCacheTimeout() const noexcept final;
        std::string getConnectionId() const final;

        ReferencePtr changeEncoding(Ice::EncodingVersion) const final;
        ReferencePtr changeCompress(bool) const final;
        ReferencePtr changeMode(Mode) const final;
        ReferencePtr changeEndpoints(std::vector<EndpointIPtr>) const final;
        ReferencePtr changeAdapterId(std::string) const final;
        ReferencePtr changeLocator(std::optional<Ice::LocatorPrx>) const final;
        ReferencePtr changeRouter(std::optional<Ice::RouterPrx>) const final;
        ReferencePtr changeCollocationOptimized(bool) const final;
        ReferencePtr changeCacheConnection(bool) const final;
        ReferencePtr changePreferSecure(bool) const final;
        ReferencePtr changeEndpointSelection(Ice::EndpointSelectionType) const final;
        ReferencePtr changeLocatorCacheTimeout(std::chrono::milliseconds) const final;

        ReferencePtr changeConnectionId(std::string) const final;
        ReferencePtr changeConnection(Ice::ConnectionIPtr) const final;

        bool isIndirect() const noexcept final;
        bool isWellKnown() const noexcept final;

        void streamWrite(Ice::OutputStream*) const final;
        std::string toString() const final;
        Ice::PropertyDict toProperty(std::string) const final;

        bool operator==(const Reference&) const noexcept final;
        bool operator<(const Reference&) const noexcept final;

        std::size_t hash() const noexcept final;

        ReferencePtr clone() const final;

        RequestHandlerPtr getRequestHandler() const final;
        const BatchRequestQueuePtr& getBatchRequestQueue() const noexcept final;

        void getConnectionAsync(
            std::function<void(Ice::ConnectionIPtr, bool)> response,
            std::function<void(std::exception_ptr)> exception) const;

        void applyOverrides(std::vector<EndpointIPtr>&) const;

    protected:
        std::vector<EndpointIPtr> filterEndpoints(const std::vector<EndpointIPtr>&) const;

    private:
        void createConnectionAsync(
            const std::vector<EndpointIPtr>&,
            std::function<void(Ice::ConnectionIPtr, bool)> response,
            std::function<void(std::exception_ptr)> exception) const;

        void getConnectionNoRouterInfoAsync(
            std::function<void(Ice::ConnectionIPtr, bool)> response,
            std::function<void(std::exception_ptr)> exception) const;

        // Sets or resets _batchRequestQueue based on _mode.
        void setBatchRequestQueue();

        BatchRequestQueuePtr _batchRequestQueue;

        std::vector<EndpointIPtr> _endpoints; // Empty if indirect proxy.
        std::string _adapterId;               // Empty if direct proxy.

        LocatorInfoPtr _locatorInfo; // Null if no locator is used.
        RouterInfoPtr _routerInfo;   // Null if no router is used.
        bool _collocationOptimized;
        bool _cacheConnection;
        bool _preferSecure;
        Ice::EndpointSelectionType _endpointSelection;
        std::chrono::milliseconds _locatorCacheTimeout;

        std::string _connectionId;
    };

    using RoutableReferencePtr = std::shared_ptr<RoutableReference>;
}

namespace std
{
    // Specialization of std::hash for Reference.
    template<> struct hash<IceInternal::Reference>
    {
        std::size_t operator()(const IceInternal::Reference& r) const { return r.hash(); }
    };
}

#endif
