// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import com.zeroc.Ice.Instrumentation.CommunicatorObserver;
import com.zeroc.Ice.SSL.SSLEngineFactory;

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Map;
import java.util.Stack;
import java.util.UUID;
import java.util.function.Function;
import java.util.stream.Stream;

/**
 * An object adapter is the main server-side Ice API. It has two main purposes:
 * - accept incoming connections from clients and dispatch requests received over these connections (see
 *   {@link #activate}); and
 * - maintain a dispatch pipeline and servants that handle the requests (see {@link #add}, {@link #addDefaultServant}, and {@link #use}).
 *
 * An object adapter can dispatch "bidirectional requests"--requests it receives over an outgoing connection
 * instead of a more common incoming connection. It can also dispatch collocated requests (with no connection at
 * all).
 *
 * @see Communicator#createObjectAdapter
 * @see ServantLocator
 */
public final class ObjectAdapter {
    private static final int StateUninitialized = 0; // Just constructed.
    private static final int StateHeld = 1;
    private static final int StateActivating = 2;
    private static final int StateActive = 3;
    private static final int StateDeactivating = 4;
    private static final int StateDeactivated = 5;
    private static final int StateDestroying = 6;
    private static final int StateDestroyed = 7;

    private int _state = StateUninitialized;
    private Instance _instance;
    private final Communicator _communicator;
    private ObjectAdapterFactory _objectAdapterFactory;
    private ThreadPool _threadPool;
    private final ServantManager _servantManager;
    private final String _name;
    private final String _id;
    private final String _replicaGroupId;
    private Reference _reference;
    private final List<IncomingConnectionFactory> _incomingConnectionFactories = new ArrayList<>();
    private RouterInfo _routerInfo;
    private EndpointI[] _publishedEndpoints = new EndpointI[0];
    private LocatorInfo _locatorInfo;
    private int _directCount; // The number of direct proxies dispatching on this object adapter.
    private final boolean _noConfig;
    private final int _messageSizeMax;
    private final SSLEngineFactory _sslEngineFactory;
    private Object _dispatchPipeline;
    private final Stack<Function<Object, Object>> _middlewareStack = new Stack<>();

    /**
     * Gets the name of this object adapter.
     *
     * @return This object adapter's name.
     */
    public String getName() {
        //
        // No mutex lock necessary, _name is immutable.
        //
        return _noConfig ? "" : _name;
    }

    /**
     * Gets the communicator that created this object adapter.
     *
     * @return This object adapter's communicator.
     * @see Communicator
     */
    public Communicator getCommunicator() {
        return _communicator;
    }

    /**
     * Starts receiving and dispatching requests received over incoming connections.
     *
     * @remark When this object adapter is an indirect object adapter configured with a locator proxy, this
     * method also registers the object adapter's published endpoints with this locator.
     * @see #deactivate
     * @see #getLocator
     * @see #getPublishedEndpoints
     */
    public void activate() {
        LocatorInfo locatorInfo = null;
        boolean printAdapterReady = false;

        synchronized (this) {
            checkForDeactivation();

            //
            // If we've previously been initialized we just need to activate the
            // incoming connection factories and we're done.
            //
            if (_state != StateUninitialized) {
                for (IncomingConnectionFactory factory : _incomingConnectionFactories) {
                    factory.activate();
                }
                _state = StateActive;
                notifyAll();
                return;
            }

            //
            // One off initializations of the adapter: update the
            // locator registry and print the "adapter ready"
            // message. We set set state to StateActivating to prevent
            // deactivation from other threads while these one off
            // initializations are done.
            //
            _state = StateActivating;

            locatorInfo = _locatorInfo;
            if (!_noConfig) {
                final Properties properties = _instance.initializationData().properties;
                printAdapterReady = properties.getIcePropertyAsInt("Ice.PrintAdapterReady") > 0;
            }
        }

        try {
            Identity dummy = new Identity();
            dummy.name = "dummy";
            updateLocatorRegistry(locatorInfo, createDirectProxy(dummy));
        } catch (LocalException ex) {
            //
            // If we couldn't update the locator registry, we let the
            // exception go through and don't activate the adapter to
            // allow to user code to retry activating the adapter
            // later.
            //
            synchronized (this) {
                _state = StateUninitialized;
                notifyAll();
            }
            throw ex;
        }

        if (printAdapterReady) {
            System.out.println(_name + " ready");
        }

        synchronized (this) {
            assert (_state == StateActivating);

            //
            // Signal threads waiting for the activation.
            //
            _state = StateActive;
            notifyAll();

            for (IncomingConnectionFactory factory : _incomingConnectionFactories) {
                factory.activate();
            }
        }
    }

    /**
     * Stops reading requests from incoming connections. Outstanding dispatches are not affected. The object
     * adapter can be reactivated with {@link #activate}.
     *
     * @remark This method is provided for backward compatibility with older versions of Ice. Don't use it in
     * new applications.
     * @see #activate
     * @see #deactivate
     * @see #waitForHold
     */
    public synchronized void hold() {
        checkForDeactivation();
        _state = StateHeld;
        for (IncomingConnectionFactory factory : _incomingConnectionFactories) {
            factory.hold();
        }
    }

    /**
     * Waits until the object adapter is in the holding state (see {@link #hold}) and the dispatch of requests received
     * over incoming connection has completed.
     *
     * @remark This method is provided for backward compatibility with older versions of Ice. Don't use it in
     * new applications.
     * @see #hold
     * @see #waitForDeactivate
     * @see Communicator#waitForShutdown
     */
    public void waitForHold() {
        List<IncomingConnectionFactory> incomingConnectionFactories;
        synchronized (this) {
            checkForDeactivation();
            incomingConnectionFactories = new ArrayList<>(_incomingConnectionFactories);
        }

        for (IncomingConnectionFactory factory : incomingConnectionFactories) {
            try {
                factory.waitUntilHolding();
            } catch (InterruptedException ex) {
                throw new OperationInterruptedException(ex);
            }
        }
    }

