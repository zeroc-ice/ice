// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import com.zeroc.Ice.Instrumentation.CommunicatorObserver;
import com.zeroc.Ice.SSL.SSLEngineFactory;

import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.UUID;
import java.util.concurrent.CompletableFuture;

/**
 * The central object in Ice. One or more communicators can be instantiated for an Ice application.
 * Communicator instantiation is language-specific, and not specified in Slice code.
 *
 * @see Logger
 * @see ObjectAdapter
 * @see Properties
 */
public final class Communicator implements AutoCloseable {

    private final Instance _instance;

    /**
     * Destroy the communicator. This Java-only method overrides close in java.lang.AutoCloseable
     * and does not throw any exception.
     *
     * @see #destroy
     */
    public void close() {
        _instance.destroy(
            false); // Don't allow destroy to be interrupted if called from try with statement.
    }

    /**
     * Destroy the communicator. This operation calls {@link #shutdown} implicitly. Calling {@link
     * #destroy} cleans up memory, and shuts down this communicator's client functionality and
     * destroys all object adapters. Subsequent calls to {@link #destroy} are ignored.
     *
     * @see #shutdown
     * @see ObjectAdapter#destroy
     */
    public void destroy() {
        _instance.destroy(true); // Destroy is interruptible when call explicitly.
    }

    /**
     * Shuts down this communicator: call {@link ObjectAdapter#deactivate} on all object adapters
     * created by this communicator. Shutting down a communicator has no effect on outgoing
     * connections.
     *
     * @see #destroy
     * @see #waitForShutdown
     * @see ObjectAdapter#deactivate
     */
    public void shutdown() {
        try {
            _instance.objectAdapterFactory().shutdown();
        } catch (CommunicatorDestroyedException ex) {
            // Ignore
        }
    }

    /**
     * Wait until the application has called {@link #shutdown} (or {@link #destroy}). On the server
     * side, this operation blocks the calling thread until all currently-executing operations have
     * completed. On the client side, the operation simply blocks until another thread has called
     * {@link #shutdown} or {@link #destroy}. A typical use of this operation is to call it from the
     * main thread, which then waits until some other thread calls {@link #shutdown}. After
     * shut-down is complete, the main thread returns and can do some cleanup work before it finally
     * calls {@link #destroy} to shut down the client functionality, and then exits the application.
     *
     * @see #shutdown
     * @see #destroy
     * @see ObjectAdapter#waitForDeactivate
     */
    public void waitForShutdown() {
        try {
            _instance.objectAdapterFactory().waitForShutdown();
        } catch (CommunicatorDestroyedException ex) {
            // Ignore
        }
    }

    /**
     * Check whether communicator has been shut down.
     *
     * @return True if the communicator has been shut down; false otherwise.
     * @see #shutdown
     */
    public boolean isShutdown() {
        try {
            return _instance.objectAdapterFactory().isShutdown();
        } catch (CommunicatorDestroyedException ex) {
            return true;
        }
    }

    /**
     * Convert a stringified proxy into a proxy. For example, <code>
     * MyCategory/MyObject:tcp -h some_host -p 10000</code> creates a proxy that refers to the Ice
     * object having an identity with a name "MyObject" and a category "MyCategory", with the server
     * running on host "some_host", port 10000. If the stringified proxy does not parse correctly,
     * the operation throws ParseException. Refer to the Ice manual for a detailed description of
     * the syntax supported by stringified proxies.
     *
     * @param str The stringified proxy to convert into a proxy.
     * @return The proxy, or nil if <code>str</code> is an empty string.
     * @see #proxyToString
     */
    public ObjectPrx stringToProxy(String str) {
        var ref = _instance.referenceFactory().create(str, null);
        return ref == null ? null : new _ObjectPrxI(ref);
    }

    /**
     * Convert a proxy into a string.
     *
     * @param proxy The proxy to convert into a stringified proxy.
     * @return The stringified proxy, or an empty string if <code>obj</code> is nil.
     * @see #stringToProxy
     */
    public String proxyToString(ObjectPrx proxy) {
        return proxy == null ? "" : proxy._getReference().toString();
    }

