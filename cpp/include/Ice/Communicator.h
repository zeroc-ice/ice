// Copyright (c) ZeroC, Inc.

#ifndef ICE_COMMUNICATOR_H
#define ICE_COMMUNICATOR_H

#include "CommunicatorF.h"
#include "Config.h"
#include "Connection.h"
#include "FacetMap.h"
#include "ImplicitContext.h"
#include "Initialize.h"
#include "InstanceF.h"
#include "Plugin.h"
#include "Properties.h"
#include "Proxy.h"
#include "SSL/ServerAuthenticationOptions.h"

namespace Ice
{
    class LocatorPrx;
    class RouterPrx;

    /// Communicator is the central object in Ice. Its responsibilities include:
    /// - creating and managing outgoing connections
    /// - executing callbacks in its client thread pool
    /// - creating and destroying object adapters
    /// - loading plug-ins
    /// - managing properties (configuration), retries, logging, instrumentation, and more.
    /// You create a communicator with `Ice::initialize`, and it's usually the first object you create when programming
    /// with Ice. You can create multiple communicators in a single program, but this is not common.
    /// @see ::initialize(int&, const char*[], InitializationData)
    /// @see ::initialize(InitializationData)
    /// @headerfile Ice/Ice.h
    class ICE_API Communicator final : public std::enable_shared_from_this<Communicator>
    {
    public:
        ~Communicator();

        /// Destroys this communicator. This function calls #shutdown implicitly. Calling #destroy destroys all
        /// object adapters, and closes all outgoing connections. `destroy` waits for all outstanding dispatches to
        /// complete before returning. This includes "bidirectional dispatches" that execute on outgoing connections.
        /// @see CommunicatorHolder
        void destroy() noexcept;

        /// Destroys this communicator asynchronously.
        /// @param completed The callback to call when the destruction is complete. This function must not throw any
        /// exception.
        /// @remark This function starts a thread to call #destroy and @p completed unless you call this function on a
        /// communicator that has already been destroyed, in which case @p completed is called by the current thread.
        /// @see #destroy
        void destroyAsync(std::function<void()> completed) noexcept;

        /// Shuts down this communicator. This function calls ObjectAdapter::deactivate on all object adapters created
        /// by this communicator. Shutting down a communicator has no effect on outgoing connections.
        /// @see #waitForShutdown
        /// @see ObjectAdapter::deactivate
        void shutdown() noexcept;

        /// Waits for shutdown to complete. This function calls ObjectAdapter::waitForDeactivate on all object adapters
        /// created by this communicator. In a client application that does not accept incoming connections, this
        /// function returns as soon as another thread calls #shutdown or #destroy on this communicator.
        /// @see #shutdown
        void waitForShutdown() noexcept;

        /// Waits until this communicator is shut down.
        /// @param completed The callback to call when the shutdown is complete. This function must not throw any
        /// exception.
        /// @remark If you call this function on a communicator that has already been shut down, the callback is called
        /// immediately by the current thread.
        /// @see #shutdown
        void waitForShutdownAsync(std::function<void()> completed) noexcept;

        /// Checks whether or not #shutdown was called on this communicator.
        /// @return `true` if #shutdown was called on this communicator, `false` otherwise.
        /// @see #shutdown
        [[nodiscard]] bool isShutdown() const noexcept;

        /// Converts a stringified proxy into a proxy.
        /// @tparam Prx The type of the proxy to return.
        /// @param str The stringified proxy to convert into a proxy.
        /// @return The proxy, or nullopt if @p str is an empty string.
        /// @throws ParseException Thrown when @p str is not a valid proxy string.
        /// @see #proxyToString
        template<typename Prx = ObjectPrx, std::enable_if_t<std::is_base_of_v<ObjectPrx, Prx>, bool> = true>
        std::optional<Prx> stringToProxy(std::string_view str) const
        {
            auto reference = _stringToProxy(str);
            if (reference)
            {
                return Prx::_fromReference(reference);
            }
            else
            {
                return std::nullopt;
            }
        }

        /// Converts a proxy into a string.
        /// @param obj The proxy to convert into a stringified proxy.
        /// @return The stringified proxy, or an empty string if `obj` is nullopt.
        /// @see #stringToProxy
        std::string proxyToString(const std::optional<ObjectPrx>& obj) const;

        /// Converts a set of proxy properties into a proxy. The "base" name supplied in the @p property argument refers
        /// to a property containing a stringified proxy, such as `MyProxy=id:tcp -h localhost -p 10000`. Additional
        /// properties configure local settings for the proxy, such as `MyProxy.PreferSecure=1`. The "Properties"
        /// appendix in the Ice manual describes each of the supported proxy properties.
        /// @tparam Prx The type of the proxy to return.
        /// @param property The base property name.
        /// @return The proxy, or nullopt if the property is not set.
        template<typename Prx = ObjectPrx, std::enable_if_t<std::is_base_of_v<ObjectPrx, Prx>, bool> = true>
        std::optional<Prx> propertyToProxy(std::string_view property) const
        {
            auto reference = _propertyToProxy(property);
            if (reference)
            {
                return Prx::_fromReference(reference);
            }
            else
            {
                return std::nullopt;
            }
        }