    /**
     * Deactivates this object adapter: stops accepting new connections from clients and closes gracefully all
     * incoming connections created by this object adapter once all outstanding dispatches have completed. If this
     * object adapter is indirect, this method also unregisters the object adapter from the locator
     * (see {@link #activate}).
     * This method does not cancel outstanding dispatches: it lets them execute until completion.
     * A deactivated object adapter cannot be reactivated again; it can only be destroyed.
     *
     * @see #waitForDeactivate
     * @see Communicator#shutdown
     */
    public void deactivate() {
        synchronized (this) {
            // Wait for activation or a previous deactivation to complete.
            // This is necessary to avoid out of order locator updates.
            while (_state == StateActivating) {
                try {
                    wait();
                } catch (InterruptedException ex) {
                    throw new OperationInterruptedException(ex);
                }
            }
            if (_state > StateDeactivating) {
                return;
            }
            _state = StateDeactivating;
        }

        //
        // NOTE: the router/locator infos and incoming connection
        // factory list are immutable at this point.
        //

        try {
            updateLocatorRegistry(_locatorInfo, null);
        } catch (LocalException ex) {
            //
            // We can't throw exceptions in deactivate so we ignore
            // failures to update the locator registry.
            //
        }

        for (IncomingConnectionFactory factory : _incomingConnectionFactories) {
            factory.destroy();
        }

        synchronized (this) {
            _state = StateDeactivated;
            notifyAll();
        }
    }

    /**
     * Waits until {@link #deactivate} is called on this object adapter and all connections accepted by this object adapter
     * are closed. A connection is closed only after all outstanding dispatches on this connection have completed.
     *
     * @see #deactivate
     * @see #waitForHold
     * @see Communicator#waitForShutdown
     */
    public void waitForDeactivate() {
        try {
            List<IncomingConnectionFactory> incomingConnectionFactories;
            synchronized (this) {
                //
                // Wait for deactivation of the adapter itself, and
                // for the return of all direct method calls using
                // this adapter.
                //
                while ((_state < StateDeactivated) || _directCount > 0) {
                    wait();
                }
                if (_state > StateDeactivated) {
                    return;
                }
                incomingConnectionFactories = new ArrayList<>(_incomingConnectionFactories);
            }

            //
            // Now we wait for until all incoming connection factories are
            // finished (the incoming connection factory list is immutable
            // at this point).
            //
            for (IncomingConnectionFactory f : incomingConnectionFactories) {
                f.waitUntilFinished();
            }
        } catch (InterruptedException e) {
            throw new OperationInterruptedException(e);
        }
    }

    /**
     * Checks whether or not {@link #deactivate} was called on this object adapter.
     *
     * @return {@code true} if {@link #deactivate} was called on this object adapter, {@code false} otherwise.
     * @see Communicator#shutdown
     */
    public synchronized boolean isDeactivated() {
        return _state >= StateDeactivated;
    }

    /**
     * Destroys this object adapter and cleans up all resources associated with it. Once this method has
     * returned, you can recreate another object adapter with the same name.
     *
     * @see #deactivate
     * @see #waitForDeactivate
     * @see Communicator#destroy
     */
    public void destroy() {
        //
        // Deactivate and wait for completion.
        //
        deactivate();
        waitForDeactivate();

        synchronized (this) {
            assert (_state >= StateDeactivated);

            //
            // Only a single thread is allowed to destroy the object
            // adapter. Other threads wait for the destruction to be
            // completed.
            //
            while (_state == StateDestroying) {
                try {
                    wait();
                } catch (InterruptedException ex) {
                    throw new OperationInterruptedException(ex);
                }
            }
            if (_state == StateDestroyed) {
                return;
            }

            _state = StateDestroying;
        }

        if (_routerInfo != null) {
            // Remove entry from the router manager.
            _instance.routerManager().erase(_routerInfo.getRouter());

            // Clear this object adapter with the router.
            _routerInfo.setAdapter(null);
        }

        _instance.outgoingConnectionFactory().removeAdapter(this);

        //
        // Now it's also time to clean up our servants and servant
        // locators.
        //
        _servantManager.destroy();

        //
        // Destroy the thread pool.
        //
        if (_threadPool != null) {
            _threadPool.destroy();
            try {
                _threadPool.joinWithAllThreads();
            } catch (InterruptedException e) {
                throw new OperationInterruptedException(e);
            }
        }

        _objectAdapterFactory.removeObjectAdapter(this);

        synchronized (this) {
            _incomingConnectionFactories.clear();

            //
            // Remove object references (some of them cyclic).
            //
            _instance = null;
            _threadPool = null;
            _routerInfo = null;
            _publishedEndpoints = new EndpointI[0];
            _locatorInfo = null;
            _reference = null;
            _objectAdapterFactory = null;

            //
            // Signal that destroying is complete.
            //
            _state = StateDestroyed;
            notifyAll();
        }
    }

    /**
     * Adds a middleware to the dispatch pipeline of this object adapter.
     *
     * @param middleware The middleware factory that creates the new middleware when this object adapter
     * creates its dispatch pipeline. A middleware factory is a function that takes an Object (the next element
     * in the dispatch pipeline) and returns a new Object (the middleware you want to install in the pipeline).
     * @return This object adapter.
     * @remark All middleware must be installed before the first dispatch.
     * @remark The middleware are executed in the order they are installed.
     * @throws IllegalStateException Thrown if the object adapter's dispatch pipeline has already
     *     been created. This creation typically occurs the first time the object adapter dispatches
     *     an incoming request.
     */
    public ObjectAdapter use(Function<Object, Object> middleware) {
        // This code is not thread-safe, and it's not supposed to be.
        if (_dispatchPipeline != null) {
            throw new IllegalStateException(
                "All middleware must be installed before the first dispatch.");
        }
        _middlewareStack.push(middleware);
        return this;
    }