    /**
     * Convert a set of proxy properties into a proxy. The "base" name supplied in the <code>
     * property
     * </code> argument refers to a property containing a stringified proxy, such as <code>
     * MyProxy=id:tcp -h localhost -p 10000</code>. Additional properties configure local settings
     * for the proxy, such as <code>MyProxy.PreferSecure=1</code>. The "Properties" appendix in the
     * Ice manual describes each of the supported proxy properties.
     *
     * @param prefix The base property name.
     * @return The proxy.
     */
    public ObjectPrx propertyToProxy(String prefix) {
        String proxy = _instance.initializationData().properties.getProperty(prefix);
        var ref = _instance.referenceFactory().create(proxy, prefix);
        return ref == null ? null : new _ObjectPrxI(ref);
    }

    /**
     * Convert a proxy to a set of proxy properties.
     *
     * @param proxy The proxy.
     * @param prefix The base property name.
     * @return The property set.
     */
    public Map<String, String> proxyToProperty(ObjectPrx proxy, String prefix) {
        return proxy == null
            ? new HashMap<>()
            : proxy._getReference().toProperty(prefix);
    }

    /**
     * Convert an identity into a string.
     *
     * @param ident The identity to convert into a string.
     * @return The "stringified" identity.
     */
    public String identityToString(Identity ident) {
        return Util.identityToString(ident, _instance.toStringMode());
    }

    /**
     * Create a new object adapter. The endpoints for the object adapter are taken from the property
     * <code><em>name</em>.Endpoints</code>. It is legal to create an object adapter with the empty
     * string as its name. Such an object adapter is accessible via bidirectional connections or by
     * collocated invocations that originate from the same communicator as is used by the adapter.
     * Attempts to create a named object adapter for which no configuration can be found raise
     * InitializationException.
     *
     * @param name The object adapter name.
     * @return The new object adapter.
     * @see #createObjectAdapterWithEndpoints
     * @see ObjectAdapter
     * @see Properties
     */
    public ObjectAdapter createObjectAdapter(String name) {
        return createObjectAdapter(name, null);
    }

    /**
     * Create a new object adapter. The endpoints for the object adapter are taken from the property
     * <code><em>name</em>.Endpoints</code>. It is legal to create an object adapter with the empty
     * string as its name. Such an object adapter is accessible via bidirectional connections or by
     * collocated invocations that originate from the same communicator as is used by the adapter.
     * Attempts to create a named object adapter for which no configuration can be found raise
     * InitializationException.
     *
     * <p>It is an error to pass a non-null sslEngineFactory when the name is empty, this raises
     * IllegalArgumentException.
     *
     * @param name The object adapter name.
     * @param sslEngineFactory The SSL engine factory used by the server-side ssl transport of the
     *     new object adapter. When set to a non-null value all Ice.SSL configuration properties are
     *     ignored, and any SSL configuration must be done through the SSLEngineFactory. Pass null
     *     if the object adapter does not use secure endpoints, or if the ssl transport is
     *     configured through Ice.SSL configuration properties. Passing null is equivalent to
     *     calling {@link #createObjectAdapterWithEndpoints(String, String)}.
     * @return The new object adapter.
     * @see #createObjectAdapterWithEndpoints
     * @see ObjectAdapter
     * @see Properties
     */
    public ObjectAdapter createObjectAdapter(String name, SSLEngineFactory sslEngineFactory) {
        if (name.isEmpty() && sslEngineFactory != null) {
            throw new IllegalArgumentException(
                "name cannot be empty when using an SSLEngineFactory");
        }
        return _instance.objectAdapterFactory().createObjectAdapter(name, null, sslEngineFactory);
    }

