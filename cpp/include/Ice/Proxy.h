// Copyright (c) ZeroC, Inc.

#ifndef ICE_PROXY_H
#define ICE_PROXY_H

#include "BatchRequestQueueF.h"
#include "CommunicatorF.h"
#include "Current.h"
#include "EndpointF.h"
#include "EndpointSelectionType.h"
#include "Ice/BuiltinSequences.h"
#include "LocalExceptions.h"
#include "ReferenceF.h"
#include "RequestHandlerF.h"

#include <chrono>
#include <functional>
#include <future>
#include <iosfwd>
#include <optional>
#include <string_view>
#include <type_traits>

namespace IceInternal
{
    class ProxyGetConnection;
    class ProxyFlushBatchAsync;

    template<typename T> class OutgoingAsyncT;
}

namespace Ice
{
    /// Marker value used to indicate that no explicit request context was passed to a proxy invocation.
    ICE_API extern const Context noExplicitContext;

    class LocatorPrx;
    class ObjectPrx;
    class OutputStream;
    class RouterPrx;

    /// Provides typed proxy functions.
    /// @headerfile Ice/Ice.h
    template<typename Prx, typename... Bases> class Proxy : public virtual Bases...
    {
    public:
        /// The arrow operator.
        /// @return A pointer to this object.
        /// @remark This operator is provided for aesthetics and for compatibility reasons. A proxy is not pointer-like:
        /// it does not hold a value and can't be null. Calling `greeter->greet("bob")` is 100% equivalent to calling
        /// `greeter.greet("bob")` when greeter is a GreeterPrx proxy. An advantage of the arrow syntax is it's the same
        /// whether greeter is a GreeterPrx, an optional<GreeterPrx>, or a smart pointer like in earlier versions of
        /// Ice. It's also aesthetically pleasing when making invocations: the proxy appears like a pointer to the
        /// remote object.
        const Prx* operator->() const noexcept { return &asPrx(); }

        // We don't provide the non-const operator-> because only the assignment operator can modify the proxy.

        /// Creates a proxy that is identical to this proxy, except for the adapter ID.
        /// @param id The adapter ID for the new proxy.
        /// @return A proxy with the new adapter ID.
        [[nodiscard]] Prx ice_adapterId(std::string id) const
        {
            return fromReference(asPrx()._adapterId(std::move(id)));
        }

        /// Creates a proxy that is identical to this proxy, but uses batch datagram invocations.
        /// @return A proxy that uses batch datagram invocations.
        [[nodiscard]] Prx ice_batchDatagram() const { return fromReference(asPrx()._batchDatagram()); }

        /// Creates a proxy that is identical to this proxy, but uses batch oneway invocations.
        /// @return A proxy that uses batch oneway invocations.
        [[nodiscard]] Prx ice_batchOneway() const { return fromReference(asPrx()._batchOneway()); }

        /// Creates a proxy that is identical to this proxy, except for collocation optimization.
        /// @param b `true` if the new proxy enables collocation optimization, `false` otherwise.
        /// @return A proxy with the specified collocation optimization.
        [[nodiscard]] Prx ice_collocationOptimized(bool b) const
        {
            return fromReference(asPrx()._collocationOptimized(b));
        }

        /// Creates a proxy that is identical to this proxy, except for its compression setting which overrides the
        /// compression setting from the proxy endpoints.
        /// @param b `true` enables compression for the new proxy, `false` disables compression.
        /// @return A proxy with the specified compression override setting.
        [[nodiscard]] Prx ice_compress(bool b) const { return fromReference(asPrx()._compress(b)); }

        /// Creates a proxy that is identical to this proxy, except for connection caching.
        /// @param b `true` if the new proxy should cache connections, `false` otherwise.
        /// @return A proxy with the specified caching policy.
        [[nodiscard]] Prx ice_connectionCached(bool b) const { return fromReference(asPrx()._connectionCached(b)); }

        /// Creates a proxy that is identical to this proxy, except for its connection ID.
        /// @param id The connection ID for the new proxy. An empty string removes the connection ID.
        /// @return A proxy with the specified connection ID.
        [[nodiscard]] Prx ice_connectionId(std::string id) const
        {
            return fromReference(asPrx()._connectionId(std::move(id)));
        }

        /// Creates a proxy that is identical to this proxy, except for the per-proxy context.
        /// @param context The context for the new proxy.
        /// @return A proxy with the new per-proxy context.
        [[nodiscard]] Prx ice_context(Context context) const
        {
            return fromReference(asPrx()._context(std::move(context)));
        }

        /// Creates a proxy that is identical to this proxy, but uses datagram invocations.
        /// @return A proxy that uses datagram invocations.
        [[nodiscard]] Prx ice_datagram() const { return fromReference(asPrx()._datagram()); }

        /// Creates a proxy that is identical to this proxy, except for the encoding used to marshal parameters.
        /// @param version The encoding version to use to marshal request parameters.
        /// @return A proxy with the specified encoding version.
        [[nodiscard]] Prx ice_encodingVersion(EncodingVersion version) const
        {
            return fromReference(asPrx()._encodingVersion(version));
        }

        /// Creates a proxy that is identical to this proxy, except for the endpoint selection policy.
        /// @param type The new endpoint selection policy.
        /// @return A proxy with the specified endpoint selection policy.
        [[nodiscard]] Prx ice_endpointSelection(EndpointSelectionType type) const
        {
            return fromReference(asPrx()._endpointSelection(type));
        }

        /// Creates a proxy that is identical to this proxy, except for the endpoints.
        /// @param endpoints The endpoints for the new proxy.
        /// @return A proxy with the new endpoints.
        [[nodiscard]] Prx ice_endpoints(EndpointSeq endpoints) const
        {
            return fromReference(asPrx()._endpoints(std::move(endpoints)));
        }

        /// Creates a proxy that is identical to this proxy, except it's a fixed proxy bound to the given connection.
        /// @param connection The fixed proxy connection.
        /// @return A fixed proxy bound to the given connection.
        [[nodiscard]] Prx ice_fixed(ConnectionPtr connection) const
        {
            return fromReference(asPrx()._fixed(std::move(connection)));
        }

        /// Creates a proxy that is identical to this proxy, except for the invocation timeout.
        /// @param timeout The new invocation timeout (in milliseconds).
        /// @return A proxy with the new timeout.
        [[nodiscard]] Prx ice_invocationTimeout(int timeout) const
        {
            return ice_invocationTimeout(std::chrono::milliseconds(timeout));
        }

        /// Creates a proxy that is identical to this proxy, except for the invocation timeout.
        /// @param timeout The new invocation timeout.
        /// @return A proxy with the new timeout.
        template<class Rep, class Period>
        [[nodiscard]] Prx ice_invocationTimeout(const std::chrono::duration<Rep, Period>& timeout) const
        {
            return fromReference(
                asPrx()._invocationTimeout(std::chrono::duration_cast<std::chrono::milliseconds>(timeout)));
        }

        /// Creates a proxy that is identical to this proxy, except for the locator.
        /// @param locator The locator for the new proxy.
        /// @return A proxy with the specified locator.
        [[nodiscard]] Prx ice_locator(const std::optional<LocatorPrx>& locator) const
        {
            return fromReference(asPrx()._locator(locator));
        }

        /// Creates a proxy that is identical to this proxy, except for the locator cache timeout.
        /// @param timeout The new locator cache timeout (in seconds).
        /// @return A proxy with the new timeout.
        [[nodiscard]] Prx ice_locatorCacheTimeout(int timeout) const
        {
            return ice_locatorCacheTimeout(std::chrono::seconds(timeout));
        }

        /// Creates a proxy that is identical to this proxy, except for the locator cache timeout.
        /// @param timeout The new locator cache timeout.
        /// @return A proxy with the new timeout.
        template<class Rep, class Period>
        [[nodiscard]] Prx ice_locatorCacheTimeout(const std::chrono::duration<Rep, Period>& timeout) const
        {
            return fromReference(
                asPrx()._locatorCacheTimeout(std::chrono::duration_cast<std::chrono::seconds>(timeout)));
        }

        /// Creates a proxy that is identical to this proxy, but uses oneway invocations.
        /// @return A proxy that uses oneway invocations.
        [[nodiscard]] Prx ice_oneway() const { return fromReference(asPrx()._oneway()); }

        /// Creates a proxy that is identical to this proxy, except for the router.
        /// @param router The router for the new proxy.
        /// @return A proxy with the specified router.
        [[nodiscard]] Prx ice_router(const std::optional<RouterPrx>& router) const
        {
            return fromReference(asPrx()._router(router));
        }

        /// Creates a proxy that is identical to this proxy, but uses twoway invocations.
        /// @return A proxy that uses twoway invocations.
        [[nodiscard]] Prx ice_twoway() const { return fromReference(asPrx()._twoway()); }

    protected:
        /// @cond INTERNAL

        // This constructor never initializes the base classes since they are all virtual and Proxy is never the most
        // derived class.
        Proxy() = default;

        // The copy constructor and assignment operators are to keep GCC happy.
        Proxy(const Proxy&) noexcept = default;
        Proxy& operator=(const Proxy&) noexcept { return *this; } // NOLINT
        Proxy& operator=(Proxy&&) noexcept { return *this; }      // NOLINT
        /// @endcond

    private:
        Prx fromReference(IceInternal::ReferencePtr&& ref) const
        {
            const Prx& self = asPrx();
            return ref == self._reference ? self : Prx::_fromReference(std::move(ref));
        }

        [[nodiscard]] const Prx& asPrx() const noexcept { return *static_cast<const Prx*>(this); }
    };