    /**
     * Adds a servant to this object adapter's Active Servant Map (ASM). The ASM is a map {identity, facet} ->
     * servant.
     *
     * @param servant The servant to add.
     * @param identity The identity of the Ice object that is implemented by the servant.
     * @return A proxy for the identity created by this object adapter.
     * @throws AlreadyRegisteredException Thrown when a servant with the same identity is already registered.
     * @remark This method is equivalent to calling {@link #addFacet} with an empty facet.
     * @see Identity
     * @see #addFacet
     * @see #addWithUUID
     * @see #remove
     * @see #find
     */
    public ObjectPrx add(Object servant, Identity identity) {
        return addFacet(servant, identity, "");
    }

    /**
     * Adds a servant to this object adapter's Active Servant Map (ASM), while specifying a facet. The ASM is a map
     * {identity, facet} -> servant.
     *
     * @param servant The servant to add.
     * @param identity The identity of the Ice object that is implemented by the servant.
     * @param facet The facet of the Ice object that is implemented by the servant.
     * @return A proxy for the identity and facet created by this object adapter.
     * @throws AlreadyRegisteredException Thrown when a servant with the same identity and facet is already
     * registered.
     * @see Identity
     * @see #add
     * @see #addFacetWithUUID
     * @see #removeFacet
     * @see #findFacet
     */
    public synchronized ObjectPrx addFacet(Object servant, Identity identity, String facet) {
        checkForDestruction();
        checkIdentity(identity);
        if (servant == null) {
            throw new IllegalArgumentException("cannot add null servant to Object Adapter");
        }

        //
        // Create a copy of the Identity argument, in case the caller
        // reuses it.
        //
        Identity id = new Identity(identity.name, identity.category);
        _servantManager.addServant(servant, id, facet);

        return newProxy(id, facet);
    }

    /**
     * Adds a servant to this object adapter's Active Servant Map (ASM), using an automatically generated UUID as
     * its identity.
     *
     * @param servant The servant to add.
     * @return A proxy with the generated UUID identity created by this object adapter.
     * @see Identity
     * @see #add
     * @see #addFacetWithUUID
     * @see #remove
     * @see #find
     */
    public ObjectPrx addWithUUID(Object servant) {
        return addFacetWithUUID(servant, "");
    }

    /**
     * Adds a servant to this object adapter's Active Servant Map (ASM), using an automatically generated UUID as
     * its identity. Also specifies a facet.
     *
     * @param servant The servant to add.
     * @param facet The facet of the Ice object that is implemented by the servant.
     * @return A proxy with the generated UUID identity and the specified facet.
     * @see Identity
     * @see #addFacet
     * @see #addWithUUID
     * @see #removeFacet
     * @see #findFacet
     */
    public ObjectPrx addFacetWithUUID(Object servant, String facet) {
        Identity ident = new Identity();
        ident.category = "";
        ident.name = UUID.randomUUID().toString();

        return addFacet(servant, ident, facet);
    }

    /**
     * Adds a default servant to handle requests for a specific category. When an object adapter dispatches an
     * incoming request, it tries to find a servant for the identity and facet carried by the request in the
     * following order:
     *  - The object adapter tries to find a servant for the identity and facet in the Active Servant Map.
     *  - If this fails, the object adapter tries to find a default servant for the category component of the
     *    identity.
     *  - If this fails, the object adapter tries to find a default servant for the empty category, regardless of
     *    the category contained in the identity.
     *  - If this fails, the object adapter tries to find a servant locator for the category component of the
     *    identity. If there is no such servant locator, the object adapter tries to find a servant locator for the
     *    empty category.
     *    - If a servant locator is found, the object adapter tries to find a servant using this servant locator.
     *  - If all the previous steps fail, the object adapter gives up and the caller receives an
     *    {@link ObjectNotExistException} or a {@link FacetNotExistException}.
     *
     * @param servant The default servant to add.
     * @param category The category for which the default servant is registered. The empty category means it
     * handles all categories.
     * @throws AlreadyRegisteredException Thrown when a default servant with the same category is already registered.
     * @see #removeDefaultServant
     * @see #findDefaultServant
     */
    public synchronized void addDefaultServant(Object servant, String category) {
        checkForDestruction();
        if (servant == null) {
            throw new IllegalArgumentException("cannot add null servant to Object Adapter");
        }

        _servantManager.addDefaultServant(servant, category);
    }

    /**
     * Removes a servant from the object adapter's Active Servant Map.
     *
     * @param identity The identity of the Ice object that is implemented by the servant.
     * @return The removed servant.
     * @throws NotRegisteredException Thrown when no servant with the given identity is registered.
     * @see Identity
     * @see #add
     * @see #addWithUUID
     */
    public Object remove(Identity identity) {
        return removeFacet(identity, "");
    }

    /**
     * Removes a servant from the object adapter's Active Servant Map, while specifying a facet.
     *
     * @param identity The identity of the Ice object that is implemented by the servant.
     * @param facet The facet. An empty facet means the default facet.
     * @return The removed servant.
     * @throws NotRegisteredException Thrown when no servant with the given identity and facet is registered.
     * @see Identity
     * @see #addFacet
     * @see #addFacetWithUUID
     */
    public synchronized Object removeFacet(Identity identity, String facet) {
        checkForDestruction();
        checkIdentity(identity);

        return _servantManager.removeServant(identity, facet);
    }

    /**
     * Removes all facets with the given identity from the Active Servant Map. The method completely removes the
     * Ice object, including its default facet.
     *
     * @param identity The identity of the Ice object to be removed.
     * @return A collection containing all the facet names and servants of the removed Ice object.
     * @throws NotRegisteredException Thrown when no servant with the given identity is registered.
     * @see #remove
     * @see #removeFacet
     */
    public synchronized Map<String, Object> removeAllFacets(Identity identity) {
        checkForDestruction();
        checkIdentity(identity);

        return _servantManager.removeAllFacets(identity);
    }

    /**
     * Removes the default servant for a specific category.
     *
     * @param category The category of the default servant to remove.
     * @return The default servant.
     * @throws NotRegisteredException Thrown when no default servant is registered for the given category.
     * @see #addDefaultServant
     * @see #findDefaultServant
     */
    public synchronized Object removeDefaultServant(String category) {
        checkForDestruction();

        return _servantManager.removeDefaultServant(category);
    }