    /**
     * Create a new object adapter with endpoints. This operation sets the property <code>
     * <em>name</em>.Endpoints</code>, and then calls {@link #createObjectAdapter}. It is provided
     * as a convenience function. Calling this operation with an empty name will result in a UUID
     * being generated for the name.
     *
     * @param name The object adapter name.
     * @param endpoints The endpoints for the object adapter.
     * @return The new object adapter.
     * @see #createObjectAdapter
     * @see ObjectAdapter
     * @see Properties
     */
    public ObjectAdapter createObjectAdapterWithEndpoints(String name, String endpoints) {
        return createObjectAdapterWithEndpoints(name, endpoints, null);
    }

    /**
     * Create a new object adapter with endpoints. This operation sets the property <code>
     * <em>name</em>.Endpoints</code>, and then calls {@link #createObjectAdapter}. It is provided
     * as a convenience function. Calling this operation with an empty name will result in a UUID
     * being generated for the name.
     *
     * @param name The object adapter name.
     * @param endpoints The endpoints for the object adapter.
     * @param sslEngineFactory The SSL engine factory used by the server-side ssl transport of the
     *     new object adapter. When set to a non-null value all Ice.SSL configuration properties are
     *     ignored, and any SSL configuration must be done through the SSLEngineFactory. Pass null
     *     if the object adapter does not use secure endpoints, or if the ssl transport is
     *     configured through Ice.SSL configuration properties. Passing null is equivalent to
     *     calling {@link #createObjectAdapterWithEndpoints(String, String)}.
     * @return The new object adapter.
     * @see #createObjectAdapter
     * @see ObjectAdapter
     * @see Properties
     */
    public ObjectAdapter createObjectAdapterWithEndpoints(
            String name, String endpoints, SSLEngineFactory sslEngineFactory) {
        if (name.isEmpty()) {
            name = UUID.randomUUID().toString();
        }

        getProperties().setProperty(name + ".Endpoints", endpoints);
        return _instance.objectAdapterFactory().createObjectAdapter(name, null, sslEngineFactory);
    }

    /**
     * Create a new object adapter with a router. This operation creates a routed object adapter.
     * Calling this operation with an empty name will result in a UUID being generated for the name.
     *
     * @param name The object adapter name.
     * @param router The router.
     * @return The new object adapter.
     * @see #createObjectAdapter
     * @see ObjectAdapter
     * @see Properties
     */
    public ObjectAdapter createObjectAdapterWithRouter(String name, RouterPrx router) {
        if (name.isEmpty()) {
            name = UUID.randomUUID().toString();
        }

        //
        // We set the proxy properties here, although we still use the proxy supplied.
        //
        Map<String, String> properties = proxyToProperty(router, name + ".Router");
        for (Map.Entry<String, String> p : properties.entrySet()) {
            getProperties().setProperty(p.getKey(), p.getValue());
        }

        return _instance.objectAdapterFactory().createObjectAdapter(name, router, null);
    }

    /**
     * Gets the object adapter that is associated by default with new outgoing connections created
     * by this communicator. This method returns null unless you set a non-null default object
     * adapter using {@link setDefaultObjectAdapter}.
     *
     * @return The object adapter associated by default with new outgoing connections.
     * @see Connection#getAdapter
     */
    public ObjectAdapter getDefaultObjectAdapter() {
        return _instance.outgoingConnectionFactory().getDefaultObjectAdapter();
    }

    /**
     * Sets the object adapter that will be associated with new outgoing connections created by this
     * communicator. This method has no effect on existing outgoing connections, or on incoming
     * connections.
     *
     * @param adapter The object adapter to associate with new outgoing connections.
     * @see Connection#setAdapter
     */
    public void setDefaultObjectAdapter(ObjectAdapter adapter) {
        _instance.outgoingConnectionFactory().setDefaultObjectAdapter(adapter);
    }

    /**
     * Get the implicit context associated with this communicator.
     *
     * @return The implicit context associated with this communicator; returns null when the
     *     property Ice.ImplicitContext is not set or is set to None.
     */
    public ImplicitContext getImplicitContext() {
        return _instance.getImplicitContext();
    }

    /**
     * Get the properties for this communicator.
     *
     * @return This communicator's properties.
     * @see Properties
     */
    public Properties getProperties() {
        return _instance.initializationData().properties;
    }

