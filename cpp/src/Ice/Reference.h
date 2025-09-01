// Copyright (c) ZeroC, Inc.

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

        [[nodiscard]] Mode getMode() const noexcept { return _mode; }
        [[nodiscard]] bool isBatch() const noexcept { return _mode == ModeBatchOneway || _mode == ModeBatchDatagram; }
        [[nodiscard]] bool isTwoway() const noexcept { return _mode == ModeTwoway; }
        [[nodiscard]] const Ice::ProtocolVersion& getProtocol() const noexcept { return _protocol; }
        [[nodiscard]] const Ice::EncodingVersion& getEncoding() const noexcept { return _encoding; }
        [[nodiscard]] const Ice::Identity& getIdentity() const noexcept { return _identity; }
        [[nodiscard]] const std::string& getFacet() const noexcept { return _facet; }
        [[nodiscard]] const InstancePtr& getInstance() const noexcept { return _instance; }
        [[nodiscard]] const SharedContextPtr& getContext() const noexcept { return _context; }
        [[nodiscard]] std::chrono::milliseconds getInvocationTimeout() const noexcept { return _invocationTimeout; }
        [[nodiscard]] std::optional<bool> getCompress() const noexcept { return _compress; }

        [[nodiscard]] Ice::CommunicatorPtr getCommunicator() const noexcept;

        [[nodiscard]] virtual std::vector<EndpointIPtr> getEndpoints() const = 0;
        [[nodiscard]] virtual std::string getAdapterId() const = 0;
        [[nodiscard]] virtual LocatorInfoPtr getLocatorInfo() const noexcept { return nullptr; }
        [[nodiscard]] virtual RouterInfoPtr getRouterInfo() const noexcept { return nullptr; }
        [[nodiscard]] virtual bool getCollocationOptimized() const noexcept = 0;
        [[nodiscard]] virtual bool getCacheConnection() const noexcept = 0;
        [[nodiscard]] virtual Ice::EndpointSelectionType getEndpointSelection() const noexcept = 0;
        [[nodiscard]] virtual std::chrono::milliseconds getLocatorCacheTimeout() const noexcept = 0;
        [[nodiscard]] virtual std::string getConnectionId() const = 0;

        //
        // The change* methods (here and in derived classes) create
        // a new reference based on the existing one, with the
        // corresponding value changed.
        //
        [[nodiscard]] ReferencePtr changeContext(Ice::Context) const;
        [[nodiscard]] ReferencePtr changeIdentity(Ice::Identity) const;
        [[nodiscard]] ReferencePtr changeFacet(std::string) const;
        [[nodiscard]] ReferencePtr changeInvocationTimeout(std::chrono::milliseconds) const;

        [[nodiscard]] virtual ReferencePtr changeEncoding(Ice::EncodingVersion) const;
        [[nodiscard]] virtual ReferencePtr changeCompress(bool) const;
        [[nodiscard]] virtual ReferencePtr changeMode(Mode) const;
        [[nodiscard]] virtual ReferencePtr changeEndpoints(std::vector<EndpointIPtr>) const = 0;
        [[nodiscard]] virtual ReferencePtr changeAdapterId(std::string) const = 0;
        [[nodiscard]] virtual ReferencePtr changeLocator(std::optional<Ice::LocatorPrx>) const = 0;
        [[nodiscard]] virtual ReferencePtr changeRouter(std::optional<Ice::RouterPrx>) const = 0;
        [[nodiscard]] virtual ReferencePtr changeCollocationOptimized(bool) const = 0;
        [[nodiscard]] virtual ReferencePtr changeLocatorCacheTimeout(std::chrono::milliseconds) const = 0;
        [[nodiscard]] virtual ReferencePtr changeCacheConnection(bool) const = 0;
        [[nodiscard]] virtual ReferencePtr changeEndpointSelection(Ice::EndpointSelectionType) const = 0;

        [[nodiscard]] virtual ReferencePtr changeConnectionId(std::string) const = 0;
        [[nodiscard]] virtual ReferencePtr changeConnection(Ice::ConnectionIPtr) const = 0;

        [[nodiscard]] virtual std::size_t hash() const noexcept;

        // Gets the effective compression setting, taking into account the override.
        [[nodiscard]] std::optional<bool> getCompressOverride() const noexcept;

        //
        // Utility methods.
        //
        [[nodiscard]] virtual bool isIndirect() const noexcept = 0;
        [[nodiscard]] virtual bool isWellKnown() const noexcept = 0;

        //
        // Marshal the reference.
        //
        virtual void streamWrite(Ice::OutputStream*) const;

        //
        // Convert the reference to its string form.
        //
        [[nodiscard]] virtual std::string toString() const;

        //
        // Convert the reference to its property form.
        //
        [[nodiscard]] virtual Ice::PropertyDict toProperty(std::string) const = 0;

        //
        // Get a suitable connection for this reference.
        //
        [[nodiscard]] virtual RequestHandlerPtr getRequestHandler() const = 0;

        //
        // Get batch request queue.
        //
        [[nodiscard]] virtual const BatchRequestQueuePtr& getBatchRequestQueue() const noexcept = 0;

        virtual bool operator==(const Reference&) const noexcept;
        virtual bool operator<(const Reference&) const noexcept;

        [[nodiscard]] virtual ReferencePtr clone() const = 0;

    protected:
        Reference(
            InstancePtr,
            Ice::CommunicatorPtr,
            Ice::Identity,
            std::string,
            Mode,
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
            std::optional<bool>,
            Ice::ProtocolVersion,
            Ice::EncodingVersion,
            Ice::ConnectionIPtr,
            std::chrono::milliseconds,
            Ice::Context);

        FixedReference(const FixedReference&) = default;

        [[nodiscard]] std::vector<EndpointIPtr> getEndpoints() const final;
        [[nodiscard]] std::string getAdapterId() const final;
        [[nodiscard]] bool getCollocationOptimized() const noexcept final;
        [[nodiscard]] bool getCacheConnection() const noexcept final;
        [[nodiscard]] Ice::EndpointSelectionType getEndpointSelection() const noexcept final;
        [[nodiscard]] std::chrono::milliseconds getLocatorCacheTimeout() const noexcept final;
        [[nodiscard]] std::string getConnectionId() const final;

        [[nodiscard]] ReferencePtr changeEndpoints(std::vector<EndpointIPtr>) const final;
        [[nodiscard]] ReferencePtr changeAdapterId(std::string) const final;
        [[nodiscard]] ReferencePtr changeLocator(std::optional<Ice::LocatorPrx>) const final;
        [[nodiscard]] ReferencePtr changeRouter(std::optional<Ice::RouterPrx>) const final;
        [[nodiscard]] ReferencePtr changeCollocationOptimized(bool) const final;
        [[nodiscard]] ReferencePtr changeCacheConnection(bool) const final;
        [[nodiscard]] ReferencePtr changeEndpointSelection(Ice::EndpointSelectionType) const final;
        [[nodiscard]] ReferencePtr changeLocatorCacheTimeout(std::chrono::milliseconds) const final;

        [[nodiscard]] ReferencePtr changeConnectionId(std::string) const final;
        [[nodiscard]] ReferencePtr changeConnection(Ice::ConnectionIPtr) const final;

        [[nodiscard]] bool isIndirect() const noexcept final;
        [[nodiscard]] bool isWellKnown() const noexcept final;

        void streamWrite(Ice::OutputStream*) const final;
        [[nodiscard]] Ice::PropertyDict toProperty(std::string) const final;

        [[nodiscard]] RequestHandlerPtr getRequestHandler() const final;
        [[nodiscard]] const BatchRequestQueuePtr& getBatchRequestQueue() const noexcept final;

        bool operator==(const Reference&) const noexcept final;
        bool operator<(const Reference&) const noexcept final;

        [[nodiscard]] ReferencePtr clone() const final;

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
            std::optional<bool>,
            Ice::ProtocolVersion,
            Ice::EncodingVersion,
            std::vector<EndpointIPtr>,
            std::string,
            LocatorInfoPtr,
            RouterInfoPtr,
            bool,
            bool,
            Ice::EndpointSelectionType,
            std::chrono::milliseconds,
            std::chrono::milliseconds,
            Ice::Context);

        RoutableReference(const RoutableReference&);

        [[nodiscard]] std::vector<EndpointIPtr> getEndpoints() const final;
        [[nodiscard]] std::string getAdapterId() const final;
        [[nodiscard]] LocatorInfoPtr getLocatorInfo() const noexcept final;
        [[nodiscard]] RouterInfoPtr getRouterInfo() const noexcept final;
        [[nodiscard]] bool getCollocationOptimized() const noexcept final;
        [[nodiscard]] bool getCacheConnection() const noexcept final;
        [[nodiscard]] Ice::EndpointSelectionType getEndpointSelection() const noexcept final;
        [[nodiscard]] std::chrono::milliseconds getLocatorCacheTimeout() const noexcept final;
        [[nodiscard]] std::string getConnectionId() const final;

        [[nodiscard]] ReferencePtr changeEncoding(Ice::EncodingVersion) const final;
        [[nodiscard]] ReferencePtr changeCompress(bool) const final;
        [[nodiscard]] ReferencePtr changeMode(Mode) const final;
        [[nodiscard]] ReferencePtr changeEndpoints(std::vector<EndpointIPtr>) const final;
        [[nodiscard]] ReferencePtr changeAdapterId(std::string) const final;
        [[nodiscard]] ReferencePtr changeLocator(std::optional<Ice::LocatorPrx>) const final;
        [[nodiscard]] ReferencePtr changeRouter(std::optional<Ice::RouterPrx>) const final;
        [[nodiscard]] ReferencePtr changeCollocationOptimized(bool) const final;
        [[nodiscard]] ReferencePtr changeCacheConnection(bool) const final;
        [[nodiscard]] ReferencePtr changeEndpointSelection(Ice::EndpointSelectionType) const final;
        [[nodiscard]] ReferencePtr changeLocatorCacheTimeout(std::chrono::milliseconds) const final;

        [[nodiscard]] ReferencePtr changeConnectionId(std::string) const final;
        [[nodiscard]] ReferencePtr changeConnection(Ice::ConnectionIPtr) const final;

        [[nodiscard]] bool isIndirect() const noexcept final;
        [[nodiscard]] bool isWellKnown() const noexcept final;

        void streamWrite(Ice::OutputStream*) const final;
        [[nodiscard]] std::string toString() const final;
        [[nodiscard]] Ice::PropertyDict toProperty(std::string) const final;

        bool operator==(const Reference&) const noexcept final;
        bool operator<(const Reference&) const noexcept final;

        [[nodiscard]] std::size_t hash() const noexcept final;

        [[nodiscard]] ReferencePtr clone() const final;

        [[nodiscard]] RequestHandlerPtr getRequestHandler() const final;
        [[nodiscard]] const BatchRequestQueuePtr& getBatchRequestQueue() const noexcept final;

        void getConnectionAsync(
            std::function<void(Ice::ConnectionIPtr, bool)> response,
            std::function<void(std::exception_ptr)> exception) const;

        void applyOverrides(std::vector<EndpointIPtr>&) const;

    protected:
        [[nodiscard]] std::vector<EndpointIPtr> filterEndpoints(const std::vector<EndpointIPtr>&) const;

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