    /**
     * Looks up a servant.
     *
     * @param identity The identity of an Ice object.
     * @return The servant that implements the Ice object with the given identity, or null if no such servant
     * has been found.
     * @remark This method only tries to find the servant in the ASM and among the default servants. It does not
     * attempt to locate a servant using servant locators.
     * @see Identity
     * @see #findFacet
     * @see #findByProxy
     */
    public Object find(Identity identity) {
        return findFacet(identity, "");
    }

    /**
     * Looks up a servant with an identity and facet.
     *
     * @param identity The identity of an Ice object.
     * @param facet The facet of an Ice object. An empty facet means the default facet.
     * @return The servant that implements the Ice object with the given identity and facet, or null
     *     if no such servant has been found.
     * @remark This method only tries to find the servant in the ASM and among the default servants. It does not
     * attempt to locate a servant using servant locators.
     * @see Identity
     * @see #find
     * @see #findByProxy
     */
    public synchronized Object findFacet(Identity identity, String facet) {
        checkForDestruction();
        checkIdentity(identity);

        return _servantManager.findServant(identity, facet);
    }

    /**
     * Finds all facets for a given identity in the Active Servant Map.
     *
     * @param identity The identity.
     * @return A collection containing all the facet names and servants that have been found. Can be empty.
     * @see #find
     * @see #findFacet
     */
    public synchronized Map<String, Object> findAllFacets(Identity identity) {
        checkForDestruction();
        checkIdentity(identity);

        return _servantManager.findAllFacets(identity);
    }

    /**
     * Looks up a servant with an identity and a facet. It's equivalent to calling {@link #findFacet}.
     *
     * @param proxy The proxy that provides the identity and facet to search.
     * @return The servant that matches the identity and facet carried by the proxy, or null if no such servant
     * has been found.
     * @see #find
     * @see #findFacet
     */
    public synchronized Object findByProxy(ObjectPrx proxy) {
        checkForDestruction();

        Reference ref = proxy._getReference();
        return findFacet(ref.getIdentity(), ref.getFacet());
    }

    /**
     * Adds a ServantLocator to this object adapter for a specific category.
     *
     * @param locator The servant locator to add.
     * @param category The category. The empty category means the locator handles all categories.
     * @throws AlreadyRegisteredException Thrown when a servant locator with the same category is already
     * registered.
     * @see #addDefaultServant
     * @see Identity
     * @see #removeServantLocator
     * @see #findServantLocator
     * @see ServantLocator
     */
    public synchronized void addServantLocator(ServantLocator locator, String category) {
        checkForDestruction();

        _servantManager.addServantLocator(locator, category);
    }

    /**
     * Removes a ServantLocator from this object adapter.
     *
     * @param category The category.
     * @return The servant locator.
     * @throws NotRegisteredException Thrown when no ServantLocator with the given category is registered.
     * @see Identity
     * @see #addServantLocator
     * @see #findServantLocator
     * @see ServantLocator
     */
    public synchronized ServantLocator removeServantLocator(String category) {
        checkForDestruction();

        return _servantManager.removeServantLocator(category);
    }

    /**
     * Finds a ServantLocator registered with this object adapter.
     *
     * @param category The category.
     * @return The servant locator, or null if not found.
     * @see Identity
     * @see #addServantLocator
     * @see #removeServantLocator
     * @see ServantLocator
     */
    public synchronized ServantLocator findServantLocator(String category) {
        checkForDestruction();

        return _servantManager.findServantLocator(category);
    }

    /**
     * Finds the default servant for a specific category.
     *
     * @param category The category.
     * @return The default servant, or null if not found.
     * @see #addDefaultServant
     * @see #removeDefaultServant
     */
    public synchronized Object findDefaultServant(String category) {
        checkForDestruction();

        return _servantManager.findDefaultServant(category);
    }

    /**
     * Gets the dispatch pipeline of this object adapter.
     *
     * @return The dispatch pipeline. The returned value is never null.
     */
    public synchronized Object dispatchPipeline() {
        if (_dispatchPipeline == null) {
            _dispatchPipeline = _servantManager;
            while (!_middlewareStack.isEmpty()) {
                _dispatchPipeline = _middlewareStack.pop().apply(_dispatchPipeline);
            }
        }
        return _dispatchPipeline;
    }

    /**
     * Creates a proxy from an Ice identity. If this object adapter is configured with an adapter ID, the proxy
     * is an indirect proxy that refers to this adapter ID. If a replica group ID is also defined, the proxy is an
     * indirect proxy that refers to this replica group ID. Otherwise, the proxy is a direct proxy containing this
     * object adapter's published endpoints.
     *
     * @param identity An Ice identity.
     * @return A proxy with the given identity.
     * @see Identity
     */
    public synchronized ObjectPrx createProxy(Identity identity) {
        checkForDestruction();
        checkIdentity(identity);

        return newProxy(identity, "");
    }

    /**
     * Creates a direct proxy from an Ice identity.
     *
     * @param identity An Ice identity.
     * @return A proxy with the given identity and this published endpoints of this object adapter.
     * @see Identity
     */
    public synchronized ObjectPrx createDirectProxy(Identity identity) {
        checkForDestruction();
        checkIdentity(identity);

        return newDirectProxy(identity, "");
    }

    /**
     * Creates an indirect proxy for an Ice identity.
     *
     * @param identity An Ice identity.
     * @return An indirect proxy with the given identity. If this object adapter is not configured with an adapter
     * ID or a replica group ID, the new proxy is a well-known proxy (i.e., an identity-only proxy).
     * @see Identity
     */
    public synchronized ObjectPrx createIndirectProxy(Identity identity) {
        checkForDestruction();
        checkIdentity(identity);

        return newIndirectProxy(identity, "", _id);
    }