    /// The base class for all Ice proxies.
    /// @headerfile Ice/Ice.h
    class ICE_API ObjectPrx : public Proxy<ObjectPrx>
    {
    public:
        /// Copy constructor. Constructs with a copy of the contents of @p other.
        /// @param other The proxy to copy from.
        ObjectPrx(const ObjectPrx& other) noexcept = default;

        /// Move constructor. Constructs a proxy with the contents of @p other using move semantics.
        /// @param other The proxy to move from.
        ObjectPrx(ObjectPrx&& other) noexcept = default;

        /// Constructs a proxy from a Communicator and a proxy string.
        /// @param communicator The communicator of the new proxy.
        /// @param proxyString The proxy string to parse.
        ObjectPrx(const Ice::CommunicatorPtr& communicator, std::string_view proxyString);

        virtual ~ObjectPrx();

        /// Copy assignment operator. Replaces the contents of this proxy with a copy of the contents of @p rhs.
        /// @param rhs The proxy to copy from.
        /// @return A reference to this proxy.
        ObjectPrx& operator=(const ObjectPrx& rhs) noexcept = default;

        /// Move assignment operator. Replaces the contents of this proxy with the contents of @p rhs using move
        /// semantics.
        /// @param rhs The proxy to move from.
        /// @return A reference to this proxy.
        ObjectPrx& operator=(ObjectPrx&& rhs) noexcept = default;

