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
 * The central object in Ice. Its responsibilities include:
 * - creating and managing outgoing connections
 * - executing callbacks in its client thread pool
 * - creating and destroying object adapters
 * - loading plug-ins
 * - managing properties (configuration), retries, logging, instrumentation, and more.
 * You create a communicator with {@code Ice.initialize}, and it's usually the first object you create when programming
 * with Ice. You can create multiple communicators in a single program, but this is not common.
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
     * Destroys this communicator. This method calls {@link #shutdown} implicitly. Calling {@link
     * #destroy} destroys all object adapters, and closes all outgoing connections. {@code destroy} waits for all
     * outstanding dispatches to complete before returning. This includes "bidirectional dispatches" that execute on
     * outgoing connections.
     *
     * @see #shutdown
     * @see ObjectAdapter#destroy
     */
    public void destroy() {
        _instance.destroy(true); // Destroy is interruptible when call explicitly.
    }

    /**
     * Shuts down this communicator. This method calls {@link ObjectAdapter#deactivate} on all object adapters
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
     * Waits for shutdown to complete. This method calls {@link ObjectAdapter#waitForDeactivate} on all object adapters
     * created by this communicator. In a client application that does not accept incoming connections, this
     * method returns as soon as another thread calls {@link #shutdown} or {@link #destroy} on this communicator.
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
     * Checks whether or not {@link #shutdown} was called on this communicator.
     *
     * @return {@code true} if {@link #shutdown} was called on this communicator, {@code false} otherwise.
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
     * Converts a stringified proxy into a proxy.
     *
     * @param str The stringified proxy to convert into a proxy.
     * @return The proxy, or null if {@code str} is an empty string.
     * @throws ParseException Thrown when {@code str} is not a valid proxy string.
     * @see #proxyToString
     */
    public ObjectPrx stringToProxy(String str) {
        var ref = _instance.referenceFactory().create(str, null);
        return ref == null ? null : new _ObjectPrxI(ref);
    }

    /**
     * Converts a proxy into a string.
     *
     * @param proxy The proxy to convert into a stringified proxy.
     * @return The stringified proxy, or an empty string if {@code proxy} is null.
     * @see #stringToProxy
     */
    public String proxyToString(ObjectPrx proxy) {
        return proxy == null ? "" : proxy._getReference().toString();
    }

    /**
     * Converts a set of proxy properties into a proxy. The "base" name supplied in the {@code
     * prefix} argument refers to a property containing a stringified proxy, such as {@code
     * MyProxy=id:tcp -h localhost -p 10000}. Additional properties configure local settings
     * for the proxy, such as {@code MyProxy.PreferSecure=1}. The "Properties" appendix in the
     * Ice manual describes each of the supported proxy properties.
     *
     * @param prefix The base property name.
     * @return The proxy, or null if the property is not set.
     */
    public ObjectPrx propertyToProxy(String prefix) {
        String proxy = _instance.initializationData().properties.getProperty(prefix);
        var ref = _instance.referenceFactory().create(proxy, prefix);
        return ref == null ? null : new _ObjectPrxI(ref);
    }

    /**
     * Converts a proxy into a set of proxy properties.
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
     * Converts an identity into a string.
     *
     * @param ident The identity to convert into a string.
     * @return The "stringified" identity.
     */
    public String identityToString(Identity ident) {
        return Util.identityToString(ident, _instance.toStringMode());
    }

    /**
     * Creates a new object adapter. The endpoints for the object adapter are taken from the property
     * {@code name.Endpoints}.
     *
     * It is legal to create an object adapter with the empty string as its name. Such an object adapter is
     * accessible via bidirectional connections or by collocated invocations.
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
     * Creates a new object adapter. The endpoints for the object adapter are taken from the property
     * {@code name.Endpoints}.
     *
     * It is legal to create an object adapter with the empty string as its name. Such an object adapter is
     * accessible via bidirectional connections or by collocated invocations.
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
     * Creates a new object adapter with endpoints. This method sets the property
     * {@code name.Endpoints}, and then calls {@link #createObjectAdapter}. It is provided
     * as a convenience method. Calling this method with an empty name will result in a UUID
     * being generated for the name.
     *
     * @param name The object adapter name.
     * @param endpoints The endpoints of the object adapter.
     * @return The new object adapter.
     * @see #createObjectAdapter
     * @see ObjectAdapter
     * @see Properties
     */
    public ObjectAdapter createObjectAdapterWithEndpoints(String name, String endpoints) {
        return createObjectAdapterWithEndpoints(name, endpoints, null);
    }

    /**
     * Creates a new object adapter with endpoints. This method sets the property
     * {@code name.Endpoints}, and then calls {@link #createObjectAdapter}. It is provided
     * as a convenience method. Calling this method with an empty name will result in a UUID
     * being generated for the name.
     *
     * @param name The object adapter name.
     * @param endpoints The endpoints of the object adapter.
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
     * Creates a new object adapter with a router. This method creates a routed object adapter.
     * Calling this method with an empty name will result in a UUID being generated for the name.
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
     * Gets the implicit context associated with this communicator.
     *
     * @return The implicit context associated with this communicator; returns null when the
     *     property {@code Ice.ImplicitContext} is not set or is set to {@code None}.
     */
    public ImplicitContext getImplicitContext() {
        return _instance.getImplicitContext();
    }

    /**
     * Gets the properties of this communicator.
     *
     * @return This communicator's properties.
     * @see Properties
     */
    public Properties getProperties() {
        return _instance.initializationData().properties;
    }

    /**
     * Gets the logger of this communicator.
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
     * Gets the observer object of this communicator.
     *
     * @return This communicator's observer object.
     */
    public CommunicatorObserver getObserver() {
        return _instance.initializationData().observer;
    }

    /**
     * Gets the default router of this communicator.
     *
     * @return The default router of this communicator.
     * @see #setDefaultRouter
     * @see Router
     */
    public RouterPrx getDefaultRouter() {
        return _instance.referenceFactory().getDefaultRouter();
    }

    /**
     * Sets the default router of this communicator. All newly created proxies will use this default router.
     * This method has no effect on existing proxies.
     *
     * @param router The new default router. Use null to remove the default router.
     * @see #getDefaultRouter
     * @see #createObjectAdapterWithRouter
     * @see Router
     */
    public void setDefaultRouter(RouterPrx router) {
        _instance.setDefaultRouter(router);
    }

    /**
     * Gets the default locator of this communicator.
     *
     * @return The default locator of this communicator.
     * @see #setDefaultLocator
     * @see Locator
     */
    public LocatorPrx getDefaultLocator() {
        return _instance.referenceFactory().getDefaultLocator();
    }

    /**
     * Sets the default locator of this communicator. All newly created proxies will use this default locator.
     * This method has no effect on existing proxies or object adapters.
     *
     * @param locator The new default locator. Use null to remove the default locator.
     * @see #getDefaultLocator
     * @see Locator
     * @see ObjectAdapter#setLocator
     */
    public void setDefaultLocator(LocatorPrx locator) {
        _instance.setDefaultLocator(locator);
    }

    /**
     * Gets the plug-in manager of this communicator.
     *
     * @return This communicator's plug-in manager.
     * @see PluginManager
     */
    public PluginManager getPluginManager() {
        return _instance.pluginManager();
    }

    /**
     * Flushes any pending batch requests of this communicator. This means all batch requests invoked
     * on fixed proxies for all connections associated with the communicator. Errors that occur
     * while flushing a connection are ignored.
     *
     * @param compressBatch Specifies whether or not the queued batch requests should be compressed
     *     before being sent over the wire.
     */
    public void flushBatchRequests(CompressBatch compressBatch) {
        _iceI_flushBatchRequestsAsync(compressBatch).waitForResponse();
    }

    /**
     * Flushes any pending batch requests of this communicator. This means all batch requests invoked
     * on fixed proxies for all connections associated with the communicator. Errors that occur
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
     * Adds the Admin object with all its facets to the provided object adapter. If
     * {@code Ice.Admin.ServerId} is set and the provided object adapter has a {@link Locator},
     * createAdmin registers the Admin's Process facet with the {@link Locator}'s {@link
     * LocatorRegistry}.
     *
     * @param adminAdapter The object adapter used to host the Admin object; if null and
     *     {@code Ice.Admin.Endpoints} is set, this method uses the {@code Ice.Admin} object adapter, after creating and
     *     activating this adapter.
     * @param adminId The identity of the Admin object.
     * @return A proxy to the main ("") facet of the Admin object.
     * @throws InitializationException Thrown when createAdmin is called more than once.
     * @see #getAdmin
     */
    public ObjectPrx createAdmin(ObjectAdapter adminAdapter, Identity adminId) {
        return _instance.createAdmin(adminAdapter, adminId);
    }

    /**
     * Gets a proxy to the main facet of the Admin object. getAdmin also creates the Admin object and creates and
     * activates the {@code Ice.Admin} object adapter to host this Admin object if {@code Ice.Admin.Endpoints} is set. The
     * identity of the Admin object created by getAdmin is {@code {value of Ice.Admin.InstanceName}/admin}, or
     * {@code {UUID}/admin} when {@code Ice.Admin.InstanceName} is not set. If {@code Ice.Admin.DelayCreation} is {@code 0} or not set,
     * getAdmin is called by the communicator initialization, after initialization of all plugins.
     *
     * @return A proxy to the main ("") facet of the Admin object, or null if no Admin
     *     object is configured.
     * @see #createAdmin
     */
    public ObjectPrx getAdmin() {
        return _instance.getAdmin();
    }

    /**
     * Adds a new facet to the Admin object.
     *
     * @param servant The servant that implements the new Admin facet.
     * @param facet The name of the new Admin facet.
     * @throws AlreadyRegisteredException Thrown when a facet with the same name is already registered.
     */
    public void addAdminFacet(Object servant, String facet) {
        _instance.addAdminFacet(servant, facet);
    }

    /**
     * Removes a facet from the Admin object.
     *
     * @param facet The name of the Admin facet.
     * @return The servant associated with this Admin facet.
     * @throws NotRegisteredException Thrown when no facet with the given name is registered.
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

    /** Certain initialization tasks need to be completed after the constructor. */
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

    /**
     * Get the {@code Instance} object associated with this communicator.
     *
     * @return the {@code Instance} object associated with this communicator
     * @hidden
     */
    public Instance getInstance() {
        return _instance;
    }
}