    /**
     * Sets an Ice locator on this object adapter.
     *
     * @param locator The locator used by this object adapter.
     * @see #createDirectProxy
     * @see Locator
     * @see LocatorRegistry
     */
    public synchronized void setLocator(LocatorPrx locator) {
        checkForDeactivation();

        _locatorInfo = _instance.locatorManager().get(locator);
    }

    /**
     * Gets the Ice locator used by this object adapter.
     *
     * @return The locator used by this object adapter, or null if no locator is used by this object
     *     adapter.
     * @see Locator
     * @see #setLocator
     */
    public synchronized LocatorPrx getLocator() {
        if (_locatorInfo == null) {
            return null;
        } else {
            return _locatorInfo.getLocator();
        }
    }

    /**
     * Gets the set of endpoints configured on this object adapter.
     *
     * @return The set of endpoints.
     * @remark This method remains usable after the object adapter has been deactivated.
     * @see Endpoint
     */
    public synchronized Endpoint[] getEndpoints() {
        List<Endpoint> endpoints = new ArrayList<>();
        for (IncomingConnectionFactory factory : _incomingConnectionFactories) {
            endpoints.add(factory.endpoint());
        }
        return endpoints.toArray(new Endpoint[0]);
    }

    /**
     * Gets the set of endpoints that proxies created by this object adapter will contain.
     *
     * @return The set of published endpoints.
     * @remark This method remains usable after the object adapter has been deactivated.
     * @see Endpoint
     */
    public synchronized Endpoint[] getPublishedEndpoints() {
        return Arrays.copyOf(
            _publishedEndpoints, _publishedEndpoints.length, Endpoint[].class);
    }

    /**
     * Sets the endpoints that proxies created by this object adapter will contain.
     *
     * @param newEndpoints The new set of endpoints that the object adapter will embed in proxies.
     * @see Endpoint
     */
    public void setPublishedEndpoints(Endpoint[] newEndpoints) {
        LocatorInfo locatorInfo = null;
        EndpointI[] oldPublishedEndpoints;

        synchronized (this) {
            checkForDeactivation();
            if (_routerInfo != null) {
                throw new IllegalArgumentException(
                    "can't set published endpoints on object adapter associated with a router");
            }

            oldPublishedEndpoints = _publishedEndpoints;
            _publishedEndpoints =
                Arrays.copyOf(newEndpoints, newEndpoints.length, EndpointI[].class);
            locatorInfo = _locatorInfo;
        }

        try {
            Identity dummy = new Identity();
            dummy.name = "dummy";
            updateLocatorRegistry(locatorInfo, createDirectProxy(dummy));
        } catch (LocalException ex) {
            synchronized (this) {
                //
                // Restore the old published endpoints.
                //
                _publishedEndpoints = oldPublishedEndpoints;
                throw ex;
            }
        }
    }

    /**
     * Checks whether the specified reference refers to a local object.
     *
     * @param ref the reference to check
     * @return true if the reference refers to a local object, false otherwise
     */
    public boolean isLocal(Reference ref) {
        //
        // NOTE: it's important that isLocal() doesn't perform any blocking operations as
        // it can be called for AMI invocations if the proxy has no delegate set yet.
        //

        if (ref.isWellKnown()) {
            //
            // Check the active servant map to see if the well-known
            // proxy is for a local object.
            //
            return _servantManager.hasServant(ref.getIdentity());
        } else if (ref.isIndirect()) {
            //
            // Proxy is local if the reference adapter id matches this
            // adapter id or replica group id.
            //
            return ref.getAdapterId().equals(_id) || ref.getAdapterId().equals(_replicaGroupId);
        } else {
            // Proxies which have at least one endpoint in common with the published endpoints
            // are considered local.
            synchronized (this) {
                checkForDestruction();

                EndpointI[] endpoints = ref.getEndpoints();
                return Arrays.stream(_publishedEndpoints)
                    .anyMatch(p -> Arrays.stream(endpoints).anyMatch(p::equivalent));
            }
        }
    }

    /**
     * Flushes any pending batch requests for this object adapter.
     *
     * @param compressBatch indicates whether to compress the batch requests
     * @param outAsync the callback for the asynchronous flush operation
     */
    public void flushAsyncBatchRequests(
            CompressBatch compressBatch, CommunicatorFlushBatch outAsync) {
        List<IncomingConnectionFactory> f;
        synchronized (this) {
            f = new ArrayList<>(_incomingConnectionFactories);
        }
        for (IncomingConnectionFactory p : f) {
            p.flushAsyncBatchRequests(compressBatch, outAsync);
        }
    }

    /**
     * Updates the connection observers for all incoming connection factories.
     */
    public void updateConnectionObservers() {
        List<IncomingConnectionFactory> f;
        synchronized (this) {
            f = new ArrayList<>(_incomingConnectionFactories);
        }
        for (IncomingConnectionFactory p : f) {
            p.updateConnectionObservers();
        }
    }

    /**
     * Updates the thread observers for the thread pool.
     */
    public void updateThreadObservers() {
        ThreadPool threadPool = null;
        synchronized (this) {
            threadPool = _threadPool;
        }
        if (threadPool != null) {
            threadPool.updateObservers();
        }
    }

    /**
     * Increments the count of direct method calls using this object adapter.
     */
    public synchronized void incDirectCount() {
        checkForDestruction();

        assert (_directCount >= 0);
        ++_directCount;
    }

    /**
     * Decrements the count of direct method calls using this object adapter.
     */
    public synchronized void decDirectCount() {
        // Not check for deactivation here!

        assert (_instance != null); // Must not be called after destroy().

        assert (_directCount > 0);
        if (--_directCount == 0) {
            notifyAll();
        }
    }

    /**
     * Gets the thread pool used by this object adapter.
     *
     * @return the thread pool used by this object adapter
     */
    public ThreadPool getThreadPool() {
        // No mutex lock necessary, _threadPool and _instance are
        // immutable after creation until they are removed in
        // destroy().

        // Not check for deactivation here!

        assert (_instance != null); // Must not be called after destroy().

        if (_threadPool != null) {
            return _threadPool;
        } else {
            return _instance.serverThreadPool();
        }
    }