        /// Tests whether this object supports a specific Slice interface.
        /// @param typeId The type ID of the Slice interface to test against.
        /// @param context The request context.
        /// @return `true` if the target object implements the Slice interface specified by @p typeId or implements a
        /// derived interface, `false` otherwise.
        [[nodiscard]] bool ice_isA(std::string_view typeId, const Ice::Context& context = Ice::noExplicitContext) const;

        /// Tests whether this object supports a specific Slice interface.
        /// @param typeId The type ID of the Slice interface to test against.
        /// @param response The response callback. It accepts:
        /// - `true` if the target object implements the Slice interface specified by @p typeId or implements a
        /// derived interface, `false` otherwise.
        /// @param ex The exception callback.
        /// @param sent The sent callback.
        /// @param context The request context.
        /// @return A function that can be called to cancel the invocation locally.
        // NOLINTNEXTLINE(modernize-use-nodiscard)
        std::function<void()> ice_isAAsync(
            std::string_view typeId,
            std::function<void(bool)> response,
            std::function<void(std::exception_ptr)> ex = nullptr,
            std::function<void(bool)> sent = nullptr,
            const Ice::Context& context = Ice::noExplicitContext) const;

        /// Tests whether this object supports a specific Slice interface.
        /// @param typeId The type ID of the Slice interface to test against.
        /// @param context The request context.
        /// @return A future that becomes available when the invocation completes. This future holds:
        /// - `true` if the target object implements the Slice interface specified by @p typeId or implements a
        /// derived interface, `false` otherwise.
        [[nodiscard]] std::future<bool>
        ice_isAAsync(std::string_view typeId, const Ice::Context& context = Ice::noExplicitContext) const;

        /// @private
        void _iceI_isA(const std::shared_ptr<IceInternal::OutgoingAsyncT<bool>>&, std::string_view, const Ice::Context&)
            const;