    /**
     * Get the logger for this communicator.
     *
     * @return This communicator's logger.
     * @see Logger
     */
    public Logger getLogger() {
        return _instance.initializationData().logger;
    }

    /**
     * Adds a Slice loader to this communicator, after the Slice loader set in {@link InitializationData}
     * (if any) and after other Slice loaders added by this method.
     *
     * <p>This method is not thread-safe and should only be called right after the communicator is created.
     * It's provided for applications that cannot set the Slice loader in the {@link InitializationData} of the
     * communicator, such as IceBox services.</p>
     *
     * @param loader The Slice loader to add.
     */
    public void addSliceLoader(SliceLoader loader) {
        _instance.addSliceLoader(loader);
    }

    /**
     * Get the observer resolver object for this communicator.
     *
     * @return This communicator's observer resolver object.
     */
    public CommunicatorObserver getObserver() {
        return _instance.initializationData().observer;
    }

    /**
     * Get the default router for this communicator.
     *
     * @return The default router for this communicator.
     * @see #setDefaultRouter
     * @see Router
     */
    public RouterPrx getDefaultRouter() {
        return _instance.referenceFactory().getDefaultRouter();
    }

    /**
     * Set a default router for this communicator. All newly created proxies will use this default
     * router. To disable the default router, null can be used. Note that this operation has no
     * effect on existing proxies. You can also set a router for an individual proxy by calling the
     * operation <code>ice_router</code> on the proxy.
     *
     * @param router The default router to use for this communicator.
     * @see #getDefaultRouter
     * @see #createObjectAdapterWithRouter
     * @see Router
     */
    public void setDefaultRouter(RouterPrx router) {
        _instance.setDefaultRouter(router);
    }

    /**
     * Get the default locator for this communicator.
     *
     * @return The default locator for this communicator.
     * @see #setDefaultLocator
     * @see Locator
     */
    public LocatorPrx getDefaultLocator() {
        return _instance.referenceFactory().getDefaultLocator();
    }

    /**
     * Set a default Ice locator for this communicator. All newly created proxy and object adapters
     * will use this default locator. To disable the default locator, null can be used. Note that
     * this operation has no effect on existing proxies or object adapters. You can also set a
     * locator for an individual proxy by calling the operation <code>ice_locator</code> on the
     * proxy, or for an object adapter by calling {@link ObjectAdapter#setLocator} on the object
     * adapter.
     *
     * @param locator The default locator to use for this communicator.
     * @see #getDefaultLocator
     * @see Locator
     * @see ObjectAdapter#setLocator
     */
    public void setDefaultLocator(LocatorPrx locator) {
        _instance.setDefaultLocator(locator);
    }

    /**
     * Get the plug-in manager for this communicator.
     *
     * @return This communicator's plug-in manager.
     * @see PluginManager
     */
    public PluginManager getPluginManager() {
        return _instance.pluginManager();
    }

    /**
     * Flush any pending batch requests for this communicator. This means all batch requests invoked
     * on fixed proxies for all connections associated with the communicator. Any errors that occur
     * while flushing a connection are ignored.
     *
     * @param compressBatch Specifies whether or not the queued batch requests should be compressed
     *     before being sent over the wire.
     */
    public void flushBatchRequests(CompressBatch compressBatch) {
        _iceI_flushBatchRequestsAsync(compressBatch).waitForResponse();
    }

    /**
     * Flush any pending batch requests for this communicator. This means all batch requests invoked
     * on fixed proxies for all connections associated with the communicator. Any errors that occur
     * while flushing a connection are ignored.
     *
     * @param compressBatch Specifies whether or not the queued batch requests should be compressed
     *     before being sent over the wire.
     * @return A future that will be completed when the invocation completes.
     */
    public CompletableFuture<Void> flushBatchRequestsAsync(
            CompressBatch compressBatch) {
        return _iceI_flushBatchRequestsAsync(compressBatch);
    }