    /**
     * Sets this object adapter on the specified connection.
     *
     * @param connection the connection to set the adapter on
     */
    public synchronized void setAdapterOnConnection(ConnectionI connection) {
        checkForDestruction();
        connection.setAdapterFromAdapter(this);
    }

    /**
     * Gets the maximum message size for this object adapter.
     *
     * @return the maximum message size in bytes
     */
    public int messageSizeMax() {
        // No mutex lock, immutable.
        return _messageSizeMax;
    }

    //
    // Only for use by com.zeroc.Ice.ObjectAdapterFactory
    //
    ObjectAdapter(
            Instance instance,
            Communicator communicator,
            ObjectAdapterFactory objectAdapterFactory,
            String name,
            RouterPrx router,
            boolean noConfig,
            SSLEngineFactory sslEngineFactory) {
        _instance = instance;
        _communicator = communicator;
        _objectAdapterFactory = objectAdapterFactory;
        _servantManager = new ServantManager(instance, name);
        _name = name;
        _directCount = 0;
        _noConfig = noConfig;
        _sslEngineFactory = sslEngineFactory;

        // Install default middleware depending on the communicator's configuration.
        if (_instance.initializationData().logger != null) {
            Logger logger = _instance.initializationData().logger;
            int warningLevel =
                _instance
                    .initializationData()
                    .properties
                    .getIcePropertyAsInt("Ice.Warn.Dispatch");
            if (_instance.traceLevels().dispatch > 0 || warningLevel > 0) {
                use(
                    next ->
                        new LoggerMiddleware(
                            next,
                            logger,
                            _instance.traceLevels().dispatch,
                            _instance.traceLevels().dispatchCat,
                            warningLevel,
                            _instance.toStringMode()));
            }
        }
        if (_instance.initializationData().observer != null) {
            CommunicatorObserver observer = _instance.initializationData().observer;
            use(next -> new ObserverMiddleware(next, observer));
        }

        if (_noConfig) {
            _id = "";
            _replicaGroupId = "";
            _reference = _instance.referenceFactory().create("dummy -t", "");
            _messageSizeMax = _instance.messageSizeMax();
            return;
        }

        final Properties properties = _instance.initializationData().properties;

        try {
            Properties.validatePropertiesWithPrefix(
                _name, properties, PropertyNames.ObjectAdapterProps);
        } catch (PropertyException ex) {
            // Prevent finalizer from complaining about the adapter not being destroyed.
            _state = StateDestroyed;
            throw ex;
        }

        //
        // Make sure named adapter has some configuration.
        //
        if (router == null && properties.getPropertiesForPrefix(_name).isEmpty()) {
            // Prevent finalizer from complaining about the adapter not being destroyed.
            _state = StateDestroyed;

            throw new InitializationException(
                "Object adapter '" + _name + "' requires configuration.");
        }

        _id = properties.getProperty(_name + ".AdapterId");
        _replicaGroupId = properties.getProperty(_name + ".ReplicaGroupId");

        //
        // Setup a reference to be used to get the default proxy options
        // when creating new proxies. By default, create twoway proxies.
        //
        String proxyOptions = properties.getPropertyWithDefault(_name + ".ProxyOptions", "-t");
        try {
            _reference = _instance.referenceFactory().create("dummy " + proxyOptions, "");
        } catch (ParseException ex) {
            // Prevent finalizer from complaining about the adapter not being destroyed.
            _state = StateDestroyed;
            throw new InitializationException(
                "invalid proxy options '"
                    + proxyOptions
                    + "' for object adapter '"
                    + _name
                    + "'.",
                ex);
        }

        // The maximum size of an Ice protocol message in bytes. This is limited to 0x7fffffff, which corresponds to
        // the maximum value of a 32-bit signed integer (int).
        final int messageSizeMaxUpperLimit = Integer.MAX_VALUE;
        final int defaultMessageSizeMax = instance.messageSizeMax() / 1024;
        int messageSizeMax = properties.getPropertyAsIntWithDefault(
            _name + ".MessageSizeMax", defaultMessageSizeMax);
        if (messageSizeMax > messageSizeMaxUpperLimit / 1024) {
            throw new InitializationException(
                _name + ".MessageSizeMax '" + messageSizeMax + "' is too large, it must be less than or equal to '"
                    + (messageSizeMaxUpperLimit / 1024) + "' KiB");
        } else if (messageSizeMax < 1) {
            _messageSizeMax = messageSizeMaxUpperLimit;
        } else {
            // The property is specified in kibibytes (KiB); _messageSizeMax is stored in bytes.
            _messageSizeMax = messageSizeMax * 1024;
        }

        try {
            int threadPoolSize = properties.getPropertyAsInt(_name + ".ThreadPool.Size");
            int threadPoolSizeMax = properties.getPropertyAsInt(_name + ".ThreadPool.SizeMax");

            //
            // Create the per-adapter thread pool, if necessary.
            //
            if (threadPoolSize > 0 || threadPoolSizeMax > 0) {
                _threadPool = new ThreadPool(_instance, _name + ".ThreadPool", 0);
            }

            if (router == null) {
                router = RouterPrx.uncheckedCast(communicator.propertyToProxy(name + ".Router"));
            }
            if (router != null) {
                _routerInfo = _instance.routerManager().get(router);
                assert (_routerInfo != null);

                if (!properties.getProperty(_name + ".Endpoints").isEmpty()) {
                    throw new InitializationException(
                        "An object adapter with a router cannot accept incoming connections.");
                }

                //
                // Make sure this router is not already registered with another adapter.
                //
                if (_routerInfo.getAdapter() != null) {
                    throw new AlreadyRegisteredException(
                        "object adapter with router",
                        _communicator.identityToString(router.ice_getIdentity()));
                }

                //
                // Associate this object adapter with the router. This way,
                // new outgoing connections to the router's client proxy will
                // use this object adapter for callbacks.
                //
                _routerInfo.setAdapter(this);

                //
                // Also modify all existing outgoing connections to the
                // router's client proxy to use this object adapter for
                // callbacks.
                //
                _instance.outgoingConnectionFactory().setRouterInfo(_routerInfo);
            } else {
                //
                // Parse the endpoints, but don't store them in the adapter. The connection
                // factory might change it, for example, to fill in the real port number.
                //
                List<EndpointI> endpoints =
                    parseEndpoints(properties.getProperty(_name + ".Endpoints"), true);

                for (EndpointI endpoint : endpoints) {
                    for (EndpointI expanded : endpoint.expandHost()) {
                        _incomingConnectionFactories.add(
                            new IncomingConnectionFactory(instance, expanded, this));
                    }
                }
                if (endpoints.isEmpty()) {
                    TraceLevels tl = _instance.traceLevels();
                    if (tl.network >= 2) {
                        _instance
                            .initializationData()
                            .logger
                            .trace(
                                tl.networkCat,
                                "created adapter '" + name + "' without endpoints");
                    }
                }
            }

            //
            // Compute the published endpoints.
            //
            _publishedEndpoints = computePublishedEndpoints();

            if (properties.getProperty(_name + ".Locator").length() > 0) {
                setLocator(
                    LocatorPrx.uncheckedCast(communicator.propertyToProxy(_name + ".Locator")));
            } else {
                setLocator(_instance.referenceFactory().getDefaultLocator());
            }
        } catch (LocalException ex) {
            destroy();
            throw ex;
        }
    }