        /// Tests whether the target object of this proxy can be reached.
        /// @param context The request context.
        void ice_ping(const Ice::Context& context = Ice::noExplicitContext) const;

        /// Tests whether the target object of this proxy can be reached.
        /// @param response The response callback.
        /// @param ex The exception callback.
        /// @param sent The sent callback.
        /// @param context The request context.
        /// @return A function that can be called to cancel the invocation locally.
        // NOLINTNEXTLINE(modernize-use-nodiscard)
        std::function<void()> ice_pingAsync(
            std::function<void()> response,
            std::function<void(std::exception_ptr)> ex = nullptr,
            std::function<void(bool)> sent = nullptr,
            const Ice::Context& context = Ice::noExplicitContext) const;

        /// Tests whether the target object of this proxy can be reached.
        /// @param context The request context.
        /// @return A future that becomes available when the response is received.
        [[nodiscard]] std::future<void> ice_pingAsync(const Ice::Context& context = Ice::noExplicitContext) const;

        /// @private
        void _iceI_ping(const std::shared_ptr<IceInternal::OutgoingAsyncT<void>>&, const Ice::Context&) const;

        /// Gets the Slice interfaces supported by this object as a list of Slice type IDs.
        /// @param context The request context.
        /// @return The Slice type IDs of the interfaces supported by this object, in alphabetical order.
        [[nodiscard]] std::vector<std::string> ice_ids(const Ice::Context& context = Ice::noExplicitContext) const;

        /// Gets the Slice interfaces supported by this object as a list of Slice type IDs.
        /// @param response The response callback. It accepts:
        /// - The Slice type IDs of the interfaces supported by this object, in alphabetical order.
        /// @param ex The exception callback.
        /// @param sent The sent callback.
        /// @param context The request context.
        /// @return A function that can be called to cancel the invocation locally.
        // NOLINTNEXTLINE(modernize-use-nodiscard)
        std::function<void()> ice_idsAsync(
            std::function<void(std::vector<std::string>)> response,
            std::function<void(std::exception_ptr)> ex = nullptr,
            std::function<void(bool)> sent = nullptr,
            const Ice::Context& context = Ice::noExplicitContext) const;

        /// Gets the Slice interfaces supported by this object as a list of Slice type IDs.
        /// @param context The request context.
        /// @return A future that becomes available when the invocation completes. This future holds:
        /// - The Slice type IDs of the interfaces supported by this object, in alphabetical order.
        [[nodiscard]] std::future<std::vector<std::string>>
        ice_idsAsync(const Ice::Context& context = Ice::noExplicitContext) const;

        /// @private
        void _iceI_ids(
            const std::shared_ptr<IceInternal::OutgoingAsyncT<std::vector<std::string>>>&,
            const Ice::Context&) const;

        /// Gets the type ID of the most-derived Slice interface supported by this object.
        /// @param context The request context.
        /// @return The Slice type ID of the most-derived interface.
        [[nodiscard]] std::string ice_id(const Ice::Context& context = Ice::noExplicitContext) const;

        /// Gets the type ID of the most-derived Slice interface supported by this object.
        /// @param response The response callback. It accepts:
        /// - The type ID of the most-derived interface.
        /// @param ex The exception callback.
        /// @param sent The sent callback.
        /// @param context The request context.
        /// @return A function that can be called to cancel the invocation locally.
        // NOLINTNEXTLINE(modernize-use-nodiscard)
        std::function<void()> ice_idAsync(
            std::function<void(std::string)> response,
            std::function<void(std::exception_ptr)> ex = nullptr,
            std::function<void(bool)> sent = nullptr,
            const Ice::Context& context = Ice::noExplicitContext) const;

        /// Gets the type ID of the most-derived Slice interface supported by this object.
        /// @param context The request context.
        /// @return A future that becomes available when the invocation completes. This future holds:
        /// - The type ID of the most-derived interface.
        [[nodiscard]] std::future<std::string> ice_idAsync(const Ice::Context& context = Ice::noExplicitContext) const;

        /// @private
        void _iceI_id(const std::shared_ptr<IceInternal::OutgoingAsyncT<std::string>>&, const Ice::Context&) const;

