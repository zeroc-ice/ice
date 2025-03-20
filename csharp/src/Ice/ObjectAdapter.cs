// Copyright (c) ZeroC, Inc.

#nullable enable

using Ice.Instrumentation;
using Ice.Internal;
using System.Diagnostics;
using System.Net;
using System.Net.Security;
using System.Text;

namespace Ice;

public sealed class ObjectAdapter
{
    private const int StateUninitialized = 0; // Just constructed.
    private const int StateHeld = 1;
    private const int StateActivating = 2;
    private const int StateActive = 3;
    private const int StateDeactivating = 4;
    private const int StateDeactivated = 5;
    private const int StateDestroying = 6;
    private const int StateDestroyed = 7;

    private int _state = StateUninitialized;
    private readonly Instance _instance;
    private readonly Communicator _communicator;
    private ObjectAdapterFactory? _objectAdapterFactory;
    private Ice.Internal.ThreadPool? _threadPool;
    private readonly ServantManager _servantManager;
    private readonly string _name;
    private readonly string _id;
    private readonly string _replicaGroupId;
    private Reference? _reference;
    private readonly List<IncomingConnectionFactory> _incomingConnectionFactories;
    private RouterInfo? _routerInfo;
    private EndpointI[] _publishedEndpoints;
    private LocatorInfo? _locatorInfo;
    private int _directCount;  // The number of colloc proxies dispatching on this object adapter.
    private readonly bool _noConfig;
    private readonly int _messageSizeMax;
    private readonly SslServerAuthenticationOptions? _serverAuthenticationOptions;

    private readonly Lazy<Object> _dispatchPipeline;
    private readonly Stack<Func<Object, Object>> _middlewareStack = new();

    private readonly object _mutex = new();

    /// <summary>
    /// Get the name of this object adapter.
    /// </summary>
    /// <returns>This object adapter's name.</returns>
    public string getName() => _noConfig ? "" : _name;

    /// <summary>
    /// Get the communicator this object adapter belongs to.
    /// </summary>
    /// <returns>This object adapter's communicator.
    /// </returns>
    public Communicator getCommunicator() => _communicator;