    /**
     * Gets the SSL engine factory used by this object adapter.
     *
     * @return the SSL engine factory
     */
    public SSLEngineFactory getSSLEngineFactory() {
        return _sslEngineFactory;
    }

    @SuppressWarnings({"nofinalizer", "deprecation"})
    @Override
    protected synchronized void finalize() throws Throwable {
        try {
            if (_state < StateDeactivated) {
                _instance
                    .initializationData()
                    .logger
                    .warning("object adapter `" + getName() + "' has not been deactivated");
            } else if (_state != StateDestroyed) {
                _instance
                    .initializationData()
                    .logger
                    .warning("object adapter `" + getName() + "' has not been destroyed");
            } else {
                Assert.FinalizerAssert(_threadPool == null);
                // Not cleared, it needs to be immutable.
                // Assert.FinalizerAssert(_servantManager == null);
                // Assert.FinalizerAssert(_incomingConnectionFactories.isEmpty());
                Assert.FinalizerAssert(_directCount == 0);
            }
        } catch (Exception ex) {} finally {
            super.finalize();
        }
    }

    private ObjectPrx newProxy(Identity ident, String facet) {
        if (_id.isEmpty()) {
            return newDirectProxy(ident, facet);
        } else if (_replicaGroupId.isEmpty()) {
            return newIndirectProxy(ident, facet, _id);
        } else {
            return newIndirectProxy(ident, facet, _replicaGroupId);
        }
    }

    private ObjectPrx newDirectProxy(Identity ident, String facet) {
        // Create a reference and return a proxy for this reference.
        var ref =
            _instance.referenceFactory().create(ident, facet, _reference, _publishedEndpoints);
        return new _ObjectPrxI(ref);
    }

    private ObjectPrx newIndirectProxy(Identity ident, String facet, String id) {
        // Create a reference with the adapter id and return a proxy for the reference.
        var ref = _instance.referenceFactory().create(ident, facet, _reference, id);
        return new _ObjectPrxI(ref);
    }

    private void checkForDeactivation() {
        checkForDestruction();
        if (_state >= StateDeactivating) {
            throw new ObjectAdapterDeactivatedException(getName());
        }
    }

    private void checkForDestruction() {
        if (_state >= StateDestroying) {
            throw new ObjectAdapterDestroyedException(getName());
        }
    }

    private static void checkIdentity(Identity ident) {
        if (ident.name == null || ident.name.isEmpty()) {
            throw new IllegalArgumentException(
                "The name of an Ice object identity cannot be empty.");
        }

        if (ident.category == null) {
            ident.category = "";
        }
    }

    private List<EndpointI> parseEndpoints(String endpts, boolean oaEndpoints) {
        int beg;
        int end = 0;

        final String delim = " \t\n\r";

        List<EndpointI> endpoints = new ArrayList<>();
        while (end < endpts.length()) {
            beg = StringUtil.findFirstNotOf(endpts, delim, end);
            if (beg == -1) {
                if (!endpoints.isEmpty()) {
                    throw new ParseException("invalid empty object adapter endpoint");
                }
                break;
            }

            end = beg;
            while (true) {
                end = endpts.indexOf(':', end);
                if (end == -1) {
                    end = endpts.length();
                    break;
                } else {
                    boolean quoted = false;
                    int quote = beg;
                    while (true) {
                        quote = endpts.indexOf('\"', quote);
                        if (quote == -1 || end < quote) {
                            break;
                        } else {
                            quote = endpts.indexOf('\"', ++quote);
                            if (quote == -1) {
                                break;
                            } else if (end < quote) {
                                quoted = true;
                                break;
                            }
                            ++quote;
                        }
                    }
                    if (!quoted) {
                        break;
                    }
                    ++end;
                }
            }

            if (end == beg) {
                throw new ParseException("invalid empty object adapter endpoint");
            }

            String s = endpts.substring(beg, end);
            EndpointI endp = _instance.endpointFactoryManager().create(s, oaEndpoints);
            if (endp == null) {
                throw new ParseException("invalid object adapter endpoint '" + s + "'");
            }
            endpoints.add(endp);

            ++end;
        }

        return endpoints;
    }