        /// Invokes an operation.
        /// @param operation The name of the operation to invoke.
        /// @param mode The operation mode (normal or idempotent).
        /// @param inParams An encapsulation containing the encoded in-parameters for the operation.
        /// @param outParams An encapsulation containing the encoded result.
        /// @param context The request context.
        /// @return `true` if the operation completed successfully, `false` if it completed with a user exception.
        bool ice_invoke(
            std::string_view operation,
            Ice::OperationMode mode,
            const std::vector<std::byte>& inParams,
            std::vector<std::byte>& outParams,
            const Ice::Context& context = Ice::noExplicitContext) const;

        /// Invokes an operation asynchronously.
        /// @param operation The name of the operation to invoke.
        /// @param mode The operation mode (normal or idempotent).
        /// @param inParams An encapsulation containing the encoded in-parameters for the operation.
        /// @param context The request context.
        /// @return A future that becomes available when the invocation completes. This future holds:
        /// - `returnValue` `true` if the operation completed successfully, `false` if it completed with a user
        ///    exception.
        /// - `outParams` An encapsulation containing the encoded result.
        [[nodiscard]] std::future<std::tuple<bool, std::vector<std::byte>>> ice_invokeAsync(
            std::string_view operation,
            Ice::OperationMode mode,
            const std::vector<std::byte>& inParams,
            const Ice::Context& context = Ice::noExplicitContext) const;

        /// Invokes an operation asynchronously.
        /// @param operation The name of the operation to invoke.
        /// @param mode The operation mode (normal or idempotent).
        /// @param inParams An encapsulation containing the encoded in-parameters for the operation.
        /// @param response The response callback. It accepts:
        /// - `returnValue` `true` if the operation completed successfully, `false` if it completed with a user
        ///    exception.
        /// - `outParams` An encapsulation containing the encoded result.
        /// @param ex The exception callback.
        /// @param sent The sent callback.
        /// @param context The request context.
        /// @return A function that can be called to cancel the invocation locally.
        // NOLINTNEXTLINE(modernize-use-nodiscard)
        std::function<void()> ice_invokeAsync(
            std::string_view operation,
            Ice::OperationMode mode,
            const std::vector<std::byte>& inParams,
            std::function<void(bool, std::vector<std::byte>)> response,
            std::function<void(std::exception_ptr)> ex = nullptr,
            std::function<void(bool)> sent = nullptr,
            const Ice::Context& context = Ice::noExplicitContext) const;

        /// Invokes an operation.
        /// @param operation The name of the operation to invoke.
        /// @param mode The operation mode (normal or idempotent).
        /// @param inParams An encapsulation containing the encoded in-parameters for the operation.
        /// @param outParams An encapsulation containing the encoded result.
        /// @param context The request context.
        /// @return `true` if the operation completed successfully, `false` if it completed with a user exception.
        bool ice_invoke(
            std::string_view operation,
            Ice::OperationMode mode,
            std::pair<const std::byte*, const std::byte*> inParams,
            std::vector<std::byte>& outParams,
            const Ice::Context& context = Ice::noExplicitContext) const;

        /// Invokes an operation asynchronously.
        /// @param operation The name of the operation to invoke.
        /// @param mode The operation mode (normal or idempotent).
        /// @param inParams An encapsulation containing the encoded in-parameters for the operation.
        /// @param context The request context.
        /// @return A future that becomes available when the invocation completes. This future holds:
        /// - `returnValue` `true` if the operation completed successfully, `false` if it completed with a user
        ///    exception.
        /// - `outParams` An encapsulation containing the encoded result.
        [[nodiscard]] std::future<std::tuple<bool, std::vector<std::byte>>> ice_invokeAsync(
            std::string_view operation,
            Ice::OperationMode mode,
            std::pair<const std::byte*, const std::byte*> inParams,
            const Ice::Context& context = Ice::noExplicitContext) const;

        /// Invokes an operation asynchronously.
        /// @param operation The name of the operation to invoke.
        /// @param mode The operation mode (normal or idempotent).
        /// @param inParams An encapsulation containing the encoded in-parameters for the operation.
        /// @param response The response callback. It accepts:
        /// - `returnValue` `true` if the operation completed successfully, `false` if it completed with a user
        ///    exception.
        /// - `outParams` An encapsulation containing the encoded result.
        /// @param ex The exception callback.
        /// @param sent The sent callback.
        /// @param context The request context.
        /// @return A function that can be called to cancel the invocation locally.
        // NOLINTNEXTLINE(modernize-use-nodiscard)
        std::function<void()> ice_invokeAsync(
            std::string_view operation,
            Ice::OperationMode mode,
            std::pair<const std::byte*, const std::byte*> inParams,
            std::function<void(bool, std::pair<const std::byte*, const std::byte*>)> response,
            std::function<void(std::exception_ptr)> ex = nullptr,
            std::function<void(bool)> sent = nullptr,
            const Ice::Context& context = Ice::noExplicitContext) const;