    /// <summary>
    /// Activate all endpoints that belong to this object adapter.
    /// After activation, the object adapter can dispatch
    /// requests received through its endpoints.
    /// </summary>
    public void activate()
    {
        LocatorInfo? locatorInfo = null;
        bool printAdapterReady = false;

        lock (_mutex)
        {
            checkForDeactivation();

            //
            // If we've previously been initialized we just need to activate the
            // incoming connection factories and we're done.
            //
            if (_state != StateUninitialized)
            {
                foreach (IncomingConnectionFactory icf in _incomingConnectionFactories)
                {
                    icf.activate();
                }
                _state = StateActive;
                Monitor.PulseAll(_mutex);
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
            if (!_noConfig)
            {
                Properties properties = _instance.initializationData().properties!;
                printAdapterReady = properties.getIcePropertyAsInt("Ice.PrintAdapterReady") > 0;
            }
        }

        try
        {
            var dummy = new Identity(name: "dummy", "");
            updateLocatorRegistry(locatorInfo, createDirectProxy(dummy));
        }
        catch (LocalException)
        {
            //
            // If we couldn't update the locator registry, we let the
            // exception go through and don't activate the adapter to
            // allow to user code to retry activating the adapter
            // later.
            //
            lock (_mutex)
            {
                _state = StateUninitialized;
                Monitor.PulseAll(_mutex);
            }
            throw;
        }

        if (printAdapterReady)
        {
            Console.Out.WriteLine(_name + " ready");
        }

        lock (_mutex)
        {
            Debug.Assert(_state == StateActivating);

            foreach (IncomingConnectionFactory icf in _incomingConnectionFactories)
            {
                icf.activate();
            }

            _state = StateActive;
            Monitor.PulseAll(_mutex);
        }
    }

    /// <summary>
    /// Temporarily hold receiving and dispatching requests.
    /// The object adapter can be reactivated with the
    /// activate operation. &lt;p class="Note"&gt; Holding is not immediate, i.e., after hold returns, the
    /// object adapter might still be active for some time. You can use waitForHold to wait until holding is
    /// complete.
    /// </summary>
    public void hold()
    {
        lock (_mutex)
        {
            checkForDeactivation();
            _state = StateHeld;
            foreach (IncomingConnectionFactory factory in _incomingConnectionFactories)
            {
                factory.hold();
            }
        }
    }

    /// <summary>
    /// Wait until the object adapter holds requests.
    /// Calling hold initiates holding of requests, and
    /// waitForHold only returns when holding of requests has been completed.
    /// </summary>
    public void waitForHold()
    {
        List<IncomingConnectionFactory> incomingConnectionFactories;
        lock (_mutex)
        {
            checkForDeactivation();
            incomingConnectionFactories = new List<IncomingConnectionFactory>(_incomingConnectionFactories);
        }

        foreach (IncomingConnectionFactory factory in incomingConnectionFactories)
        {
            factory.waitUntilHolding();
        }
    }

    /// <summary>
    /// Deactivates this object adapter: stop accepting new connections from clients and close gracefully all incoming
    /// connections created by this object adapter once all outstanding dispatches have completed. If this object
    /// adapter is indirect, this method also unregisters the object adapter from the Locator.
    /// This method does not cancel outstanding dispatches--it lets them execute until completion. A new incoming
    /// request on an existing connection will be accepted and can delay the closure of the connection.
    /// A deactivated object adapter cannot be reactivated again; it can only be destroyed.
    /// </summary>
    public void deactivate()
    {
        lock (_mutex)
        {
            // Wait for activation or a previous deactivation to complete.
            // This is necessary to avoid out of order locator updates.
            while (_state == StateActivating || _state == StateDeactivating)
            {
                Monitor.Wait(_mutex);
            }
            if (_state > StateDeactivating)
            {
                return;
            }
            _state = StateDeactivating;
        }

        // NOTE: the locator infos and incoming connection factory list are immutable at this point.

        try
        {
            updateLocatorRegistry(_locatorInfo, null);
        }
        catch (LocalException)
        {
            // We can't throw exceptions in deactivate so we ignore failures to update the locator registry.
        }

        foreach (IncomingConnectionFactory factory in _incomingConnectionFactories)
        {
            factory.destroy();
        }

        lock (_mutex)
        {
            Debug.Assert(_state == StateDeactivating);
            _state = StateDeactivated;
            Monitor.PulseAll(_mutex);
        }
    }

    /// <summary>
    /// Wait until <see cref="deactivate" /> is called on this object adapter and all connections accepted by this
    /// object adapter are closed. A connection is closed only after all outstanding dispatches on this connection have
    /// completed.
    /// </summary>
    public void waitForDeactivate()
    {
        IncomingConnectionFactory[]? incomingConnectionFactories = null;
        lock (_mutex)
        {
            // Wait for deactivation of the adapter itself.
            while (_state < StateDeactivated)
            {
                Monitor.Wait(_mutex);
            }
            if (_state > StateDeactivated)
            {
                return;
            }

            incomingConnectionFactories = _incomingConnectionFactories.ToArray();
        }

        //
        // Now we wait for until all incoming connection factories are
        // finished.
        //
        foreach (IncomingConnectionFactory factory in incomingConnectionFactories)
        {
            factory.waitUntilFinished();
        }
    }

    /// <summary>
    /// Checks if this object adapter has been deactivated.
    /// </summary>
    /// <returns><see langword="true" /> if <see cref="deactivate"/> has been called on this object adapter; otherwise,
    /// <see langword="false" />.
    /// </returns>
    public bool isDeactivated()
    {
        lock (_mutex)
        {
            return _state >= StateDeactivated;
        }
    }

    /// <summary>
    /// Destroys this object adapter and cleans up all resources held by this object adapter.
    /// Once this method has returned, it is possible to create another object adapter with the same name.
    /// </summary>
    public void destroy()
    {
        //
        // Deactivate and wait for completion.
        //
        deactivate();
        waitForDeactivate();

        lock (_mutex)
        {
            //
            // Only a single thread is allowed to destroy the object
            // adapter. Other threads wait for the destruction to be
            // completed.
            //
            while (_state == StateDestroying)
            {
                Monitor.Wait(_mutex);
            }
            if (_state == StateDestroyed)
            {
                return;
            }
            _state = StateDestroying;

            while (_directCount > 0)
            {
                Monitor.Wait(_mutex);
            }
        }

        if (_routerInfo is not null)
        {
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
        if (_threadPool is not null)
        {
            _threadPool.destroy();
            _threadPool.joinWithAllThreads();
        }

        _objectAdapterFactory?.removeObjectAdapter(this);

        lock (_mutex)
        {
            //
            // We're done, now we can throw away all incoming connection
            // factories.
            //
            _incomingConnectionFactories.Clear();

            //
            // Remove object references (some of them cyclic).
            //
            _threadPool = null;
            _routerInfo = null;
            _publishedEndpoints = [];
            _locatorInfo = null;
            _reference = null;
            _objectAdapterFactory = null;

            _state = StateDestroyed;
            Monitor.PulseAll(_mutex);
        }
    }

    /// <summary>
    /// Install a middleware in this object adapter.
    /// </summary>
    /// <param name="middleware">The middleware to install.</param>
    /// <returns>This object adapter.</returns>
    /// <exception cref="InvalidOperationException">Thrown if the object adapter's dispatch pipeline has already been
    /// created. This creation typically occurs the first time the object adapter dispatches an incoming request.
    /// </exception>
    public ObjectAdapter use(Func<Object, Object> middleware)
    {
        if (_dispatchPipeline.IsValueCreated)
        {
            throw new InvalidOperationException("All middleware must be installed before the first dispatch.");
        }
        _middlewareStack.Push(middleware);
        return this;
    }

    /// <summary>
    /// Add a servant to this object adapter's Active Servant Map.
    /// Note that one servant can implement several Ice
    /// objects by registering the servant with multiple identities. Adding a servant with an identity that is in the
    /// map already throws AlreadyRegisteredException.
    /// </summary>
    /// <param name="servant">The servant to add.</param>
    /// <param name="id">The identity of the Ice object that is implemented by the servant.</param>
    /// <returns>A proxy that matches the given identity and this object adapter.</returns>
    public ObjectPrx add(Object servant, Identity id) => addFacet(servant, id, "");

    /// <summary>
    /// Like add, but with a facet.
    /// Calling add(servant, id) is equivalent to calling
    /// addFacet with an empty facet.
    /// </summary>
    /// <param name="servant">The servant to add.</param>
    /// <param name="identity">The identity of the Ice object that is implemented by the servant.</param>
    /// <param name="facet">The facet. An empty facet means the default facet.</param>
    /// <returns>A proxy that matches the given identity, facet, and this object adapter.</returns>
    public ObjectPrx addFacet(Object servant, Identity identity, string facet)
    {
        lock (_mutex)
        {
            checkForDestruction();
            checkIdentity(identity);
            ArgumentNullException.ThrowIfNull(servant);

            // Create a copy of the Identity argument, in case the caller reuses it.
            var id = new Identity(identity.name, identity.category);
            _servantManager.addServant(servant, id, facet);

            return newProxy(id, facet);
        }
    }

    /// <summary>
    /// Add a servant to this object adapter's Active Servant Map, using an automatically generated UUID as its
    /// identity. Note that the generated UUID identity can be accessed using the proxy's ice_getIdentity operation.
    /// </summary>
    /// <param name="servant">The servant to add.</param>
    /// <returns>A proxy that matches the generated UUID identity and this object adapter.</returns>
    public ObjectPrx addWithUUID(Object servant) => addFacetWithUUID(servant, "");

    /// <summary>
    /// Like addWithUUID, but with a facet.
    /// Calling addWithUUID(servant) is equivalent to calling
    /// addFacetWithUUID with an empty facet.
    /// </summary>
    /// <param name="servant">The servant to add.</param>
    /// <param name="facet">The facet. An empty facet means the default facet.</param>
    /// <returns>A proxy that matches the generated UUID identity, facet, and this object adapter.</returns>
    public ObjectPrx addFacetWithUUID(Object servant, string facet)
    {
        var ident = new Identity(Guid.NewGuid().ToString(), "");
        return addFacet(servant, ident, facet);
    }

    /// <summary>
    /// Add a default servant to handle requests for a specific category.
    /// Adding a default servant for a category for
    /// which a default servant is already registered throws AlreadyRegisteredException. To dispatch operation
    /// calls on servants, the object adapter tries to find a servant for a given Ice object identity and facet in the
    /// following order:
    ///
    /// The object adapter tries to find a servant for the identity and facet in the Active Servant Map.
    /// If no servant has been found in the Active Servant Map, the object adapter tries to find a default servant
    /// for the category component of the identity.
    /// If no servant has been found by any of the preceding steps, the object adapter tries to find a default
    /// servant for an empty category, regardless of the category contained in the identity.
    /// If no servant has been found by any of the preceding steps, the object adapter gives up and the caller
    /// receives ObjectNotExistException or FacetNotExistException.
    /// </summary>
    /// <param name="servant">The default servant.</param>
    /// <param name="category">The category for which the default servant is registered. An empty category means it
    /// will handle all categories.</param>
    public void addDefaultServant(Ice.Object servant, string category)
    {
        ArgumentNullException.ThrowIfNull(servant);

        lock (_mutex)
        {
            checkForDestruction();
            _servantManager.addDefaultServant(servant, category);
        }
    }

    /// <summary>
    /// Remove a servant (that is, the default facet) from the object adapter's Active Servant Map.
    /// </summary>
    /// <param name="id">The identity of the Ice object that is implemented by the servant. If the servant implements multiple
    /// Ice objects, remove has to be called for all those Ice objects. Removing an identity that is not in
    /// the map throws NotRegisteredException.</param>
    /// <returns>The removed servant.</returns>
    public Object remove(Identity id) => removeFacet(id, "");

    /// <summary>
    /// Like remove, but with a facet.
    /// Calling remove(id) is equivalent to calling
    /// removeFacet with an empty facet.
    /// </summary>
    /// <param name="id">The identity of the Ice object that is implemented by the servant.</param>
    /// <param name="facet">The facet. An empty facet means the default facet.</param>
    /// <returns>The removed servant.</returns>
    public Object removeFacet(Identity id, string facet)
    {
        lock (_mutex)
        {
            checkForDestruction();
            checkIdentity(id);

            return _servantManager.removeServant(id, facet);
        }
    }

    /// <summary>
    /// Remove all facets with the given identity from the Active Servant Map.
    /// The operation completely removes the Ice
    /// object, including its default facet. Removing an identity that is not in the map throws
    /// NotRegisteredException.
    /// </summary>
    /// <param name="id">The identity of the Ice object to be removed.</param>
    /// <returns>A collection containing all the facet names and servants of the removed Ice object.</returns>
    public Dictionary<string, Object> removeAllFacets(Identity id)
    {
        lock (_mutex)
        {
            checkForDestruction();
            checkIdentity(id);

            return _servantManager.removeAllFacets(id);
        }
    }

    /// <summary>
    /// Remove the default servant for a specific category.
    /// Attempting to remove a default servant for a category that
    /// is not registered throws NotRegisteredException.
    /// </summary>
    /// <param name="category">The category of the default servant to remove.</param>
    /// <returns>The default servant.</returns>
    public Object removeDefaultServant(string category)
    {
        lock (_mutex)
        {
            checkForDestruction();
            return _servantManager.removeDefaultServant(category);
        }
    }

    /// <summary>
    /// Look up a servant in this object adapter's Active Servant Map by the identity of the Ice object it implements.
    /// This operation only tries to look up a servant in the Active Servant Map. It does not attempt to find a servant
    /// by using any installed ServantLocator.
    /// </summary>
    /// <param name="id">The identity of the Ice object for which the servant should be returned.</param>
    /// <returns>The servant that implements the Ice object with the given identity, or null if no such servant has
    /// been found.</returns>
    public Object? find(Identity id) => findFacet(id, "");

    /// <summary>
    /// Like find, but with a facet.
    /// Calling find(id) is equivalent to calling findFacet
    /// with an empty facet.
    /// </summary>
    /// <param name="id">The identity of the Ice object for which the servant should be returned.</param>
    /// <param name="facet">The facet. An empty facet means the default facet.</param>
    /// <returns>The servant that implements the Ice object with the given identity and facet, or null if no such
    /// servant has been found.</returns>
    public Object? findFacet(Identity id, string facet)
    {
        lock (_mutex)
        {
            checkForDestruction();
            checkIdentity(id);

            return _servantManager.findServant(id, facet);
        }
    }

    /// <summary>
    /// Find all facets with the given identity in the Active Servant Map.
    /// </summary>
    /// <param name="id">The identity of the Ice object for which the facets should be returned.</param>
    /// <returns>A collection containing all the facet names and servants that have been found, or an empty map if there
    /// is no facet for the given identity.</returns>
    public Dictionary<string, Object> findAllFacets(Identity id)
    {
        lock (_mutex)
        {
            checkForDestruction();
            checkIdentity(id);

            return _servantManager.findAllFacets(id);
        }
    }

    /// <summary>
    /// Look up a servant in this object adapter's Active Servant Map, given a proxy.
    /// This operation only tries to lookup a servant in the Active Servant Map. It does not attempt to find a servant
    /// by using any installed ServantLocator.</summary>
    /// <param name="proxy">The proxy for which the servant should be returned.</param>
    /// <returns>The servant that matches the proxy, or null if no such servant has been found.</returns>
    public Object? findByProxy(ObjectPrx proxy)
    {
        lock (_mutex)
        {
            checkForDestruction();
            Reference @ref = ((ObjectPrxHelperBase)proxy).iceReference();
            return findFacet(@ref.getIdentity(), @ref.getFacet());
        }
    }

    /// <summary>
    /// Add a Servant Locator to this object adapter.
    ///
    /// Adding a servant locator for a category for which a servant locator is already registered throws
    /// <see cref="AlreadyRegisteredException" />. To dispatch operation calls on servants, the object adapter tries to
    /// find a servant for a given Ice object identity and facet in the following order:
    ///
    /// The object adapter tries to find a servant for the identity and facet in the Active Servant Map.
    /// If no servant has been found in the Active Servant Map, the object adapter tries to find a servant locator
    /// for the category component of the identity. If a locator is found, the object adapter tries to find a servant
    /// using this locator.
    /// If no servant has been found by any of the preceding steps, the object adapter tries to find a locator for
    /// an empty category, regardless of the category contained in the identity. If a locator is found, the object
    /// adapter tries to find a servant using this locator.
    /// If no servant has been found by any of the preceding steps, the object adapter gives up and the caller
    /// receives <see cref="ObjectNotExistException" /> or <see cref="FacetNotExistException" />.
    /// </summary>
    /// <param name="locator">The locator to add.</param>
    /// <param name="category">The category for which the Servant Locator can locate servants, or an empty string if the
    /// Servant Locator does not belong to any specific category.</param>
    public void addServantLocator(ServantLocator locator, string category)
    {
        lock (_mutex)
        {
            checkForDestruction();
            _servantManager.addServantLocator(locator, category);
        }
    }

    /// <summary>
    /// Remove a Servant Locator from this object adapter.
    /// </summary>
    /// <param name="category">The category for which the Servant Locator can locate servants, or an empty string if the
    /// servant locator does not belong to any specific category.</param>
    /// <returns>The Servant Locator, or throws <see cref="NotRegisteredException"/> if no Servant Locator was found
    /// for the given category.</returns>
    public ServantLocator removeServantLocator(string category)
    {
        lock (_mutex)
        {
            checkForDestruction();
            return _servantManager.removeServantLocator(category);
        }
    }

    /// <summary>
    /// Find a Servant Locator installed with this object adapter.
    /// </summary>
    /// <param name="category">The category for which the Servant Locator can locate servants, or an empty string if the
    /// Servant Locator does not belong to any specific category.
    /// </param>
    /// <returns>The servant locator, or null if no servant locator was found for the given category.</returns>
    public ServantLocator? findServantLocator(string category)
    {
        lock (_mutex)
        {
            checkForDestruction();
            return _servantManager.findServantLocator(category);
        }
    }

    /// <summary>
    /// Find the default servant for a specific category.
    /// </summary>
    /// <param name="category">The category of the default servant to find.</param>
    /// <returns>The default servant or null if no default servant was registered for the category.</returns>
    public Object? findDefaultServant(string category)
    {
        lock (_mutex)
        {
            checkForDestruction();
            return _servantManager.findDefaultServant(category);
        }
    }

    /// <summary>
    /// Gets the dispatch pipeline of this object adapter.
    /// </summary>
    /// <value>The dispatch pipeline.</value>
    public Object dispatchPipeline => _dispatchPipeline.Value;

    /// <summary>
    /// Create a proxy for the object with the given identity.
    /// If this object adapter is configured with an adapter id,
    /// the return value is an indirect proxy that refers to the adapter id. If a replica group id is also defined, the
    /// return value is an indirect proxy that refers to the replica group id. Otherwise, if no adapter id is defined,
    /// the return value is a direct proxy containing this object adapter's published endpoints.
    /// </summary>
    /// <param name="id">The object's identity.</param>
    /// <returns>A proxy for the object with the given identity.</returns>
    public ObjectPrx createProxy(Identity id)
    {
        lock (_mutex)
        {
            checkForDestruction();
            checkIdentity(id);

            return newProxy(id, "");
        }
    }

    /// <summary>
    /// Create a direct proxy for the object with the given identity.
    /// The returned proxy contains this object adapter's
    /// published endpoints.
    /// </summary>
    /// <param name="id">The object's identity.</param>
    /// <returns>A proxy for the object with the given identity.</returns>
    public ObjectPrx createDirectProxy(Identity id)
    {
        lock (_mutex)
        {
            checkForDestruction();
            checkIdentity(id);

            return newDirectProxy(id, "");
        }
    }

    /// <summary>
    /// Create an indirect proxy for the object with the given identity.
    /// If this object adapter is configured with an
    /// adapter id, the return value refers to the adapter id. Otherwise, the return value contains only the object
    /// identity.
    /// </summary>
    /// <param name="id">The object's identity.</param>
    /// <returns>A proxy for the object with the given identity.</returns>
    public ObjectPrx createIndirectProxy(Identity id)
    {
        lock (_mutex)
        {
            checkForDestruction();
            checkIdentity(id);

            return newIndirectProxy(id, "", _id);
        }
    }

    /// <summary>
    /// Set an Ice locator for this object adapter.
    /// By doing so, the object adapter will register itself with the
    /// locator registry when it is activated for the first time. Furthermore, the proxies created by this object
    /// adapter will contain the adapter identifier instead of its endpoints. The adapter identifier must be configured
    /// using the AdapterId property.
    /// </summary>
    /// <param name="locator">The locator used by this object adapter.</param>
    public void setLocator(LocatorPrx? locator)
    {
        lock (_mutex)
        {
            checkForDeactivation();
            _locatorInfo = _instance.locatorManager().get(locator);
        }
    }

    /// <summary>
    /// Get the Ice locator used by this object adapter.
    /// </summary>
    /// <returns>The locator used by this object adapter, or null if no locator is used by this object adapter.
    /// </returns>
    public LocatorPrx? getLocator()
    {
        lock (_mutex)
        {
            return _locatorInfo?.getLocator();
        }
    }

    /// <summary>
    /// Get the set of endpoints configured with this object adapter.
    /// </summary>
    /// <returns>The set of endpoints.</returns>
    public Endpoint[] getEndpoints()
    {
        lock (_mutex)
        {
            var endpoints = new List<Endpoint>();
            foreach (IncomingConnectionFactory factory in _incomingConnectionFactories)
            {
                endpoints.Add(factory.endpoint());
            }
            return endpoints.ToArray();
        }
    }

    /// <summary>
    /// Get the set of endpoints that proxies created by this object adapter will contain.
    /// </summary>
    /// <returns>The set of published endpoints.
    /// </returns>
    public Endpoint[] getPublishedEndpoints()
    {
        lock (_mutex)
        {
            return (Endpoint[])_publishedEndpoints.Clone();
        }
    }

    /// <summary>
    /// Set of the endpoints that proxies created by this object adapter will contain.
    /// </summary>
    /// <param name="newEndpoints">The new set of endpoints that the object adapter will embed in proxies.
    /// </param>
    public void setPublishedEndpoints(Endpoint[] newEndpoints)
    {
        LocatorInfo? locatorInfo = null;
        EndpointI[] oldPublishedEndpoints;

        lock (_mutex)
        {
            checkForDeactivation();
            if (_routerInfo is not null)
            {
                throw new ArgumentException(
                                "can't set published endpoints on object adapter associated with a router");
            }

            oldPublishedEndpoints = _publishedEndpoints;
            _publishedEndpoints = Array.ConvertAll(newEndpoints, endpt => (EndpointI)endpt);
            locatorInfo = _locatorInfo;
        }

        try
        {
            var dummy = new Identity("dummy", "");
            updateLocatorRegistry(locatorInfo, createDirectProxy(dummy));
        }
        catch (LocalException)
        {
            lock (_mutex)
            {
                //
                // Restore the old published endpoints.
                //
                _publishedEndpoints = oldPublishedEndpoints;
                throw;
            }
        }
    }

    internal bool isLocal(Reference r)
    {
        //
        // NOTE: it's important that isLocal() doesn't perform any blocking operations as
        // it can be called for AMI invocations if the proxy has no delegate set yet.
        //

        if (r.isWellKnown())
        {
            //
            // Check the active servant map to see if the well-known
            // proxy is for a local object.
            //
            return _servantManager.hasServant(r.getIdentity());
        }
        else if (r.isIndirect())
        {
            //
            // Proxy is local if the reference adapter id matches this
            // adapter id or replica group id.
            //
            return r.getAdapterId().Equals(_id, StringComparison.Ordinal) ||
                r.getAdapterId().Equals(_replicaGroupId, StringComparison.Ordinal);
        }
        else
        {
            // Proxies which have at least one endpoint in common with the published endpoints are considered local.
            lock (_mutex)
            {
                checkForDestruction();
                EndpointI[] endpoints = r.getEndpoints();
                return _publishedEndpoints.Any(e => endpoints.Any(e.equivalent));
            }
        }
    }

    internal void flushAsyncBatchRequests(Ice.CompressBatch compressBatch, CommunicatorFlushBatchAsync outAsync)
    {
        List<IncomingConnectionFactory> f;
        lock (_mutex)
        {
            f = new List<IncomingConnectionFactory>(_incomingConnectionFactories);
        }

        foreach (IncomingConnectionFactory factory in f)
        {
            factory.flushAsyncBatchRequests(compressBatch, outAsync);
        }
    }

    internal void updateConnectionObservers()
    {
        List<IncomingConnectionFactory> f;
        lock (_mutex)
        {
            f = new List<IncomingConnectionFactory>(_incomingConnectionFactories);
        }

        foreach (IncomingConnectionFactory p in f)
        {
            p.updateConnectionObservers();
        }
    }

    internal void updateThreadObservers()
    {
        Ice.Internal.ThreadPool? threadPool;
        lock (_mutex)
        {
            threadPool = _threadPool;
        }

        threadPool?.updateObservers();
    }

    internal void incDirectCount()
    {
        lock (_mutex)
        {
            checkForDestruction();

            Debug.Assert(_directCount >= 0);
            ++_directCount;
        }
    }

    internal void decDirectCount()
    {
        lock (_mutex)
        {
            // Not check for destruction here!

            Debug.Assert(_instance is not null); // destroy waits for _directCount to reach 0

            Debug.Assert(_directCount > 0);
            if (--_directCount == 0)
            {
                Monitor.PulseAll(_mutex);
            }
        }
    }

    internal Ice.Internal.ThreadPool getThreadPool()
    {
        // No mutex lock necessary, _threadPool and _instance are
        // immutable after creation until they are removed in
        // destroy().

        // Not check for deactivation here!

        Debug.Assert(_instance is not null); // Must not be called after destroy().

        if (_threadPool is not null)
        {
            return _threadPool;
        }
        else
        {
            return _instance.serverThreadPool();
        }
    }

    internal void setAdapterOnConnection(Ice.ConnectionI connection)
    {
        lock (_mutex)
        {
            checkForDestruction();
            connection.setAdapterFromAdapter(this);
        }
    }

    internal int messageSizeMax() => _messageSizeMax;

    //
    // Only for use by ObjectAdapterFactory
    //
    internal ObjectAdapter(
        Instance instance,
        Communicator communicator,
        ObjectAdapterFactory objectAdapterFactory,
        string name,
        RouterPrx? router,
        bool noConfig,
        SslServerAuthenticationOptions? serverAuthenticationOptions)
    {
        _instance = instance;
        _communicator = communicator;
        _objectAdapterFactory = objectAdapterFactory;
        _servantManager = new ServantManager(instance, name);

        _dispatchPipeline = new Lazy<Object>(createDispatchPipeline);
        _name = name;
        _incomingConnectionFactories = [];
        _publishedEndpoints = [];
        _routerInfo = null;
        _directCount = 0;
        _noConfig = noConfig;
        _serverAuthenticationOptions = serverAuthenticationOptions;

        // Install default middleware depending on the communicator's configuration.
        if (_instance.initializationData().logger is Logger logger)
        {
            int warningLevel = _instance.initializationData().properties!.getIcePropertyAsInt("Ice.Warn.Dispatch");
            if (_instance.traceLevels().dispatch > 0 || warningLevel > 0)
            {
                use(next =>
                    new LoggerMiddleware(
                        next,
                        logger,
                        _instance.traceLevels().dispatch,
                        _instance.traceLevels().dispatchCat,
                        warningLevel,
                        _instance.toStringMode()));
            }
        }
        if (_instance.initializationData().observer is CommunicatorObserver observer)
        {
            use(next => new ObserverMiddleware(next, observer));
        }

        if (_noConfig)
        {
            _id = "";
            _replicaGroupId = "";
            _reference = _instance.referenceFactory().create("dummy -t", "");
            return;
        }

        Properties properties = _instance.initializationData().properties!;
        Properties.validatePropertiesWithPrefix(_name, properties, PropertyNames.ObjectAdapterProps);

        //
        // Make sure named adapter has configuration.
        //
        if (router is null && properties.getPropertiesForPrefix($"{_name}.").Count == 0)
        {
            //
            // These need to be set to prevent warnings/asserts in the destructor.
            //
            _state = StateDestroyed;
            _incomingConnectionFactories = [];

            throw new InitializationException($"Object adapter '{name}' requires configuration.");
        }

        _id = properties.getProperty(_name + ".AdapterId");
        _replicaGroupId = properties.getProperty(_name + ".ReplicaGroupId");

        //
        // Setup a reference to be used to get the default proxy options
        // when creating new proxies. By default, create twoway proxies.
        //
        string proxyOptions = properties.getPropertyWithDefault(_name + ".ProxyOptions", "-t");
        try
        {
            _reference = _instance.referenceFactory().create("dummy " + proxyOptions, "");
        }
        catch (ParseException ex)
        {
            throw new InitializationException(
                $"Invalid proxy options '{proxyOptions}' for object adapter '{_name}'.",
                ex);
        }

        {
            int defaultMessageSizeMax = instance.messageSizeMax() / 1024;
            int num = properties.getPropertyAsIntWithDefault(_name + ".MessageSizeMax", defaultMessageSizeMax);
            if (num < 1 || num > 0x7fffffff / 1024)
            {
                _messageSizeMax = 0x7fffffff;
            }
            else
            {
                _messageSizeMax = num * 1024; // Property is in kilobytes, _messageSizeMax in bytes
            }
        }

        try
        {
            int threadPoolSize = properties.getPropertyAsInt(_name + ".ThreadPool.Size");
            int threadPoolSizeMax = properties.getPropertyAsInt(_name + ".ThreadPool.SizeMax");
            if (threadPoolSize > 0 || threadPoolSizeMax > 0)
            {
                _threadPool = new Ice.Internal.ThreadPool(_instance, _name + ".ThreadPool", 0);
            }

            router ??= RouterPrxHelper.uncheckedCast(communicator.propertyToProxy(_name + ".Router"));
            if (router is not null)
            {
                _routerInfo = _instance.routerManager().get(router);
                Debug.Assert(_routerInfo is not null);

                if (properties.getProperty($"{_name}.Endpoints").Length > 0)
                {
                    throw new InitializationException(
                        "An object adapter with a router cannot accept incoming connections.");
                }

                //
                // Make sure this router is not already registered with another adapter.
                //
                if (_routerInfo.getAdapter() is not null)
                {
                    throw new AlreadyRegisteredException(
                        "object adapter with router",
                        Util.identityToString(router.ice_getIdentity(), _instance.toStringMode()));
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
            }
            else
            {
                //
                // Parse the endpoints, but don't store them in the adapter. The connection
                // factory might change it, for example, to fill in the real port number.
                //
                List<EndpointI> endpoints = parseEndpoints(properties.getProperty(_name + ".Endpoints"), true);
                foreach (EndpointI endp in endpoints)
                {
                    foreach (EndpointI expanded in endp.expandHost())
                    {
                        var factory = new IncomingConnectionFactory(instance, expanded, this);
                        _incomingConnectionFactories.Add(factory);
                    }
                }
                if (endpoints.Count == 0)
                {
                    TraceLevels tl = _instance.traceLevels();
                    if (tl.network >= 2)
                    {
                        _instance.initializationData().logger!.trace(
                            tl.networkCat,
                            $"created adapter '{_name}' without endpoints");
                    }
                }
            }

            //
            // Parse published endpoints.
            //
            _publishedEndpoints = computePublishedEndpoints();

            if (properties.getProperty(_name + ".Locator").Length > 0)
            {
                setLocator(LocatorPrxHelper.uncheckedCast(communicator.propertyToProxy(_name + ".Locator")));
            }
            else
            {
                setLocator(_instance.referenceFactory().getDefaultLocator());
            }
        }
        catch (LocalException)
        {
            destroy();
            throw;
        }
    }

    internal static void checkIdentity(Identity ident)
    {
        if (ident.name.Length == 0)
        {
            throw new ArgumentException("The name of an Ice object identity cannot be empty.", nameof(ident));
        }
    }

    internal SslServerAuthenticationOptions? getServerAuthenticationOptions() => _serverAuthenticationOptions;

    private ObjectPrx newProxy(Identity ident, string facet)
    {
        if (_id.Length == 0)
        {
            return newDirectProxy(ident, facet);
        }
        else if (_replicaGroupId.Length == 0)
        {
            return newIndirectProxy(ident, facet, _id);
        }
        else
        {
            return newIndirectProxy(ident, facet, _replicaGroupId);
        }
    }

    private ObjectPrx newDirectProxy(Identity ident, string facet) =>
        new ObjectPrxHelper(_instance.referenceFactory().create(ident, facet, _reference, _publishedEndpoints));

    private ObjectPrx newIndirectProxy(Identity ident, string facet, string id) =>
        new ObjectPrxHelper(_instance.referenceFactory().create(ident, facet, _reference, id));

    private void checkForDeactivation()
    {
        checkForDestruction();

        if (_state >= StateDeactivating)
        {
            throw new ObjectAdapterDeactivatedException(getName());
        }
    }

    private void checkForDestruction()
    {
        if (_state >= StateDestroying)
        {
            throw new ObjectAdapterDestroyedException(getName());
        }
    }

    private List<EndpointI> parseEndpoints(string endpts, bool oaEndpoints)
    {
        int beg;
        int end = 0;

        string delim = " \t\n\r";

        var endpoints = new List<EndpointI>();
        while (end < endpts.Length)
        {
            beg = Ice.UtilInternal.StringUtil.findFirstNotOf(endpts, delim, end);
            if (beg == -1)
            {
                if (endpoints.Count != 0)
                {
                    throw new ParseException("invalid empty object adapter endpoint");
                }
                break;
            }

            end = beg;
            while (true)
            {
                end = endpts.IndexOf(':', end);
                if (end == -1)
                {
                    end = endpts.Length;
                    break;
                }
                else
                {
                    bool quoted = false;
                    int quote = beg;
                    while (true)
                    {
                        quote = endpts.IndexOf('\"', quote);
                        if (quote == -1 || end < quote)
                        {
                            break;
                        }
                        else
                        {
                            quote = endpts.IndexOf('\"', ++quote);
                            if (quote == -1)
                            {
                                break;
                            }
                            else if (end < quote)
                            {
                                quoted = true;
                                break;
                            }
                            ++quote;
                        }
                    }
                    if (!quoted)
                    {
                        break;
                    }
                    ++end;
                }
            }

            if (end == beg)
            {
                throw new ParseException("invalid empty object adapter endpoint");
            }

            string s = endpts[beg..end];
            EndpointI endp = _instance.endpointFactoryManager().create(s, oaEndpoints) ??
                throw new ParseException($"invalid object adapter endpoint '{s}'");
            endpoints.Add(endp);

            ++end;
        }

        return endpoints;
    }

    private EndpointI[] computePublishedEndpoints()
    {
        IEnumerable<EndpointI> endpoints;
        if (_routerInfo is not null)
        {
            // Get the router's server proxy endpoints and use them as the published endpoints.
            endpoints = _routerInfo.getServerEndpoints();
        }
        else
        {
            // Parse published endpoints. If set, these are used instead of the connection factory endpoints.
            endpoints = parseEndpoints(
                _instance.initializationData().properties!.getProperty($"{_name}.PublishedEndpoints"),
                oaEndpoints: false);

            if (!endpoints.Any())
            {
                // If the PublishedEndpoints property isn't set, we compute the published endpoints from the factory
                // endpoints.
                endpoints = _incomingConnectionFactories.Select(f => f.endpoint());

                // Remove all loopback/multicast endpoints.
                IEnumerable<EndpointI> endpointsNoLoopback = endpoints.Where(e => !e.isLoopbackOrMulticast());

                // Retrieve published host
                string publishedHost = _instance.initializationData().properties!.getProperty($"{_name}.PublishedHost");

                if (endpointsNoLoopback.Any())
                {
                    endpoints = endpointsNoLoopback;

                    // For non-loopback & non-multicast endpoints, we use the fully qualified name of the local host as
                    // default for publishedHost.
                    if (publishedHost.Length == 0)
                    {
                        publishedHost = Dns.GetHostEntry("").HostName; // fully qualified name of local host
                    }
                }

                // Replace the host in all endpoints by publishedHost (when applicable) and clear all local options.
                endpoints = endpoints.Select(e => e.toPublishedEndpoint(publishedHost)).Distinct();
            }
        }

        EndpointI[] endpointsArray = endpoints.ToArray();

        if (_instance.traceLevels().network >= 1 && endpointsArray.Length > 0)
        {
            var s = new StringBuilder("published endpoints for object adapter '");
            s.Append(_name);
            s.Append("':\n");
            bool first = true;
            foreach (EndpointI endpoint in endpointsArray)
            {
                if (!first)
                {
                    s.Append(':');
                }
                s.Append(endpoint.ToString());
                first = false;
            }
            _instance.initializationData().logger!.trace(_instance.traceLevels().networkCat, s.ToString());
        }

        return endpointsArray;
    }

    private void updateLocatorRegistry(LocatorInfo? locatorInfo, ObjectPrx? proxy)
    {
        if (_id.Length == 0 || locatorInfo is null)
        {
            return; // Nothing to update.
        }

        //
        // Call on the locator registry outside the synchronization to
        // blocking other threads that need to lock this OA.
        //
        LocatorRegistryPrx locatorRegistry = locatorInfo.getLocatorRegistry();
        if (locatorRegistry is null)
        {
            return;
        }

        try
        {
            if (_replicaGroupId.Length == 0)
            {
                locatorRegistry.setAdapterDirectProxy(_id, proxy);
            }
            else
            {
                locatorRegistry.setReplicatedAdapterDirectProxy(_id, _replicaGroupId, proxy);
            }
        }
        catch (AdapterNotFoundException)
        {
            if (_instance!.traceLevels().location >= 1)
            {
                var s = new StringBuilder();
                s.Append("couldn't update object adapter `" + _id + "' endpoints with the locator registry:\n");
                s.Append("the object adapter is not known to the locator registry");
                _instance.initializationData().logger!.trace(_instance.traceLevels().locationCat, s.ToString());
            }

            throw new NotRegisteredException("object adapter", _id);
        }
        catch (InvalidReplicaGroupIdException)
        {
            if (_instance.traceLevels().location >= 1)
            {
                var s = new StringBuilder();
                s.Append("couldn't update object adapter `" + _id + "' endpoints with the locator registry:\n");
                s.Append("the replica group `" + _replicaGroupId + "' is not known to the locator registry");
                _instance.initializationData().logger!.trace(_instance.traceLevels().locationCat, s.ToString());
            }

            throw new NotRegisteredException("replica group", _replicaGroupId);
        }
        catch (AdapterAlreadyActiveException)
        {
            if (_instance.traceLevels().location >= 1)
            {
                var s = new StringBuilder();
                s.Append("couldn't update object adapter `" + _id + "' endpoints with the locator registry:\n");
                s.Append("the object adapter endpoints are already set");
                _instance.initializationData().logger!.trace(_instance.traceLevels().locationCat, s.ToString());
            }

            throw new ObjectAdapterIdInUseException(_id);
        }
        catch (ObjectAdapterDeactivatedException)
        {
            // Expected if collocated call and OA is deactivated, ignore.
        }
        catch (ObjectAdapterDestroyedException)
        {
            // Ignore
        }
        catch (CommunicatorDestroyedException)
        {
            // Ignore
        }
        catch (LocalException e)
        {
            if (_instance.traceLevels().location >= 1)
            {
                var s = new StringBuilder();
                s.Append("couldn't update object adapter `" + _id + "' endpoints with the locator registry:\n");
                s.Append(e.ToString());
                _instance.initializationData().logger!.trace(_instance.traceLevels().locationCat, s.ToString());
            }
            throw; // TODO: Shall we raise a special exception instead of a non obvious local exception?
        }

        if (_instance.traceLevels().location >= 1)
        {
            var s = new StringBuilder();
            s.Append("updated object adapter `" + _id + "' endpoints with the locator registry\n");
            s.Append("endpoints = ");
            if (proxy is not null)
            {
                Endpoint[] endpoints = proxy.ice_getEndpoints();
                for (int i = 0; i < endpoints.Length; i++)
                {
                    s.Append(endpoints[i].ToString());
                    if (i + 1 < endpoints.Length)
                    {
                        s.Append(':');
                    }
                }
            }
            _instance.initializationData().logger!.trace(_instance.traceLevels().locationCat, s.ToString());
        }
    }

    private Object createDispatchPipeline()
    {
        Object dispatchPipeline = _servantManager; // the "final" dispatcher
        foreach (Func<Object, Object> middleware in _middlewareStack)
        {
            dispatchPipeline = middleware(dispatchPipeline);
        }
        _middlewareStack.Clear(); // we no longer need these functions
        return dispatchPipeline;
    }
}