    private EndpointI[] computePublishedEndpoints() {
        EndpointI[] endpointsArray;
        if (_routerInfo != null) {
            // Get the router's server proxy endpoints and use them as the published endpoints.
            endpointsArray = _routerInfo.getServerEndpoints();
        } else {
            // Parse published endpoints. If set, these are used instead of the connection factory
            // endpoints.
            var endpointsList =
                parseEndpoints(
                    _instance
                        .initializationData()
                        .properties
                        .getProperty(_name + ".PublishedEndpoints"),
                    false);

            if (endpointsList.isEmpty()) {
                // If the PublishedEndpoints property isn't set, we compute the published endpoints
                // from the factory endpoints.
                endpointsList =
                    _incomingConnectionFactories.stream()
                        .map(IncomingConnectionFactory::endpoint)
                        .toList();

                // Remove all loopback/multicast endpoints.
                var endpointsNoLoopback =
                    endpointsList.stream().filter(e -> !e.isLoopbackOrMulticast()).toList();

                // Retrieve published host.
                String publishedHost =
                    _instance
                        .initializationData()
                        .properties
                        .getProperty(_name + ".PublishedHost");

                Stream<EndpointI> endpoints;

                if (endpointsNoLoopback.isEmpty()) {
                    endpoints = endpointsList.stream();
                } else {
                    endpoints = endpointsNoLoopback.stream();

                    // For non-loopback/multicast endpoints, we use the fully qualified name of the
                    // local host as default for publishedHost.
                    if (publishedHost.isEmpty()) {
                        try {
                            publishedHost = InetAddress.getLocalHost().getHostName();
                        } catch (UnknownHostException e) {
                            throw new InitializationException(
                                "failed to get the local host name", e);
                        }
                    }
                }

                // Replace the host in all endpoints by publishedHost (when applicable) and clear
                // local options.
                final String publishedHostCapture = publishedHost;
                endpoints =
                    endpoints.map(e -> e.toPublishedEndpoint(publishedHostCapture)).distinct();

                endpointsArray = endpoints.toArray(EndpointI[]::new);
            } else {
                endpointsArray = endpointsList.toArray(EndpointI[]::new);
            }
        }

        if (_instance.traceLevels().network >= 1 && endpointsArray.length > 0) {
            StringBuffer s = new StringBuffer("published endpoints for object adapter '");
            s.append(_name);
            s.append("':\n");
            boolean first = true;
            for (EndpointI endpoint : endpointsArray) {
                if (!first) {
                    s.append(':');
                }
                s.append(endpoint.toString());
                first = false;
            }
            _instance
                .initializationData()
                .logger
                .trace(_instance.traceLevels().networkCat, s.toString());
        }
        return endpointsArray;
    }

    private void updateLocatorRegistry(LocatorInfo locatorInfo, ObjectPrx proxy) {
        if (_id.isEmpty() || locatorInfo == null) {
            return; // Nothing to update.
        }

        //
        // Call on the locator registry outside the synchronization to
        // blocking other threads that need to lock this OA.
        //
        LocatorRegistryPrx locatorRegistry = locatorInfo.getLocatorRegistry();
        if (locatorRegistry == null) {
            return;
        }

        try {
            if (_replicaGroupId.isEmpty()) {
                locatorRegistry.setAdapterDirectProxy(_id, proxy);
            } else {
                locatorRegistry.setReplicatedAdapterDirectProxy(_id, _replicaGroupId, proxy);
            }
        } catch (AdapterNotFoundException ex) {
            if (_instance.traceLevels().location >= 1) {
                StringBuilder s = new StringBuilder(128);
                s.append("couldn't update object adapter `");
                s.append(_id);
                s.append("' endpoints with the locator registry:\n");
                s.append("the object adapter is not known to the locator registry");
                _instance
                    .initializationData()
                    .logger
                    .trace(_instance.traceLevels().locationCat, s.toString());
            }

            throw new NotRegisteredException("object adapter", _id);
        } catch (InvalidReplicaGroupIdException ex) {
            if (_instance.traceLevels().location >= 1) {
                StringBuilder s = new StringBuilder(128);
                s.append("couldn't update object adapter `");
                s.append(_id);
                s.append("' endpoints with the locator registry:\n");
                s.append("the replica group `");
                s.append(_replicaGroupId);
                s.append("' is not known to the locator registry");
                _instance
                    .initializationData()
                    .logger
                    .trace(_instance.traceLevels().locationCat, s.toString());
            }

            throw new NotRegisteredException("replica group", _replicaGroupId);
        } catch (AdapterAlreadyActiveException ex) {
            if (_instance.traceLevels().location >= 1) {
                StringBuilder s = new StringBuilder(128);
                s.append("couldn't update object adapter `");
                s.append(_id);
                s.append("' endpoints with the locator registry:\n");
                s.append("the object adapter endpoints are already set");
                _instance
                    .initializationData()
                    .logger
                    .trace(_instance.traceLevels().locationCat, s.toString());
            }

            throw new ObjectAdapterIdInUseException(_id);
        } catch (ObjectAdapterDestroyedException e) {
            // Expected if collocated call and OA is destroyed, ignore.
        } catch (CommunicatorDestroyedException e) {
            // Ignore
        } catch (LocalException e) {
            if (_instance.traceLevels().location >= 1) {
                StringBuilder s = new StringBuilder(128);
                s.append("couldn't update object adapter `");
                s.append(_id);
                s.append("' endpoints with the locator registry:\n");
                s.append(e.toString());
                _instance
                    .initializationData()
                    .logger
                    .trace(_instance.traceLevels().locationCat, s.toString());
            }
            throw e; // TODO: Shall we raise a special exception instead of a non obvious local
            // exception?
        }

        if (_instance.traceLevels().location >= 1) {
            StringBuilder s = new StringBuilder(128);
            s.append("updated object adapter `");
            s.append(_id);
            s.append("' endpoints with the locator registry\n");
            s.append("endpoints = ");
            if (proxy != null) {
                Endpoint[] endpoints = proxy.ice_getEndpoints();
                for (int i = 0; i < endpoints.length; i++) {
                    s.append(endpoints[i].toString());
                    if (i + 1 < endpoints.length) {
                        s.append(':');
                    }
                }
            }
            _instance
                .initializationData()
                .logger
                .trace(_instance.traceLevels().locationCat, s.toString());
        }
    }
}