        /// Gets the connection for this proxy. If the proxy does not yet have an established connection,
        /// it first attempts to create a connection.
        /// @return The connection for this proxy.
        /// @remark You can call this function to establish a connection or associate the proxy with an existing
        /// connection and ignore the return value.
        Ice::ConnectionPtr ice_getConnection() const; // NOLINT(modernize-use-nodiscard)

        /// Gets the connection for this proxy. If the proxy does not yet have an established connection,
        /// it first attempts to create a connection.
        /// @param response The response callback. It accepts:
        /// - The connection for this proxy.
        /// @param ex The exception callback.
        /// @param sent The sent callback.
        /// @return A function that can be called to cancel the invocation locally.
        // NOLINTNEXTLINE(modernize-use-nodiscard)
        std::function<void()> ice_getConnectionAsync(
            std::function<void(Ice::ConnectionPtr)> response,
            std::function<void(std::exception_ptr)> ex = nullptr,
            std::function<void(bool)> sent = nullptr) const;

        /// Gets the connection for this proxy. If the proxy does not yet have an established connection,
        /// it first attempts to create a connection.
        /// @return A future that becomes available when the invocation completes. This future holds:
        /// - The connection for this proxy.
        [[nodiscard]] std::future<Ice::ConnectionPtr> ice_getConnectionAsync() const;

        /// @private
        void _iceI_getConnection(const std::shared_ptr<IceInternal::ProxyGetConnection>&) const;

        /// Gets the cached Connection for this proxy. If the proxy does not yet have an established connection, it does
        /// not attempt to create a connection.
        /// @return The cached connection for this proxy, or nullptr if the proxy does not have an established
        /// connection.
        [[nodiscard]] Ice::ConnectionPtr ice_getCachedConnection() const noexcept;

        /// Flushes any pending batched requests for this proxy. The call blocks until the flush is complete.
        void ice_flushBatchRequests() const;

        /// Flushes any pending batched requests for this proxy asynchronously.
        /// @param ex The exception callback.
        /// @param sent The sent callback.
        /// @return A function that can be called to cancel the invocation locally.
        // NOLINTNEXTLINE(modernize-use-nodiscard)
        std::function<void()> ice_flushBatchRequestsAsync(
            std::function<void(std::exception_ptr)> ex,
            std::function<void(bool)> sent = nullptr) const;

        /// Flushes any pending batched requests for this proxy asynchronously.
        /// @return A future that becomes available when the flush completes.
        [[nodiscard]] std::future<void> ice_flushBatchRequestsAsync() const;

        /// Gets the identity embedded in this proxy.
        /// @return The identity of the target object.
        [[nodiscard]] const Ice::Identity& ice_getIdentity() const noexcept;

        /// Creates a proxy that is identical to this proxy, except for the identity.
        /// @param id The identity for the new proxy.
        /// @return A proxy with the new identity.
        template<typename Prx = ObjectPrx, std::enable_if_t<std::is_base_of_v<ObjectPrx, Prx>, bool> = true>
        [[nodiscard]] Prx ice_identity(Ice::Identity id) const
        {
            return Prx::_fromReference(_identity(std::move(id)));
        }

        /// Gets the per-proxy context for this proxy.
        /// @return The per-proxy context.
        [[nodiscard]] Ice::Context ice_getContext() const;

        /// Gets the facet for this proxy.
        /// @return The facet for this proxy. If the proxy uses the default facet, the return value is the empty string.
        [[nodiscard]] const std::string& ice_getFacet() const noexcept;

        /// Creates a proxy that is identical to this proxy, except for the facet.
        /// @param facet The facet for the new proxy.
        /// @return A proxy with the new facet.
        template<typename Prx = ObjectPrx, std::enable_if_t<std::is_base_of_v<ObjectPrx, Prx>, bool> = true>
        [[nodiscard]] Prx ice_facet(std::string facet) const
        {
            return Prx::_fromReference(_facet(std::move(facet)));
        }

