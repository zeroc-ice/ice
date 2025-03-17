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

    /**
     * The central object in Ice. One or more communicators can be instantiated for an Ice application.
     * @see Logger
     * @see ObjectAdapter
     * @see Properties
     * @see ValueFactory
     * \headerfile Ice/Ice.h
     */
    class ICE_API Communicator final : public std::enable_shared_from_this<Communicator>
    {
    public:
        ~Communicator();

        /**
         * Destroy the communicator. This operation calls {@link #shutdown} implicitly. Calling {@link #destroy} cleans
         * up memory, and shuts down this communicator's client functionality and destroys all object adapters.
         * Subsequent calls to {@link #destroy} are ignored.
         * @see #shutdown
         * @see ObjectAdapter#destroy
         */
        void destroy() noexcept;

        /**
         * Shuts down this communicator: call deactivate on all object adapters created by this communicator. Shutting
         * down a communicator has no effect on outgoing connections.
         * @see #destroy
         * @see #waitForShutdown
         * @see ObjectAdapter#deactivate
         */
        void shutdown() noexcept;

        /**
         * Wait until the application has called {@link #shutdown} (or {@link #destroy}). On the server side, this
         * operation blocks the calling thread until all currently-executing operations have completed. On the client
         * side, the operation simply blocks until another thread has called {@link #shutdown} or {@link #destroy}.
         * A typical use of this operation is to call it from the main thread, which then waits until some other thread
         * calls {@link #shutdown}. After shut-down is complete, the main thread returns and can do some cleanup work
         * before it finally calls {@link #destroy} to shut down the client functionality, and then exits the
         * application.
         * @see #shutdown
         * @see #destroy
         * @see ObjectAdapter#waitForDeactivate
         */
        void waitForShutdown() noexcept;

        /// Waits until the communicator is shut down.
        /// @param completed The callback to call when the shutdown is complete. This callback must not throw any
        /// exception.
        /// @remark If you call this function on a communicator that has already been shut down, the callback is called
        /// immediately by the current thread.
        /// @see #shutdown
        void waitForShutdownAsync(std::function<void()> completed) noexcept;

        /**
         * Check whether communicator has been shut down.
         * @return True if the communicator has been shut down; false otherwise.
         * @see #shutdown
         */
        [[nodiscard]] bool isShutdown() const noexcept;

        /**
         * Convert a stringified proxy into a proxy.
         * For example, <code>MyCategory/MyObject:tcp -h some_host -p 10000</code> creates a proxy that refers to the
         * Ice object having an identity with a name "MyObject" and a category "MyCategory", with the server running on
         * host "some_host", port 10000. If the stringified proxy does not parse correctly, the operation throws
         * ParseException. Refer to the Ice manual for a detailed description of the syntax supported by stringified
         * proxies.
         * @param str The stringified proxy to convert into a proxy.
         * @return The proxy, or nullopt if <code>str</code> is an empty string.
         * @see #proxyToString
         */
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

        /**
         * Convert a proxy into a string.
         * @param obj The proxy to convert into a stringified proxy.
         * @return The stringified proxy, or an empty string if <code>obj</code> is nullopt.
         * @see #stringToProxy
         */
        std::string proxyToString(const std::optional<ObjectPrx>& obj) const;

        /**
         * Convert a set of proxy properties into a proxy. The "base" name supplied in the <code>property</code>
         * argument refers to a property containing a stringified proxy, such as <code>MyProxy=id:tcp -h localhost -p
         * 10000</code>. Additional properties configure local settings for the proxy, such as
         * <code>MyProxy.PreferSecure=1</code>. The "Properties" appendix in the Ice manual describes each of the
         * supported proxy properties.
         * @tparam Prx The type of the proxy to return.
         * @param property The base property name.
         * @return The proxy, or nullopt if the property is not set.
         */
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

        /**
         * Convert a proxy to a set of proxy properties.
         * @param proxy The proxy.
         * @param property The base property name.
         * @return The property set.
         */
        PropertyDict proxyToProperty(const std::optional<ObjectPrx>& proxy, std::string property) const;

        /**
         * Convert an identity into a string.
         * @param ident The identity to convert into a string.
         * @return The "stringified" identity.
         */
        [[nodiscard]] std::string identityToString(const Identity& ident) const;

        /**
         * Create a new object adapter. The endpoints for the object adapter are taken from the property
         * <code><em>name</em>.Endpoints</code>.
         * It is legal to create an object adapter with the empty string as its name. Such an object adapter is
         * accessible via bidirectional connections or by collocated invocations that originate from the same
         * communicator as is used by the adapter. Attempts to create a named object adapter for which no configuration
         * can be found raise InitializationException.
         * @param name The object adapter name.
         * @param serverAuthenticationOptions The %SSL configuration properties for server connections.
         * The `SSL::ServerAuthenticationOptions` type is an alias to the platform specific %SSL server authentication
         * options.
         * @return The new object adapter.
         * @see #createObjectAdapterWithEndpoints
         * @see ObjectAdapter
         * @see Properties
         * @see SSL::OpenSSLServerAuthenticationOptions
         * @see SSL::SecureTransportServerAuthenticationOptions
         * @see SSL::SchannelServerAuthenticationOptions

         */
        ObjectAdapterPtr createObjectAdapter(
            std::string name,
            std::optional<SSL::ServerAuthenticationOptions> serverAuthenticationOptions = std::nullopt);

        /**
         * Create a new object adapter with endpoints. This operation sets the property
         * <code><em>name</em>.Endpoints</code>, and then calls {@link #createObjectAdapter}. It is provided as a
         * convenience function. Calling this operation with an empty name will result in a UUID being generated for the
         * name.
         * @param name The object adapter name.
         * @param endpoints The endpoints for the object adapter.
         * @param serverAuthenticationOptions The %SSL configuration properties for server connections.
         * The `SSL::ServerAuthenticationOptions` type is an alias to the platform specific %SSL server authentication
         * options.
         * @return The new object adapter.
         * @see #createObjectAdapter
         * @see ObjectAdapter
         * @see Properties
         * @see SSL::OpenSSLServerAuthenticationOptions
         * @see SSL::SecureTransportServerAuthenticationOptions
         * @see SSL::SchannelServerAuthenticationOptions
         */
        ObjectAdapterPtr createObjectAdapterWithEndpoints(
            std::string name,
            std::string_view endpoints,
            std::optional<SSL::ServerAuthenticationOptions> serverAuthenticationOptions = std::nullopt);

        /**
         * Create a new object adapter with a router. This operation creates a routed object adapter.
         * Calling this operation with an empty name will result in a UUID being generated for the name.
         * @param name The object adapter name.
         * @param rtr The router.
         * @return The new object adapter.
         * @see #createObjectAdapter
         * @see ObjectAdapter
         * @see Properties
         */
        ObjectAdapterPtr createObjectAdapterWithRouter(std::string name, RouterPrx rtr);

        /**
         * Gets the object adapter that is associated by default with new outgoing connections created by this
         * communicator. This function returns null unless you set a non-null default object adapter using
         * setDefaultObjectAdapter.
         * @return The object adapter associated by default with new outgoing connections.
         * @see Connection::getAdapter
         */
        [[nodiscard]] ObjectAdapterPtr getDefaultObjectAdapter() const;

        /**
         * Sets the object adapter that will be associated with new outgoing connections created by this
         * communicator. This function has no effect on existing outgoing connections, or on incoming connections.
         * @param adapter The object adapter to associate with new outgoing connections.
         * @see Connection::setAdapter
         */
        void setDefaultObjectAdapter(ObjectAdapterPtr adapter);

        /**
         * Get the implicit context associated with this communicator.
         * @return The implicit context associated with this communicator; returns null when the property
         * Ice.ImplicitContext is not set or is set to None.
         */
        [[nodiscard]] ImplicitContextPtr getImplicitContext() const noexcept;

        /**
         * Get the properties for this communicator.
         * @return This communicator's properties.
         * @see Properties
         */
        [[nodiscard]] PropertiesPtr getProperties() const noexcept;

        /**
         * Get the logger for this communicator.
         * @return This communicator's logger.
         * @see Logger
         */
        [[nodiscard]] LoggerPtr getLogger() const noexcept;

        /**
         * Get the observer resolver object for this communicator.
         * @return This communicator's observer resolver object.
         */
        [[nodiscard]] Instrumentation::CommunicatorObserverPtr getObserver() const noexcept;

        /**
         * Get the default router for this communicator.
         * @return The default router for this communicator.
         * @see #setDefaultRouter
         * @see Router
         */
        [[nodiscard]] std::optional<RouterPrx> getDefaultRouter() const;

        /**
         * Set a default router for this communicator. All newly created proxies will use this default router. To
         * disable the default router, null can be used. Note that this operation has no effect on existing proxies. You
         * can also set a router for an individual proxy by calling the operation <code>ice_router</code> on the proxy.
         * @param rtr The default router to use for this communicator.
         * @see #getDefaultRouter
         * @see #createObjectAdapterWithRouter
         * @see Router
         */
        void setDefaultRouter(const std::optional<RouterPrx>& rtr);

        /**
         * Get the default locator for this communicator.
         * @return The default locator for this communicator.
         * @see #setDefaultLocator
         * @see Locator
         */
        [[nodiscard]] std::optional<Ice::LocatorPrx> getDefaultLocator() const;

        /**
         * Set a default Ice locator for this communicator. All newly created proxy and object adapters will use this
         * default locator. To disable the default locator, null can be used. Note that this operation has no effect on
         * existing proxies or object adapters.
         * You can also set a locator for an individual proxy by calling the operation <code>ice_locator</code> on the
         * proxy, or for an object adapter by calling {@link ObjectAdapter#setLocator} on the object adapter.
         * @param loc The default locator to use for this communicator.
         * @see #getDefaultLocator
         * @see Locator
         * @see ObjectAdapter#setLocator
         */
        void setDefaultLocator(const std::optional<LocatorPrx>& loc);

        /**
         * Get the plug-in manager for this communicator.
         * @return This communicator's plug-in manager.
         * @see PluginManager
         */
        [[nodiscard]] PluginManagerPtr getPluginManager() const;

        /**
         * Get the value factory manager for this communicator.
         * @return This communicator's value factory manager.
         * @see ValueFactoryManager
         */
        [[nodiscard]] ValueFactoryManagerPtr getValueFactoryManager() const noexcept;

        /**
         * Flush any pending batch requests for this communicator. This means all batch requests invoked on fixed
         * proxies for all connections associated with the communicator. Any errors that occur while flushing a
         * connection are ignored.
         * @param compress Specifies whether or not the queued batch requests should be compressed before being sent
         * over the wire.
         */
        void flushBatchRequests(CompressBatch compress);

        /**
         * Flush any pending batch requests for this communicator. This means all batch requests invoked on fixed
         * proxies for all connections associated with the communicator. Any errors that occur while flushing a
         * connection are ignored.
         * @param compress Specifies whether or not the queued batch requests should be compressed before being sent
         * over the wire.
         * @param exception The exception callback.
         * @param sent The sent callback.
         * @return A function that can be called to cancel the invocation locally.
         */
        std::function<void()> flushBatchRequestsAsync(
            CompressBatch compress,
            std::function<void(std::exception_ptr)> exception,
            std::function<void(bool)> sent = nullptr);

        /**
         * Flush any pending batch requests for this communicator. This means all batch requests invoked on fixed
         * proxies for all connections associated with the communicator. Any errors that occur while flushing a
         * connection are ignored.
         * @param compress Specifies whether or not the queued batch requests should be compressed before being sent
         * over the wire.
         * @return The future object for the invocation.
         */
        [[nodiscard]] std::future<void> flushBatchRequestsAsync(CompressBatch compress);

        /**
         * Add the Admin object with all its facets to the provided object adapter. If <code>Ice.Admin.ServerId</code>
         * is set and the provided object adapter has a {@link Locator}, createAdmin registers the Admin's Process facet
         * with the {@link Locator}'s {@link LocatorRegistry}. createAdmin must only be called once; subsequent calls
         * raise InitializationException.
         * @param adminAdapter The object adapter used to host the Admin object; if null and Ice.Admin.Endpoints is set,
         * create, activate and use the Ice.Admin object adapter.
         * @param adminId The identity of the Admin object.
         * @return A proxy to the main ("") facet of the Admin object.
         * @see #getAdmin
         */
        ObjectPrx createAdmin(const ObjectAdapterPtr& adminAdapter, const Identity& adminId);

        /**
         * Get a proxy to the main facet of the Admin object. getAdmin also creates the Admin object and creates and
         * activates the Ice.Admin object adapter to host this Admin object if Ice.Admin.Endpoints is set. The identity
         * of the Admin object created by getAdmin is {value of Ice.Admin.InstanceName}/admin, or {UUID}/admin when
         * <code>Ice.Admin.InstanceName</code> is not set. If Ice.Admin.DelayCreation is 0 or not set, getAdmin is
         * called by the communicator initialization, after initialization of all plugins.
         * @return A proxy to the main ("") facet of the Admin object, or nullopt if no Admin object is configured.
         * @see #createAdmin
         */
        std::optional<ObjectPrx> getAdmin() const; // NOLINT(modernize-use-nodiscard)

        /**
         * Add a new facet to the Admin object. Adding a servant with a facet that is already registered throws
         * AlreadyRegisteredException.
         * @param servant The servant that implements the new Admin facet.
         * @param facet The name of the new Admin facet.
         */
        void addAdminFacet(ObjectPtr servant, std::string facet);

        /**
         * Remove the following facet to the Admin object. Removing a facet that was not previously registered throws
         * NotRegisteredException.
         * @param facet The name of the Admin facet.
         * @return The servant associated with this Admin facet.
         */
        ObjectPtr removeAdminFacet(std::string_view facet);

        /**
         * Returns a facet of the Admin object.
         * @param facet The name of the Admin facet.
         * @return The servant associated with this Admin facet, or null if no facet is registered with the given name.
         */
        ObjectPtr findAdminFacet(std::string_view facet);

        /**
         * Returns a map of all facets of the Admin object.
         * @return A collection containing all the facet names and servants of the Admin object.
         * @see #findAdminFacet
         */
        FacetMap findAllAdminFacets();

        /// @private
        void postToClientThreadPool(std::function<void()> call);

    private:
        static CommunicatorPtr create(InitializationData);

        //
        // Certain initialization tasks need to be completed after the
        // constructor.
        //
        void finishSetup(int&, const char*[]);

        [[nodiscard]] IceInternal::ReferencePtr _stringToProxy(std::string_view str) const;
        [[nodiscard]] IceInternal::ReferencePtr _propertyToProxy(std::string_view property) const;

        friend ICE_API CommunicatorPtr initialize(int&, const char*[], InitializationData);
        friend ICE_API CommunicatorPtr initialize(StringSeq&, InitializationData);
        friend ICE_API CommunicatorPtr initialize(InitializationData);
        friend ICE_API IceInternal::InstancePtr IceInternal::getInstance(const Ice::CommunicatorPtr&);
        friend ICE_API IceInternal::TimerPtr IceInternal::getInstanceTimer(const Ice::CommunicatorPtr&);

        const IceInternal::InstancePtr _instance;
    };
}

#endif