        /// Converts a proxy into a set of proxy properties.
        /// @param proxy The proxy.
        /// @param property The base property name.
        /// @return The property set.
        PropertyDict proxyToProperty(const std::optional<ObjectPrx>& proxy, std::string property) const;

        /// Converts an identity into a string.
        /// @param ident The identity to convert into a string.
        /// @return The "stringified" identity.
        [[nodiscard]] std::string identityToString(const Identity& ident) const;

        /// Creates a new object adapter. The endpoints for the object adapter are taken from the property
        /// `name.Endpoints`.
        /// It is legal to create an object adapter with the empty string as its name. Such an object adapter is
        /// accessible via bidirectional connections or by collocated invocations.
        /// @param name The object adapter name.
        /// @param serverAuthenticationOptions The SSL options for server connections.
        /// @return The new object adapter.
        /// @see #createObjectAdapterWithEndpoints
        /// @see ObjectAdapter
        /// @see Properties
        /// @see SSL::OpenSSLServerAuthenticationOptions
        /// @see SSL::SecureTransportServerAuthenticationOptions
        /// @see SSL::SchannelServerAuthenticationOptions
        ObjectAdapterPtr createObjectAdapter(
            std::string name,
            std::optional<SSL::ServerAuthenticationOptions> serverAuthenticationOptions = std::nullopt);

        /// Creates a new object adapter with endpoints. This function sets the property `name.Endpoints`, and then
        /// calls #createObjectAdapter. It is provided as a convenience function. Calling this function with an empty
        /// name will result in a UUID being generated for the name.
        /// @param name The object adapter name.
        /// @param endpoints The endpoints of the object adapter.
        /// @param serverAuthenticationOptions The SSL options for server connections.
        /// @return The new object adapter.
        /// @see #createObjectAdapter
        /// @see ObjectAdapter
        /// @see Properties
        /// @see SSL::OpenSSLServerAuthenticationOptions
        /// @see SSL::SecureTransportServerAuthenticationOptions
        /// @see SSL::SchannelServerAuthenticationOptions
        ObjectAdapterPtr createObjectAdapterWithEndpoints(
            std::string name,
            std::string_view endpoints,
            std::optional<SSL::ServerAuthenticationOptions> serverAuthenticationOptions = std::nullopt);

        /// Creates a new object adapter with a router. This function creates a routed object adapter. Calling this
        /// function with an empty name will result in a UUID being generated for the name.
        /// @param name The object adapter name.
        /// @param rtr The router.
        /// @return The new object adapter.
        /// @see #createObjectAdapter
        /// @see ObjectAdapter
        /// @see Properties
        ObjectAdapterPtr createObjectAdapterWithRouter(std::string name, RouterPrx rtr);

        /// Gets the object adapter that is associated by default with new outgoing connections created by this
        /// communicator. This function returns `nullptr` unless you set a non-null default object adapter using
        /// #setDefaultObjectAdapter.
        /// @return The object adapter associated by default with new outgoing connections.
        /// @throws CommunicatorDestroyedException Thrown when the communicator has been destroyed.
        /// @see Connection::getAdapter
        [[nodiscard]] ObjectAdapterPtr getDefaultObjectAdapter() const;

        /// Sets the object adapter that will be associated with new outgoing connections created by this
        /// communicator. This function has no effect on existing outgoing connections, or on incoming connections.
        /// @param adapter The object adapter to associate with new outgoing connections.
        /// @see Connection::setAdapter
        void setDefaultObjectAdapter(ObjectAdapterPtr adapter);

        /// Gets the implicit context associated with this communicator.
        /// @return The implicit context associated with this communicator; returns `nullptr` when the property
        /// `Ice.ImplicitContext` is not set or is set to `None`.
        [[nodiscard]] ImplicitContextPtr getImplicitContext() const noexcept;

        /// Gets the properties of this communicator.
        /// @return This communicator's properties.
        [[nodiscard]] PropertiesPtr getProperties() const noexcept;

        /// Gets the logger of this communicator.
        /// @return This communicator's logger.
        [[nodiscard]] LoggerPtr getLogger() const noexcept;

        /// Adds a Slice loader to this communicator, after the Slice loader set in InitializationData (if any) and
        /// after other Slice loaders added by this function.
        /// @param loader The Slice loader to add.
        /// @remarks This function is not thread-safe and should only be called right after the communicator is created.
        /// It's provided for applications that cannot set the Slice loader in the InitializationData of the
        /// communicator, such as IceBox services.
        void addSliceLoader(SliceLoaderPtr loader) noexcept;