        /// Gets the adapter ID for this proxy.
        /// @return The adapter ID. If the proxy does not have an adapter ID, the return value is the empty string.
        [[nodiscard]] std::string ice_getAdapterId() const;

        /// Gets the endpoints used by this proxy.
        /// @return The endpoints used by this proxy.
        [[nodiscard]] Ice::EndpointSeq ice_getEndpoints() const;

        /// Gets the locator cache timeout of this proxy.
        /// @return The locator cache timeout value.
        [[nodiscard]] std::chrono::milliseconds ice_getLocatorCacheTimeout() const noexcept;

        /// Determines whether this proxy caches connections.
        /// @return `true` if this proxy caches connections, `false` otherwise.
        [[nodiscard]] bool ice_isConnectionCached() const noexcept;

        /// Gets the endpoint selection policy for this proxy (randomly or ordered).
        /// @return The endpoint selection policy.
        [[nodiscard]] Ice::EndpointSelectionType ice_getEndpointSelection() const noexcept;

        /// Gets the encoding version used to marshal request parameters.
        /// @return The encoding version.
        [[nodiscard]] Ice::EncodingVersion ice_getEncodingVersion() const noexcept;

        /// Gets the router for this proxy.
        /// @return The router for the proxy. If no router is configured for the proxy, the return value
        /// is nullopt.
        [[nodiscard]] std::optional<RouterPrx> ice_getRouter() const noexcept;

        /// Gets the locator for this proxy.
        /// @return The locator for this proxy. If no locator is configured, the return value is nullopt.
        [[nodiscard]] std::optional<LocatorPrx> ice_getLocator() const noexcept;

        /// Determines whether this proxy uses collocation optimization.
        /// @return `true` if the proxy uses collocation optimization, `false` otherwise.
        [[nodiscard]] bool ice_isCollocationOptimized() const noexcept;

        /// Gets the invocation timeout of this proxy.
        /// @return The invocation timeout value.
        [[nodiscard]] std::chrono::milliseconds ice_getInvocationTimeout() const noexcept;

        /// Determines whether this proxy uses twoway invocations.
        /// @return `true` if this proxy uses twoway invocations, `false` otherwise.
        [[nodiscard]] bool ice_isTwoway() const noexcept;

        /// Determines whether this proxy uses oneway invocations.
        /// @return `true` if this proxy uses oneway invocations, `false` otherwise.
        [[nodiscard]] bool ice_isOneway() const noexcept;

        /// Determines whether this proxy uses batch oneway invocations.
        /// @return `true` if this proxy uses batch oneway invocations, `false` otherwise.
        [[nodiscard]] bool ice_isBatchOneway() const noexcept;

        /// Determines whether this proxy uses datagram invocations.
        /// @return `true` if this proxy uses datagram invocations, `false` otherwise.
        [[nodiscard]] bool ice_isDatagram() const noexcept;

        /// Determines whether this proxy uses batch datagram invocations.
        /// @return `true` if this proxy uses batch datagram invocations, `false` otherwise.
        [[nodiscard]] bool ice_isBatchDatagram() const noexcept;

        /// Gets the compression override setting of this proxy.
        /// @return The compression override setting. If nullopt is returned, no override is set. Otherwise, true
        /// if compression is enabled, `false` otherwise.
        [[nodiscard]] std::optional<bool> ice_getCompress() const noexcept;

        /// Gets the connection ID of this proxy.
        /// @return The connection ID.
        [[nodiscard]] std::string ice_getConnectionId() const;

        /// Determines whether this proxy is a fixed proxy.
        /// @return `true` if this proxy is a fixed proxy, `false` otherwise.
        [[nodiscard]] bool ice_isFixed() const noexcept;

        /// Returns the Slice type ID associated with this type.
        /// @return The Slice type ID.
        static const char* ice_staticId() noexcept;

        /// Gets the communicator that created this proxy.
        /// @return The communicator that created this proxy.
        [[nodiscard]] Ice::CommunicatorPtr ice_getCommunicator() const noexcept;

        /// Creates a stringified version of this proxy.
        /// @return A stringified proxy.
        [[nodiscard]] std::string ice_toString() const;