    private CommunicatorFlushBatch _iceI_flushBatchRequestsAsync(CompressBatch compressBatch) {
        //
        // This callback object receives the results of all invocations
        // of Connection.begin_flushBatchRequests.
        //
        var f = new CommunicatorFlushBatch(this, _instance);
        f.invoke(compressBatch);
        return f;
    }

    /**
     * Add the Admin object with all its facets to the provided object adapter. If <code>
     * Ice.Admin.ServerId</code> is set and the provided object adapter has a {@link Locator},
     * createAdmin registers the Admin's Process facet with the {@link Locator}'s {@link
     * LocatorRegistry}. createAdmin must only be called once; subsequent calls raise
     * InitializationException.
     *
     * @param adminAdapter The object adapter used to host the Admin object; if null and
     *     Ice.Admin.Endpoints is set, create, activate and use the Ice.Admin object adapter.
     * @param adminId The identity of the Admin object.
     * @return A proxy to the main ("") facet of the Admin object. Never returns a null proxy.
     * @see #getAdmin
     */
    public ObjectPrx createAdmin(ObjectAdapter adminAdapter, Identity adminId) {
        return _instance.createAdmin(adminAdapter, adminId);
    }

    /**
     * Get a proxy to the main facet of the Admin object. getAdmin also creates the Admin object and
     * creates and activates the Ice.Admin object adapter to host this Admin object if
     * Ice.Admin.Endpoints is set. The identity of the Admin object created by getAdmin is {value of
     * Ice.Admin.InstanceName}/admin, or {UUID}/admin when <code>Ice.Admin.InstanceName</code> is
     * not set. If Ice.Admin.DelayCreation is 0 or not set, getAdmin is called by the communicator
     * initialization, after initialization of all plugins.
     *
     * @return A proxy to the main ("") facet of the Admin object, or a null proxy if no Admin
     *     object is configured.
     * @see #createAdmin
     */
    public ObjectPrx getAdmin() {
        return _instance.getAdmin();
    }

    /**
     * Add a new facet to the Admin object. Adding a servant with a facet that is already registered
     * throws AlreadyRegisteredException.
     *
     * @param servant The servant that implements the new Admin facet.
     * @param facet The name of the new Admin facet.
     */
    public void addAdminFacet(Object servant, String facet) {
        _instance.addAdminFacet(servant, facet);
    }

    /**
     * Remove the following facet to the Admin object. Removing a facet that was not previously
     * registered throws NotRegisteredException.
     *
     * @param facet The name of the Admin facet.
     * @return The servant associated with this Admin facet.
     */
    public Object removeAdminFacet(String facet) {
        return _instance.removeAdminFacet(facet);
    }

    /**
     * Returns a facet of the Admin object.
     *
     * @param facet The name of the Admin facet.
     * @return The servant associated with this Admin facet, or null if no facet is registered with
     *     the given name.
     */
    public Object findAdminFacet(String facet) {
        return _instance.findAdminFacet(facet);
    }

    /**
     * Returns a map of all facets of the Admin object.
     *
     * @return A collection containing all the facet names and servants of the Admin object.
     * @see #findAdminFacet
     */
    public Map<String, Object> findAllAdminFacets() {
        return _instance.findAllAdminFacets();
    }

    Communicator(InitializationData initData) {
        _instance = new Instance();
        _instance.initialize(this, initData);
    }

    /**
     * For compatibility with C#, we do not invoke methods on other objects from within a finalizer.
     *
     * <p>protected synchronized void finalize() throws Throwable { if(!_instance.destroyed()) {
     * _instance.logger().warning("Ice::Communicator::destroy() has not been called"); }
     *
     * <p>super.finalize(); }
     */

    //
    // Certain initialization tasks need to be completed after the constructor.
    //
    void finishSetup(String[] args, List<String> rArgs) {
        try {
            args = _instance.finishSetup(args, this);
            if (rArgs != null) {
                rArgs.clear();
                if (args.length > 0) {
                    rArgs.addAll(Arrays.asList(args));
                }
            }
        } catch (RuntimeException ex) {
            _instance.destroy(false);
            throw ex;
        }
    }

    public Instance getInstance() {
        return _instance;
    }
}