        /// Gets the observer object of this communicator.
        /// @return This communicator's observer object.
        [[nodiscard]] Instrumentation::CommunicatorObserverPtr getObserver() const noexcept;

        /// Gets the default router of this communicator.
        /// @return The default router of this communicator.
        /// @throws CommunicatorDestroyedException Thrown when the communicator has been destroyed.
        /// @see #setDefaultRouter
        [[nodiscard]] std::optional<RouterPrx> getDefaultRouter() const;

        /// Sets the default router of this communicator. All newly created proxies will use this default router. This
        /// function has no effect on existing proxies.
        /// @param rtr The new default router. Use `nullopt` to remove the default router.
        /// @see #getDefaultRouter
        /// @see #createObjectAdapterWithRouter
        /// @see Router
        void setDefaultRouter(const std::optional<RouterPrx>& rtr);

        /// Gets the default locator of this communicator.
        /// @return The default locator of this communicator.
        /// @see #setDefaultLocator
        /// @see Locator
        [[nodiscard]] std::optional<Ice::LocatorPrx> getDefaultLocator() const;

        /// Sets the default locator of this communicator. All newly created proxies will use this default locator.
        /// This function has no effect on existing proxies or object adapters.
        /// @param loc The new default locator. Use `nullopt` to remove the default locator.
        /// @see #getDefaultLocator
        /// @see Locator
        /// @see ObjectAdapter#setLocator
        void setDefaultLocator(const std::optional<LocatorPrx>& loc);

        /// Gets the plug-in manager of this communicator.
        /// @return This communicator's plug-in manager.
        /// @throws CommunicatorDestroyedException Thrown when the communicator has been destroyed.
        /// @see PluginManager
        [[nodiscard]] PluginManagerPtr getPluginManager() const;

        /// Flushes any pending batch requests of this communicator. This means all batch requests invoked on fixed
        /// proxies for all connections associated with the communicator. Errors that occur while flushing a connection
        /// are ignored.
        /// @param compress Specifies whether or not the queued batch requests should be compressed before being sent
        /// over the wire.
        void flushBatchRequests(CompressBatch compress);

        /// Flushes any pending batch requests of this communicator. This means all batch requests invoked on fixed
        /// proxies for all connections associated with the communicator. Errors that occur while flushing a connection
        /// are ignored.
        /// @param compress Specifies whether or not the queued batch requests should be compressed before being sent
        /// over the wire.
        /// @param exception The exception callback.
        /// @param sent The sent callback.
        /// @return A function that can be called to cancel the flush.
        std::function<void()> flushBatchRequestsAsync(
            CompressBatch compress,
            std::function<void(std::exception_ptr)> exception,
            std::function<void(bool)> sent = nullptr);

        /// Flushes any pending batch requests of this communicator. This means all batch requests invoked on fixed
        /// proxies for all connections associated with the communicator. Errors that occur while flushing a connection
        /// are ignored.
        /// @param compress Specifies whether or not the queued batch requests should be compressed before being sent
        /// over the wire.
        /// @return A future that becomes available when all batch requests have been sent.
        [[nodiscard]] std::future<void> flushBatchRequestsAsync(CompressBatch compress);

        /// Adds the Admin object with all its facets to the provided object adapter. If `Ice.Admin.ServerId`
        /// is set and the provided object adapter has a Locator, #createAdmin registers the Admin's Process facet with
        /// the Locator's LocatorRegistry.
        /// @param adminAdapter The object adapter used to host the Admin object; if it is null and
        /// `Ice.Admin.Endpoints` is set, this function uses the `Ice.Admin` object adapter, after creating and
        /// activating this adapter.
        /// @param adminId The identity of the Admin object.
        /// @return A proxy to the main ("") facet of the Admin object.
        /// @throws InitializationException Thrown when #createAdmin is called more than once.
        /// @see #getAdmin
        ObjectPrx createAdmin(const ObjectAdapterPtr& adminAdapter, const Identity& adminId);

        /// Gets a proxy to the main facet of the Admin object. #getAdmin also creates the Admin object and creates and
        /// activates the `Ice.Admin` object adapter to host this Admin object if `Ice.Admin.Endpoints` is set. The
        /// identity of the Admin object created by getAdmin is `{value of Ice.Admin.InstanceName}/admin`, or
        /// `{UUID}/admin` when `Ice.Admin.InstanceName` is not set. If `Ice.Admin.DelayCreation` is `0` or not set,
        /// #getAdmin is called by the communicator initialization, after initialization of all plugins.
        /// @return A proxy to the main ("") facet of the Admin object, or nullopt if no Admin object is configured.
        /// @throws CommunicatorDestroyedException Thrown when the communicator has been destroyed.
        /// @see #createAdmin
        std::optional<ObjectPrx> getAdmin() const; // NOLINT(modernize-use-nodiscard)