        /// @private
        void _iceI_flushBatchRequests(const std::shared_ptr<IceInternal::ProxyFlushBatchAsync>&) const;

        /// @cond INTERNAL
        static ObjectPrx _fromReference(IceInternal::ReferencePtr ref) { return ObjectPrx(std::move(ref)); }

        [[nodiscard]] const IceInternal::ReferencePtr& _getReference() const noexcept { return _reference; }
        [[nodiscard]] const IceInternal::RequestHandlerCachePtr& _getRequestHandlerCache() const noexcept
        {
            return _requestHandlerCache;
        }

        void _checkTwowayOnly(std::string_view) const;

        [[nodiscard]] size_t _hash() const noexcept;

        void _write(OutputStream&) const;

    protected:
        // This constructor is never called; it allows Proxy's default constructor to compile.
        ObjectPrx() = default;

        // The constructor used by _fromReference.
        explicit ObjectPrx(IceInternal::ReferencePtr&&);
        /// @endcond

    private:
        template<typename Prx, typename... Bases> friend class Proxy;

        // Gets a reference with the specified setting; returns _reference if the setting is already set.
        [[nodiscard]] IceInternal::ReferencePtr _adapterId(std::string) const;
        [[nodiscard]] IceInternal::ReferencePtr _batchDatagram() const;
        [[nodiscard]] IceInternal::ReferencePtr _batchOneway() const;
        [[nodiscard]] IceInternal::ReferencePtr _collocationOptimized(bool) const;
        [[nodiscard]] IceInternal::ReferencePtr _compress(bool) const;
        [[nodiscard]] IceInternal::ReferencePtr _connectionCached(bool) const;
        [[nodiscard]] IceInternal::ReferencePtr _connectionId(std::string) const;
        [[nodiscard]] IceInternal::ReferencePtr _context(Context) const;
        [[nodiscard]] IceInternal::ReferencePtr _datagram() const;
        [[nodiscard]] IceInternal::ReferencePtr _encodingVersion(EncodingVersion) const;
        [[nodiscard]] IceInternal::ReferencePtr _endpointSelection(EndpointSelectionType) const;
        [[nodiscard]] IceInternal::ReferencePtr _endpoints(EndpointSeq) const;
        [[nodiscard]] IceInternal::ReferencePtr _identity(Identity) const;
        [[nodiscard]] IceInternal::ReferencePtr _facet(std::string) const;
        [[nodiscard]] IceInternal::ReferencePtr _fixed(ConnectionPtr) const;
        [[nodiscard]] IceInternal::ReferencePtr _invocationTimeout(std::chrono::milliseconds) const;
        [[nodiscard]] IceInternal::ReferencePtr _locator(const std::optional<LocatorPrx>&) const;
        [[nodiscard]] IceInternal::ReferencePtr _locatorCacheTimeout(std::chrono::milliseconds) const;
        [[nodiscard]] IceInternal::ReferencePtr _oneway() const;
        [[nodiscard]] IceInternal::ReferencePtr _router(const std::optional<RouterPrx>&) const;
        [[nodiscard]] IceInternal::ReferencePtr _twoway() const;

        // Only the assignment operators can change these fields. All other member functions must be const.
        IceInternal::ReferencePtr _reference;
        IceInternal::RequestHandlerCachePtr _requestHandlerCache;
    };

    /// Outputs the stringified version of a proxy to a stream.
    /// @param os The output stream.
    /// @param proxy The proxy to output.
    /// @return The output stream.
    ICE_API std::ostream& operator<<(std::ostream& os, const ObjectPrx& proxy);

    /// Outputs the stringified version of a proxy to a stream.
    /// @tparam Prx The proxy type.
    /// @param os The output stream.
    /// @param proxy The proxy to output.
    /// @return The output stream.
    template<typename Prx, std::enable_if_t<std::is_base_of_v<ObjectPrx, Prx>, bool> = true>
    inline std::ostream& operator<<(std::ostream& os, const std::optional<Prx>& proxy)
    {
        if (proxy)
        {
            os << *proxy;
        }
        else
        {
            os << "";
        }
        return os;
    }
}

namespace std
{
    /// Specialization of std::hash for Ice::ObjectPrx.
    template<> struct hash<Ice::ObjectPrx>
    {
        std::size_t operator()(const Ice::ObjectPrx& p) const noexcept { return p._hash(); }
    };
}

#endif