        /// Adds a new facet to the Admin object.
        /// @param servant The servant that implements the new Admin facet.
        /// @param facet The name of the new Admin facet.
        /// @throws AlreadyRegisteredException Thrown when a facet with the same name is already registered.
        void addAdminFacet(ObjectPtr servant, std::string facet);

        /// Removes a facet from the Admin object.
        /// @param facet The name of the Admin facet.
        /// @return The servant associated with this Admin facet.
        /// @throws NotRegisteredException Thrown when no facet with the given name is registered.
        ObjectPtr removeAdminFacet(std::string_view facet);

        /// Returns a facet of the Admin object.
        /// @param facet The name of the Admin facet.
        /// @return The servant associated with this Admin facet, or null if no facet is registered with the given name.
        ObjectPtr findAdminFacet(std::string_view facet);

        /// Returns a map of all facets of the Admin object.
        /// @return A collection containing all the facet names and servants of the Admin object.
        /// @see #findAdminFacet
        FacetMap findAllAdminFacets();

        /// @private
        void postToClientThreadPool(std::function<void()> call);

    private:
        Communicator() = default;

        static CommunicatorPtr create(InitializationData);

        // Certain initialization tasks need to be completed after the constructor.
        void finishSetup(int&, const char*[]);

        [[nodiscard]] IceInternal::ReferencePtr _stringToProxy(std::string_view str) const;
        [[nodiscard]] IceInternal::ReferencePtr _propertyToProxy(std::string_view property) const;

        /// @cond INTERNAL
        friend ICE_API_FRIEND CommunicatorPtr initialize(int&, const char*[], InitializationData);
        friend ICE_API_FRIEND CommunicatorPtr initialize(InitializationData);
        /// @endcond

        friend ICE_API_FRIEND IceInternal::InstancePtr IceInternal::getInstance(const Ice::CommunicatorPtr&);
        friend ICE_API_FRIEND IceInternal::TimerPtr IceInternal::getInstanceTimer(const Ice::CommunicatorPtr&);

        const IceInternal::InstancePtr _instance;
    };

    /// A helper class that uses Resource Acquisition Is Initialization (RAII) to hold a communicator instance, and
    /// automatically destroy this instance when the holder goes out of scope.
    /// @headerfile Ice/Ice.h
    class ICE_API CommunicatorHolder
    {
    public:
        /// Default constructor.
        CommunicatorHolder() = default;

        /// Constructs a CommunicatorHolder that adopts an existing communicator.
        /// @param communicator The communicator to adopt.
        CommunicatorHolder(CommunicatorPtr communicator) noexcept;

        /// Constructs a CommunicatorHolder for a new communicator created using an `Ice::initialize` overload.
        /// @tparam T The types of the arguments to pass to the `Ice::initialize` function.
        /// @param args The arguments to pass to the `Ice::initialize` function.
        template<class... T>
        explicit CommunicatorHolder(T&&... args) : _communicator(std::move(initialize(std::forward<T>(args)...)))
        {
        }

        /// Move constructor. Constructs a CommunicatorHolder with the contents of @p other using move semantics.
        /// @param other The holder to move from.
        CommunicatorHolder(CommunicatorHolder&& other) = default;

        CommunicatorHolder(const CommunicatorHolder&) = delete;

        /// Assignment operator. Destroys the current communicator (if any) and adopts a new communicator.
        /// @param communicator The communicator to adopt.
        CommunicatorHolder& operator=(CommunicatorPtr communicator) noexcept;

        CommunicatorHolder& operator=(const CommunicatorHolder&) noexcept = delete;

        /// Move assignment operator. Destroys the current communicator (if any) and adopts a new communicator.
        /// @param holder The holder from which to adopt a communicator.
        CommunicatorHolder& operator=(CommunicatorHolder&& holder) noexcept;

        /// Determines whether this holder holds a communicator.
        /// @return `true` if the holder currently a holds communicator, `false` otherwise.
        explicit operator bool() const noexcept { return _communicator != nullptr; }

        ~CommunicatorHolder();

        /// Gets the communicator.
        /// @return The communicator held by this holder, or null if the holder is empty.
        [[nodiscard]] const CommunicatorPtr& communicator() const noexcept { return _communicator; }

        /// Gets the communicator.
        /// @return The communicator held by this holder, or null if the holder is empty.
        const CommunicatorPtr& operator->() const noexcept { return _communicator; }

        /// Gets the communicator and clears the reference held by the holder.
        /// @return The communicator held by this holder, or null if the holder is empty.
        CommunicatorPtr release() noexcept { return std::move(_communicator); }

    private:
        CommunicatorPtr _communicator;
    };
}

#endif
