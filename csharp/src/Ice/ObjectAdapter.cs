//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;
using System.Threading.Tasks;
using IceInternal;

namespace Ice
{
    // TODO: rename Disp to Dispatcher and fix its signature
    public delegate Task<OutputStream?>? Disp(Incoming inS, Current current);

    public sealed class ObjectAdapter
    {
        /// <summary>
        /// Get the name of this object adapter.
        /// </summary>
        /// <returns>This object adapter's name.</returns>
        public string GetName()
        {
            //
            // No mutex lock necessary, _name is immutable.
            //
            return _noConfig ? "" : _name;
        }

        /// <summary>
        /// Get the communicator this object adapter belongs to.
        /// </summary>
        /// <returns>This object adapter's communicator.
        ///
        /// </returns>
        public Communicator Communicator
        {
            get
            {
                lock (this)
                {
                    checkForDeactivation();
                    Debug.Assert(_communicator != null);
                    return _communicator;
                }
            }
        }

        /// <summary>
        /// Activate all endpoints that belong to this object adapter.
        /// After activation, the object adapter can dispatch requests
        /// received through its endpoints.
        ///
        /// </summary>
        public void Activate()
        {
            LocatorInfo? locatorInfo = null;
            bool printAdapterReady = false;

            lock (this)
            {
                checkForDeactivation();

                //
                // If we've previously been initialized we just need to activate the
                // incoming connection factories and we're done.
                //
                if (_state != StateUninitialized)
                {
                    Debug.Assert(_incomingConnectionFactories != null);
                    foreach (IncomingConnectionFactory icf in _incomingConnectionFactories)
                    {
                        icf.Activate();
                    }
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
                    printAdapterReady = _communicator!.GetPropertyAsInt("Ice.PrintAdapterReady") > 0;
                }
            }

            try
            {
                UpdateLocatorRegistry(locatorInfo, CreateDirectProxy(new Identity("dummy", ""), IObjectPrx.Factory));
            }
            catch (LocalException)
            {
                //
                // If we couldn't update the locator registry, we let the
                // exception go through and don't activate the adapter to
                // allow to user code to retry activating the adapter
                // later.
                //
                lock (this)
                {
                    _state = StateUninitialized;
                    System.Threading.Monitor.PulseAll(this);
                }
                throw;
            }

            if (printAdapterReady)
            {
                Console.Out.WriteLine($"{_name} ready");
            }

            lock (this)
            {
                Debug.Assert(_state == StateActivating);
                Debug.Assert(_incomingConnectionFactories != null);
                foreach (IncomingConnectionFactory icf in _incomingConnectionFactories)
                {
                    icf.Activate();
                }

                _state = StateActive;
                System.Threading.Monitor.PulseAll(this);
            }
        }

        /// <summary>
        /// Temporarily hold receiving and dispatching requests.
        /// The object
        /// adapter can be reactivated with the activate operation.
        ///
        ///  Holding is not immediate, i.e., after hold
        /// returns, the object adapter might still be active for some
        /// time. You can use waitForHold to wait until holding is
        /// complete.
        ///
        /// </summary>
        public void Hold()
        {
            lock (this)
            {
                checkForDeactivation();
                _state = StateHeld;
                Debug.Assert(_incomingConnectionFactories != null);
                foreach (IncomingConnectionFactory factory in _incomingConnectionFactories)
                {
                    factory.Hold();
                }
            }
        }

        /// <summary>
        /// Wait until the object adapter holds requests.
        /// Calling hold
        /// initiates holding of requests, and waitForHold only returns
        /// when holding of requests has been completed.
        ///
        /// </summary>
        public void WaitForHold()
        {
            List<IncomingConnectionFactory> incomingConnectionFactories;
            lock (this)
            {
                checkForDeactivation();

                incomingConnectionFactories = new List<IncomingConnectionFactory>(_incomingConnectionFactories);
            }

            foreach (IncomingConnectionFactory factory in incomingConnectionFactories)
            {
                factory.WaitUntilHolding();
            }
        }

        /// <summary>
        /// Deactivate all endpoints that belong to this object adapter.
        /// After deactivation, the object adapter stops receiving
        /// requests through its endpoints. IObject adapters that have been
        /// deactivated must not be reactivated again, and cannot be used
        /// otherwise. Attempts to use a deactivated object adapter raise
        /// ObjectAdapterDeactivatedException however, attempts to
        /// deactivate an already deactivated object adapter are
        /// ignored and do nothing. Once deactivated, it is possible to
        /// destroy the adapter to clean up resources and then create and
        /// activate a new adapter with the same name.
        ///
        ///  After deactivate returns, no new requests
        /// are processed by the object adapter. However, requests that
        /// have been started before deactivate was called might
        /// still be active. You can use waitForDeactivate to wait
        /// for the completion of all requests for this object adapter.
        ///
        /// </summary>
        public void Deactivate()
        {
            lock (this)
            {
                //
                //
                // Wait for activation to complete. This is necessary to not
                // get out of order locator updates.
                //
                while (_state == StateActivating || _state == StateDeactivating)
                {
                    System.Threading.Monitor.Wait(this);
                }
                if (_state > StateDeactivating)
                {
                    return;
                }
                _state = StateDeactivating;
                Debug.Assert(_communicator != null);
            }

            //
            // NOTE: the router/locator infos and incoming connection
            // factory list are immutable at this point.
            //

            try
            {
                if (_routerInfo != null)
                {
                    //
                    // Remove entry from the router manager.
                    //
                    _communicator.EraseRouterInfo(_routerInfo.Router);

                    //
                    // Clear this object adapter with the router.
                    //
                    _routerInfo.Adapter = null;
                }

                UpdateLocatorRegistry(_locatorInfo, null);
            }
            catch (LocalException)
            {
                //
                // We can't throw exceptions in deactivate so we ignore
                // failures to update the locator registry.
                //
            }

            Debug.Assert(_incomingConnectionFactories != null);
            foreach (IncomingConnectionFactory factory in _incomingConnectionFactories)
            {
                factory.Destroy();
            }

            _communicator.OutgoingConnectionFactory().RemoveAdapter(this);

            lock (this)
            {
                Debug.Assert(_state == StateDeactivating);
                _state = StateDeactivated;
                System.Threading.Monitor.PulseAll(this);
            }
        }

        /// <summary>
        /// Wait until the object adapter has deactivated.
        /// Calling
        /// deactivate initiates object adapter deactivation, and
        /// waitForDeactivate only returns when deactivation has
        /// been completed.
        ///
        /// </summary>
        public void WaitForDeactivate()
        {
            IncomingConnectionFactory[] incomingConnectionFactories;
            lock (this)
            {
                //
                // Wait for deactivation of the adapter itself, and
                // for the return of all direct method calls using this
                // adapter.
                //
                while ((_state < StateDeactivated) || _directCount > 0)
                {
                    System.Threading.Monitor.Wait(this);
                }
                if (_state > StateDeactivated)
                {
                    return;
                }
                Debug.Assert(_incomingConnectionFactories != null);
                incomingConnectionFactories = _incomingConnectionFactories.ToArray();
            }

            //
            // Now we wait for until all incoming connection factories are
            // finished.
            //
            foreach (IncomingConnectionFactory factory in incomingConnectionFactories)
            {
                factory.WaitUntilFinished();
            }
        }

        /// <summary>
        /// Check whether object adapter has been deactivated.
        /// </summary>
        /// <returns>Whether adapter has been deactivated.
        ///
        /// </returns>
        public bool IsDeactivated()
        {
            lock (this)
            {
                return _state >= StateDeactivated;
            }
        }

        /// <summary>
        /// Destroys the object adapter and cleans up all resources held by
        /// the object adapter.
        /// If the object adapter has not yet been
        /// deactivated, destroy implicitly initiates the deactivation
        /// and waits for it to finish. Subsequent calls to destroy are
        /// ignored. Once destroy has returned, it is possible to create
        /// another object adapter with the same name.
        ///
        /// </summary>
        public void Destroy()
        {
            //
            // Deactivate and wait for completion.
            //
            Deactivate();
            WaitForDeactivate();

            lock (this)
            {
                //
                // Only a single thread is allowed to destroy the object
                // adapter. Other threads wait for the destruction to be
                // completed.
                //
                while (_state == StateDestroying)
                {
                    System.Threading.Monitor.Wait(this);
                }
                if (_state == StateDestroyed)
                {
                    return;
                }
                _state = StateDestroying;
            }

            //
            // Now it's also time to clean up our servants and servant
            // locators.
            //
            _servantManager.Destroy();

            //
            // Destroy the thread pool.
            //
            if (_threadPool != null)
            {
                _threadPool.Destroy();
                _threadPool.JoinWithAllThreads();
            }

            if (_communicator != null)
            {
                _communicator.RemoveObjectAdapter(this);
            }

            lock (this)
            {
                //
                // We're done, now we can throw away all incoming connection
                // factories.
                //
                Debug.Assert(_incomingConnectionFactories != null);
                _incomingConnectionFactories.Clear();

                //
                // Remove object references (some of them cyclic).
                //
                _communicator = null;
                _threadPool = null;
                _routerInfo = null;
                _publishedEndpoints = Array.Empty<Endpoint>();
                _locatorInfo = null;
                _reference = null;

                _state = StateDestroyed;
                System.Threading.Monitor.PulseAll(this);
            }
        }

        /// <summary>
        /// Add a servant to this object adapter's Active Servant Map (ASM).
        /// You can incarnate several Ice objects with the same servant by adding this servant with multiple identities
        /// in the ASM. Adding a servant with an identity that is already in the ASM throws AlreadyRegisteredException.
        /// </summary>
        /// <param name="servant">The servant to add.</param>
        /// <param name="proxyFactory">The proxy factory used to manufacture the returned proxy. Pass INamePrx.Factory
        /// for this parameter. See <see cref="CreateProxy"/>.</param>
        /// <param name="identity">The identity of the Ice object incarnated by this servant. When null, the
        /// ObjectAdapter creates a new unique identity with a UUID name and empty category.</param>
        /// <param name="facet">The facet of the Ice object. An empty facet means the default facet.</param>
        /// <returns>A proxy associated with this object adapter, object identity and facet.</returns>
        public T Add<T>(IObject servant, ProxyFactory<T> proxyFactory, Identity? identity = null, string facet = "")
            where T : class, IObjectPrx
        {
            lock (this)
            {
                identity = identity ?? new Identity(Guid.NewGuid().ToString(), "");
                checkForDeactivation();
                checkIdentity(identity.Value);

                _servantManager.AddServant(servant, identity.Value, facet);
                return newProxy(identity.Value, proxyFactory, facet);
            }
        }

        /// <summary>
        /// Add a servant to this object adapter's Active Servant Map (ASM).
        /// You can incarnate several Ice objects with the same servant by adding this servant with multiple identities
        /// in the ASM. Adding a servant with an identity that is already in the ASM throws AlreadyRegisteredException.
        /// </summary>
        /// <param name="servant">The servant to add.</param>
        /// <param name="proxyFactory">The proxy factory used to manufacture the returned proxy. Pass INamePrx.Factory
        /// for this parameter. See <see cref="CreateProxy"/>.</param>
        /// <param name="identity">The stringified identity of the Ice object incarnated by this servant.</param>
        /// <param name="facet">The facet of the Ice object. An empty facet means the default facet.</param>
        /// <returns>A proxy associated with this object adapter, object identity and facet.</returns>
        public T Add<T>(IObject servant, ProxyFactory<T> proxyFactory, string identity, string facet = "")
            where T : class, IObjectPrx => Add(servant, proxyFactory, Identity.Parse(identity), facet);

        /// <summary>
        /// Add a servant to this object adapter's Active Servant Map (ASM).
        /// You can incarnate several Ice objects with the same servant by adding this servant with multiple identities
        /// in the ASM. Adding a servant with an identity that is already in the ASM throws AlreadyRegisteredException.
        /// </summary>
        /// <param name="servant">The servant to add.</param>
        /// <param name="identity">The identity of the Ice object incarnated by this servant.</param>
        /// <param name="facet">The facet of the Ice object. An empty facet means the default facet.</param>
        public void Add(IObject servant, Identity identity, string facet = "")
        {
            lock (this)
            {
                checkForDeactivation();
                checkIdentity(identity);

                _servantManager.AddServant(servant, identity, facet);
            }
        }

        /// <summary>
        /// Add a servant to this object adapter's Active Servant Map (ASM).
        /// You can incarnate several Ice objects with the same servant by adding this servant with multiple identities
        /// in the ASM. Adding a servant with an identity that is already in the ASM throws AlreadyRegisteredException.
        /// </summary>
        /// <param name="servant">The servant to add.</param>
        /// <param name="identity">The stringified identity of the Ice object incarnated by this servant.</param>
        /// <param name="facet">The facet of the Ice object. An empty facet means the default facet.</param>
        public void Add(IObject servant, string identity, string facet = "")
            => Add(servant, Identity.Parse(identity), facet);

        public IObject? Remove(Identity identity, string facet = "")
        {
            lock (this)
            {
                checkForDeactivation();
                checkIdentity(identity);

                return _servantManager.RemoveServant(identity, facet);
            }
        }

        public IObject? Remove(string identity, string facet = "") => Remove(Identity.Parse(identity), facet);

        public Dictionary<string, IObject> RemoveAllFacets(Identity identity)
        {
            lock (this)
            {
                checkForDeactivation();
                checkIdentity(identity);

                return _servantManager.RemoveAllFacets(identity);
            }
        }

        public Dictionary<string, IObject> RemoveAllFacets(string identity)
            => RemoveAllFacets(Identity.Parse(identity));

        public IObject? Find(Identity identity, string facet = "")
        {
            lock (this)
            {
                checkForDeactivation();
                checkIdentity(identity);

                return _servantManager.FindServant(identity, facet);
            }
        }

        public IObject? Find(string identity, string facet = "") => Find(Identity.Parse(identity), facet);

        public Dictionary<string, IObject> FindAllFacets(Identity identity)
        {
            lock (this)
            {
                checkForDeactivation();
                checkIdentity(identity);

                return _servantManager.FindAllFacets(identity);
            }
        }

        public Dictionary<string, IObject> FindAllFacets(string identity) => FindAllFacets(Identity.Parse(identity));

        public void AddDefaultForCategory(string category, IObject servant, string facet = "")
        {
            lock (this)
            {
                checkForDeactivation();
                _servantManager.AddDefaultServant(servant, category);
            }
        }

        public IObject? RemoveDefaultForCategory(string category, string facet = "")
        {
            lock (this)
            {
                checkForDeactivation();
                return _servantManager.RemoveDefaultServant(category);
            }
        }

        public void AddDefault(IObject servant, string facet = "")
        {
            lock (this)
            {
                checkForDeactivation();
                _servantManager.AddDefaultServant(servant, "");
            }
        }

        public IObject? RemoveDefault(string facet = "")
        {
            lock (this)
            {
                checkForDeactivation();
                return _servantManager.RemoveDefaultServant("");
            }
        }

        /// <summary>
        /// Create a proxy for the object with the given identity.
        /// If this
        /// object adapter is configured with an adapter id, the return
        /// value is an indirect proxy that refers to the adapter id. If
        /// a replica group id is also defined, the return value is an
        /// indirect proxy that refers to the replica group id. Otherwise,
        /// if no adapter id is defined, the return value is a direct
        /// proxy containing this object adapter's published endpoints.
        ///
        /// </summary>
        /// <param name="identity">The object's identity.
        ///
        /// </param>
        /// <returns>A proxy for the object with the given identity.
        ///
        /// </returns>
        public T CreateProxy<T>(string identity, ProxyFactory<T> factory) where T : class, IObjectPrx
            => CreateProxy(Identity.Parse(identity), factory);

        /// <summary>
        /// Create a proxy for the object with the given identity.
        /// If this
        /// object adapter is configured with an adapter id, the return
        /// value is an indirect proxy that refers to the adapter id. If
        /// a replica group id is also defined, the return value is an
        /// indirect proxy that refers to the replica group id. Otherwise,
        /// if no adapter id is defined, the return value is a direct
        /// proxy containing this object adapter's published endpoints.
        ///
        /// </summary>
        /// <param name="identity">The object's identity.
        ///
        /// </param>
        /// <returns>A proxy for the object with the given identity.
        ///
        /// </returns>
        public T CreateProxy<T>(Identity identity, ProxyFactory<T> factory) where T : class, IObjectPrx
        {
            lock (this)
            {
                checkForDeactivation();
                checkIdentity(identity);

                return newProxy(identity, factory, "");
            }
        }

        /// <summary>
        /// Create a direct proxy for the object with the given identity.
        /// The returned proxy contains this object adapter's published
        /// endpoints.
        ///
        /// </summary>
        /// <param name="identity">The object's identity.
        ///
        /// </param>
        /// <returns>A proxy for the object with the given identity.
        ///
        /// </returns>
        public T CreateDirectProxy<T>(string identity, ProxyFactory<T> factory) where T : class, IObjectPrx
            => CreateDirectProxy(Identity.Parse(identity), factory);

        /// <summary>
        /// Create a direct proxy for the object with the given identity.
        /// The returned proxy contains this object adapter's published
        /// endpoints.
        ///
        /// </summary>
        /// <param name="identity">The object's identity.
        ///
        /// </param>
        /// <returns>A proxy for the object with the given identity.
        ///
        /// </returns>
        public T CreateDirectProxy<T>(Identity identity, ProxyFactory<T> factory) where T : class, IObjectPrx
        {
            lock (this)
            {
                checkForDeactivation();
                checkIdentity(identity);

                return newDirectProxy(identity, factory, "");
            }
        }

        /// <summary>
        /// Create an indirect proxy for the object with the given identity.
        /// If this object adapter is configured with an adapter id, the
        /// return value refers to the adapter id. Otherwise, the return
        /// value contains only the object identity.
        ///
        /// </summary>
        /// <param name="identity">The object's identity.
        ///
        /// </param>
        /// <returns>A proxy for the object with the given identity.
        ///
        /// </returns>
        public T CreateIndirectProxy<T>(string identity, ProxyFactory<T> factory) where T : class, IObjectPrx
            => CreateIndirectProxy(Identity.Parse(identity), factory);

        /// <summary>
        /// Create an indirect proxy for the object with the given identity.
        /// If this object adapter is configured with an adapter id, the
        /// return value refers to the adapter id. Otherwise, the return
        /// value contains only the object identity.
        ///
        /// </summary>
        /// <param name="identity">The object's identity.
        ///
        /// </param>
        /// <returns>A proxy for the object with the given identity.
        ///
        /// </returns>
        public T CreateIndirectProxy<T>(Identity identity, ProxyFactory<T> factory) where T : class, IObjectPrx
        {
            lock (this)
            {
                checkForDeactivation();
                checkIdentity(identity);

                return newIndirectProxy(identity, factory, "", _id);
            }
        }

        /// <summary>
        /// Set an Ice locator for this object adapter.
        /// By doing so, the
        /// object adapter will register itself with the locator registry
        /// when it is activated for the first time. Furthermore, the proxies
        /// created by this object adapter will contain the adapter identifier
        /// instead of its endpoints. The adapter identifier must be configured
        /// using the AdapterId property.
        ///
        /// </summary>
        /// <param name="locator">The locator used by this object adapter.
        ///
        /// </param>
        public void SetLocator(ILocatorPrx? locator)
        {
            lock (this)
            {
                checkForDeactivation();

                if (locator != null)
                {
                    _locatorInfo = _communicator!.GetLocatorInfo(locator);
                }
                else
                {
                    _locatorInfo = null;
                }

            }
        }

        /// <summary>
        /// Get the Ice locator used by this object adapter.
        /// </summary>
        /// <returns> The locator used by this object adapter, or null if no locator is
        /// used by this object adapter.
        ///
        /// </returns>
        public ILocatorPrx? GetLocator()
        {
            lock (this)
            {
                checkForDeactivation();

                if (_locatorInfo == null)
                {
                    return null;
                }
                else
                {
                    return _locatorInfo.Locator;
                }
            }
        }

        /// <summary>
        /// Get the set of endpoints configured with this object adapter.
        /// </summary>
        /// <returns>The set of endpoints.
        ///
        /// </returns>
        public IEndpoint[] GetEndpoints()
        {
            lock (this)
            {
                List<IEndpoint> endpoints = new List<IEndpoint>();
                Debug.Assert(_incomingConnectionFactories != null);
                foreach (IncomingConnectionFactory factory in _incomingConnectionFactories)
                {
                    endpoints.Add(factory.Endpoint());
                }
                return endpoints.ToArray();
            }
        }

        /// <summary>
        /// Refresh the set of published endpoints.
        /// The run time re-reads
        /// the PublishedEndpoints property if it is set and re-reads the
        /// list of local interfaces if the adapter is configured to listen
        /// on all endpoints. This operation is useful to refresh the endpoint
        /// information that is published in the proxies that are created by
        /// an object adapter if the network interfaces used by a host changes.
        /// </summary>
        public void RefreshPublishedEndpoints()
        {
            LocatorInfo? locatorInfo = null;
            Endpoint[] oldPublishedEndpoints;

            lock (this)
            {
                checkForDeactivation();

                oldPublishedEndpoints = _publishedEndpoints;
                _publishedEndpoints = ComputePublishedEndpoints();

                locatorInfo = _locatorInfo;
            }

            try
            {
                UpdateLocatorRegistry(locatorInfo, CreateDirectProxy(new Identity("dummy", ""), IObjectPrx.Factory));
            }
            catch (LocalException)
            {
                lock (this)
                {
                    //
                    // Restore the old published endpoints.
                    //
                    _publishedEndpoints = oldPublishedEndpoints;
                    throw;
                }
            }
        }

        /// <summary>
        /// Get the set of endpoints that proxies created by this object
        /// adapter will contain.
        /// </summary>
        /// <returns>The set of published endpoints.
        ///
        /// </returns>
        public IEndpoint[] GetPublishedEndpoints()
        {
            lock (this)
            {
                return (IEndpoint[])_publishedEndpoints.Clone();
            }
        }

        /// <summary>
        /// Set of the endpoints that proxies created by this object
        /// adapter will contain.
        /// </summary>
        /// <param name="newEndpoints">The new set of endpoints that the object adapter will embed in proxies.
        ///
        /// </param>
        public void SetPublishedEndpoints(IEndpoint[] newEndpoints)
        {
            LocatorInfo? locatorInfo = null;
            Endpoint[] oldPublishedEndpoints;

            lock (this)
            {
                checkForDeactivation();
                if (_routerInfo != null)
                {
                    throw new ArgumentException(
                                    "can't set published endpoints on object adapter associated with a router");
                }

                oldPublishedEndpoints = _publishedEndpoints;
                _publishedEndpoints = Array.ConvertAll(newEndpoints, endpt => (Endpoint)endpt);
                locatorInfo = _locatorInfo;
            }

            try
            {
                UpdateLocatorRegistry(locatorInfo, CreateDirectProxy(new Identity("dummy", ""), IObjectPrx.Factory));
            }
            catch (LocalException)
            {
                lock (this)
                {
                    //
                    // Restore the old published endpoints.
                    //
                    _publishedEndpoints = oldPublishedEndpoints;
                    throw;
                }
            }
        }

        public bool isLocal(IObjectPrx proxy)
        {
            //
            // NOTE: it's important that isLocal() doesn't perform any blocking operations as
            // it can be called for AMI invocations if the proxy has no delegate set yet.
            //

            Reference r = proxy.IceReference;
            if (r.IsWellKnown())
            {
                //
                // Check the active dispatcher map to see if the well-known
                // proxy is for a local object.
                //
                return _servantManager.HasServant(r.GetIdentity());
            }
            else if (r.IsIndirect())
            {
                //
                // Proxy is local if the reference adapter id matches this
                // adapter id or replica group id.
                //
                return r.GetAdapterId().Equals(_id) || r.GetAdapterId().Equals(_replicaGroupId);
            }
            else
            {
                Endpoint[] endpoints = r.GetEndpoints();

                lock (this)
                {
                    checkForDeactivation();

                    //
                    // Proxies which have at least one endpoint in common with the
                    // endpoints used by this object adapter's incoming connection
                    // factories are considered local.
                    //
                    for (int i = 0; i < endpoints.Length; ++i)
                    {
                        foreach (Endpoint endpoint in _publishedEndpoints)
                        {
                            if (endpoints[i].Equivalent(endpoint))
                            {
                                return true;
                            }
                        }

                        Debug.Assert(_incomingConnectionFactories != null);
                        foreach (IncomingConnectionFactory factory in _incomingConnectionFactories)
                        {
                            if (factory.IsLocal(endpoints[i]))
                            {
                                return true;
                            }
                        }
                    }
                    return false;
                }
            }
        }

        public void updateConnectionObservers()
        {
            List<IncomingConnectionFactory> f;
            lock (this)
            {
                f = new List<IncomingConnectionFactory>(_incomingConnectionFactories);
            }

            foreach (IncomingConnectionFactory p in f)
            {
                p.UpdateConnectionObservers();
            }
        }

        public void updateThreadObservers()
        {
            ThreadPool? threadPool = null;
            lock (this)
            {
                threadPool = _threadPool;
            }

            if (threadPool != null)
            {
                threadPool.UpdateObservers();
            }
        }

        public void incDirectCount()
        {
            lock (this)
            {
                checkForDeactivation();

                Debug.Assert(_directCount >= 0);
                ++_directCount;
            }
        }

        public void decDirectCount()
        {
            lock (this)
            {
                // Not check for deactivation here!

                Debug.Assert(_communicator != null); // Must not be called after destroy().

                Debug.Assert(_directCount > 0);
                if (--_directCount == 0)
                {
                    System.Threading.Monitor.PulseAll(this);
                }
            }
        }

        public ThreadPool getThreadPool()
        {
            // No mutex lock necessary, _threadPool and _instance are
            // immutable after creation until they are removed in
            // destroy().

            // Not check for deactivation here!

            Debug.Assert(_communicator != null); // Must not be called after destroy().

            if (_threadPool != null)
            {
                return _threadPool;
            }
            else
            {
                return _communicator.ServerThreadPool();
            }

        }

        internal ServantManager getServantManager()
        {
            //
            // No mutex lock necessary, _servantManager is immutable.
            //
            return _servantManager;
        }

        internal ACMConfig getACM()
        {
            // Not check for deactivation here!

            Debug.Assert(_communicator != null); // Must not be called after destroy().
            return _acm;
        }

        internal void setAdapterOnConnection(Ice.Connection connection)
        {
            lock (this)
            {
                checkForDeactivation();
                connection.SetAdapterAndServantManager(this, _servantManager);
            }
        }

        internal int messageSizeMax()
        {
            // No mutex lock, immutable.
            return _messageSizeMax;
        }

        //
        // Only for use by ObjectAdapterFactory
        //
        internal ObjectAdapter(Communicator communicator, string name, IRouterPrx? router, bool noConfig)
        {
            _communicator = communicator;
            _servantManager = new ServantManager(communicator, name);
            _name = name;
            _incomingConnectionFactories = new List<IncomingConnectionFactory>();
            _publishedEndpoints = Array.Empty<Endpoint>();
            _routerInfo = null;
            _directCount = 0;
            _noConfig = noConfig;

            if (_noConfig)
            {
                _id = "";
                _replicaGroupId = "";
                _reference = _communicator.CreateReference("dummy -t", "");
                _acm = _communicator.ServerACM;
                return;
            }

            List<string> unknownProps = new List<string>();
            bool noProps = FilterProperties(unknownProps);

            //
            // Warn about unknown object adapter properties.
            //
            if (unknownProps.Count != 0 && (_communicator.GetPropertyAsInt("Ice.Warn.UnknownProperties") ?? 1) > 0)
            {
                StringBuilder message = new StringBuilder("found unknown properties for object adapter `");
                message.Append(_name);
                message.Append("':");
                foreach (string s in unknownProps)
                {
                    message.Append("\n    ");
                    message.Append(s);
                }
                _communicator.Logger.Warning(message.ToString());
            }

            //
            // Make sure named adapter has configuration.
            //
            if (router == null && noProps)
            {
                //
                // These need to be set to prevent warnings/asserts in the destructor.
                //
                _state = StateDestroyed;
                _communicator = null;
                _incomingConnectionFactories = null;
                throw new InitializationException($"object adapter `{_name}' requires configuration");
            }

            _id = _communicator.GetProperty($"{_name}.AdapterId") ?? "";
            _replicaGroupId = _communicator.GetProperty($"{_name}.ReplicaGroupId") ?? "";

            //
            // Setup a reference to be used to get the default proxy options
            // when creating new proxies. By default, create twoway proxies.
            //
            string proxyOptions = _communicator.GetProperty($"{_name}.ProxyOptions") ?? "-t";
            _reference = _communicator.CreateReference($"dummy {proxyOptions}", "");

            _acm = new ACMConfig(communicator, communicator.Logger, $"{_name}.ACM", _communicator.ServerACM);
            {
                int defaultMessageSizeMax = communicator.MessageSizeMax / 1024;
                int num = communicator.GetPropertyAsInt($"{_name}.MessageSizeMax") ?? defaultMessageSizeMax;
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
                int threadPoolSize = communicator.GetPropertyAsInt($"{_name}.ThreadPool.Size") ?? 0;
                int threadPoolSizeMax = communicator.GetPropertyAsInt($"{_name}.ThreadPool.SizeMax") ?? 0;
                if (threadPoolSize > 0 || threadPoolSizeMax > 0)
                {
                    _threadPool = new ThreadPool(_communicator, _name + ".ThreadPool", 0);
                }

                router ??= communicator.GetPropertyAsProxy($"{_name}.Router", IRouterPrx.Factory);

                if (router != null)
                {
                    _routerInfo = _communicator.GetRouterInfo(router);

                    //
                    // Make sure this router is not already registered with another adapter.
                    //
                    if (_routerInfo.Adapter != null)
                    {
                        throw new ArgumentException(
                            $"Router `{router.Identity.ToString(_communicator.ToStringMode)}' already registered with an object adater",
                            nameof(router));
                    }

                    //
                    // Associate this object adapter with the router. This way,
                    // new outgoing connections to the router's client proxy will
                    // use this object adapter for callbacks.
                    //
                    _routerInfo.Adapter = this;

                    //
                    // Also modify all existing outgoing connections to the
                    // router's client proxy to use this object adapter for
                    // callbacks.
                    //
                    _communicator.OutgoingConnectionFactory().SetRouterInfo(_routerInfo);
                }
                else
                {
                    //
                    // Parse the endpoints, but don't store them in the adapter. The connection
                    // factory might change it, for example, to fill in the real port number.
                    //
                    List<Endpoint> endpoints = ParseEndpoints(communicator.GetProperty($"{_name}.Endpoints") ?? "", true);
                    foreach (Endpoint endp in endpoints)
                    {
                        Endpoint? publishedEndpoint;
                        foreach (Endpoint expanded in endp.ExpandHost(out publishedEndpoint))
                        {
                            IncomingConnectionFactory factory = new IncomingConnectionFactory(communicator,
                                                                                              expanded,
                                                                                              publishedEndpoint,
                                                                                              this);
                            _incomingConnectionFactories.Add(factory);
                        }
                    }
                    if (endpoints.Count == 0)
                    {
                        TraceLevels tl = _communicator.TraceLevels;
                        if (tl.network >= 2)
                        {
                            _communicator.Logger.Trace(tl.networkCat, "created adapter `" + _name +
                                                                        "' without endpoints");
                        }
                    }
                }

                //
                // Parse published endpoints.
                //
                _publishedEndpoints = ComputePublishedEndpoints();
                ILocatorPrx? locator = communicator.GetPropertyAsProxy($"{_name}.Locator", ILocatorPrx.Factory);
                if (locator != null)
                {
                    SetLocator(locator);
                }
                else
                {
                    SetLocator(_communicator.GetDefaultLocator());
                }
            }
            catch (LocalException)
            {
                Destroy();
                throw;
            }
        }

        private T newProxy<T>(Identity identity, ProxyFactory<T> factory, string facet) where T : class, IObjectPrx
        {
            if (_id.Length == 0)
            {
                return newDirectProxy(identity, factory, facet);
            }
            else if (_replicaGroupId.Length == 0)
            {
                return newIndirectProxy(identity, factory, facet, _id);
            }
            else
            {
                return newIndirectProxy(identity, factory, facet, _replicaGroupId);
            }
        }

        //
        // Create a reference and return a proxy for this reference.
        //
        private T newDirectProxy<T>(Identity identity, ProxyFactory<T> factory, string facet)
            where T : class, IObjectPrx
            => factory(_communicator!.CreateReference(identity, facet, _reference!, _publishedEndpoints));

        //
        // Create a reference with the adapter id and return a
        // proxy for the reference.
        //
        private T newIndirectProxy<T>(Identity identity, ProxyFactory<T> factory, string facet, string id)
            where T : class, IObjectPrx
            => factory(_communicator!.CreateReference(identity, facet, _reference!, id));

        private void checkForDeactivation()
        {
            if (_state >= StateDeactivating)
            {
                ObjectAdapterDeactivatedException ex = new ObjectAdapterDeactivatedException();
                ex.Name = GetName();
                throw ex;
            }
        }

        private static void checkIdentity(Identity ident)
        {
            if (ident.Name.Length == 0)
            {
                throw new ArgumentException("Identity name cannot be empty", nameof(ident));
            }
        }

        private List<Endpoint> ParseEndpoints(string endpts, bool oaEndpoints)
        {
            Debug.Assert(_communicator != null);
            int beg;
            int end = 0;

            string delim = " \t\n\r";

            List<Endpoint> endpoints = new List<Endpoint>();
            while (end < endpts.Length)
            {
                beg = IceUtilInternal.StringUtil.findFirstNotOf(endpts, delim, end);
                if (beg == -1)
                {
                    if (endpoints.Count != 0)
                    {
                        throw new FormatException("invalid empty object adapter endpoint");
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
                    throw new FormatException("invalid empty object adapter endpoint");
                }

                string s = endpts.Substring(beg, (end) - (beg));
                Endpoint? endp = _communicator.CreateEndpoint(s, oaEndpoints);
                if (endp == null)
                {
                    throw new FormatException($"invalid object adapter endpoint `{s}'");
                }
                endpoints.Add(endp);

                ++end;
            }

            return endpoints;
        }

        private Endpoint[] ComputePublishedEndpoints()
        {
            Debug.Assert(_communicator != null);
            List<Endpoint> endpoints;
            if (_routerInfo != null)
            {
                //
                // Get the router's server proxy endpoints and use them as the published endpoints.
                //
                endpoints = new List<Endpoint>();
                foreach (Endpoint endpt in _routerInfo.GetServerEndpoints())
                {
                    if (!endpoints.Contains(endpt))
                    {
                        endpoints.Add(endpt);
                    }
                }
            }
            else
            {
                //
                // Parse published endpoints. If set, these are used in proxies
                // instead of the connection factory endpoints.
                //
                endpoints = ParseEndpoints(_communicator.GetProperty($"{_name}.PublishedEndpoints") ?? "", false);
                if (endpoints.Count == 0)
                {
                    //
                    // If the PublishedEndpoints property isn't set, we compute the published enpdoints
                    // from the OA endpoints, expanding any endpoints that may be listening on INADDR_ANY
                    // to include actual addresses in the published endpoints.
                    //
                    Debug.Assert(_incomingConnectionFactories != null);
                    foreach (IncomingConnectionFactory factory in _incomingConnectionFactories)
                    {
                        foreach (Endpoint endpt in factory.Endpoint().ExpandIfWildcard())
                        {
                            //
                            // Check for duplicate endpoints, this might occur if an endpoint with a DNS name
                            // expands to multiple addresses. In this case, multiple incoming connection
                            // factories can point to the same published endpoint.
                            //
                            if (!endpoints.Contains(endpt))
                            {
                                endpoints.Add(endpt);
                            }
                        }
                    }
                }
            }

            if (_communicator.TraceLevels.network >= 1 && endpoints.Count > 0)
            {
                StringBuilder s = new StringBuilder("published endpoints for object adapter `");
                s.Append(_name);
                s.Append("':\n");
                bool first = true;
                foreach (Endpoint endpoint in endpoints)
                {
                    if (!first)
                    {
                        s.Append(":");
                    }
                    s.Append(endpoint.ToString());
                    first = false;
                }
                _communicator.Logger.Trace(_communicator.TraceLevels.networkCat, s.ToString());
            }

            return endpoints.ToArray();
        }

        private void UpdateLocatorRegistry(LocatorInfo? locatorInfo, IObjectPrx? proxy)
        {
            if (_id.Length == 0 || locatorInfo == null)
            {
                return; // Nothing to update.
            }

            //
            // Call on the locator registry outside the synchronization to
            // blocking other threads that need to lock this OA.
            //
            ILocatorRegistryPrx locatorRegistry = locatorInfo.GetLocatorRegistry();
            if (locatorRegistry == null)
            {
                return;
            }
            Debug.Assert(_communicator != null);

            try
            {
                if (_replicaGroupId.Length == 0)
                {
                    locatorRegistry.SetAdapterDirectProxy(_id, proxy);
                }
                else
                {
                    locatorRegistry.SetReplicatedAdapterDirectProxy(_id, _replicaGroupId, proxy);
                }
            }
            catch (AdapterNotFoundException)
            {
                if (_communicator.TraceLevels.location >= 1)
                {
                    StringBuilder s = new StringBuilder();
                    s.Append("couldn't update object adapter `" + _id + "' endpoints with the locator registry:\n");
                    s.Append("the object adapter is not known to the locator registry");
                    _communicator.Logger.Trace(_communicator.TraceLevels.locationCat, s.ToString());
                }

                NotRegisteredException ex1 = new NotRegisteredException();
                ex1.KindOfObject = "object adapter";
                ex1.Id = _id;
                throw ex1;
            }
            catch (InvalidReplicaGroupIdException)
            {
                if (_communicator.TraceLevels.location >= 1)
                {
                    StringBuilder s = new StringBuilder();
                    s.Append("couldn't update object adapter `" + _id + "' endpoints with the locator registry:\n");
                    s.Append("the replica group `" + _replicaGroupId + "' is not known to the locator registry");
                    _communicator.Logger.Trace(_communicator.TraceLevels.locationCat, s.ToString());
                }

                NotRegisteredException ex1 = new NotRegisteredException();
                ex1.KindOfObject = "replica group";
                ex1.Id = _replicaGroupId;
                throw ex1;
            }
            catch (AdapterAlreadyActiveException)
            {
                if (_communicator.TraceLevels.location >= 1)
                {
                    StringBuilder s = new StringBuilder();
                    s.Append("couldn't update object adapter `" + _id + "' endpoints with the locator registry:\n");
                    s.Append("the object adapter endpoints are already set");
                    _communicator.Logger.Trace(_communicator.TraceLevels.locationCat, s.ToString());
                }

                ObjectAdapterIdInUseException ex1 = new ObjectAdapterIdInUseException();
                ex1.Id = _id;
                throw;
            }
            catch (ObjectAdapterDeactivatedException)
            {
                // Expected if collocated call and OA is deactivated, ignore.
            }
            catch (CommunicatorDestroyedException)
            {
                // Ignore
            }
            catch (LocalException e)
            {
                if (_communicator.TraceLevels.location >= 1)
                {
                    StringBuilder s = new StringBuilder();
                    s.Append("couldn't update object adapter `" + _id + "' endpoints with the locator registry:\n");
                    s.Append(e.ToString());
                    _communicator.Logger.Trace(_communicator.TraceLevels.locationCat, s.ToString());
                }
                throw; // TODO: Shall we raise a special exception instead of a non obvious local exception?
            }

            if (_communicator.TraceLevels.location >= 1)
            {
                StringBuilder s = new StringBuilder();
                s.Append("updated object adapter `" + _id + "' endpoints with the locator registry\n");
                s.Append("endpoints = ");
                if (proxy != null)
                {
                    IEndpoint[] endpoints = proxy.Endpoints;
                    for (int i = 0; i < endpoints.Length; i++)
                    {
                        s.Append(endpoints[i].ToString());
                        if (i + 1 < endpoints.Length)
                        {
                            s.Append(":");
                        }
                    }
                }
                _communicator.Logger.Trace(_communicator.TraceLevels.locationCat, s.ToString());
            }
        }

        private static readonly string[] _suffixes =
        {
            "ACM",
            "ACM.Timeout",
            "ACM.Heartbeat",
            "ACM.Close",
            "AdapterId",
            "Endpoints",
            "Locator",
            "Locator.EncodingVersion",
            "Locator.EndpointSelection",
            "Locator.ConnectionCached",
            "Locator.PreferSecure",
            "Locator.CollocationOptimized",
            "Locator.Router",
            "MessageSizeMax",
            "PublishedEndpoints",
            "ReplicaGroupId",
            "Router",
            "Router.EncodingVersion",
            "Router.EndpointSelection",
            "Router.ConnectionCached",
            "Router.PreferSecure",
            "Router.CollocationOptimized",
            "Router.Locator",
            "Router.Locator.EndpointSelection",
            "Router.Locator.ConnectionCached",
            "Router.Locator.PreferSecure",
            "Router.Locator.CollocationOptimized",
            "Router.Locator.LocatorCacheTimeout",
            "Router.Locator.InvocationTimeout",
            "Router.LocatorCacheTimeout",
            "Router.InvocationTimeout",
            "ProxyOptions",
            "ThreadPool.Size",
            "ThreadPool.SizeMax",
            "ThreadPool.SizeWarn",
            "ThreadPool.StackSize",
            "ThreadPool.Serialize"
        };

        private bool FilterProperties(List<string> unknownProps)
        {
            Debug.Assert(_communicator != null);
            //
            // Do not create unknown properties list if Ice prefix, ie Ice, Glacier2, etc
            //
            bool addUnknown = true;
            string prefix = _name + ".";
            foreach (var propertyName in PropertyNames.clPropNames)
            {
                if (prefix.StartsWith(string.Format("{0}.", propertyName), StringComparison.Ordinal))
                {
                    addUnknown = false;
                    break;
                }
            }

            bool noProps = true;
            Dictionary<string, string> props = _communicator.GetProperties(forPrefix: prefix);
            foreach (string prop in props.Keys)
            {
                bool valid = false;
                for (int i = 0; i < _suffixes.Length; ++i)
                {
                    if (prop.Equals(prefix + _suffixes[i]))
                    {
                        noProps = false;
                        valid = true;
                        break;
                    }
                }

                if (!valid && addUnknown)
                {
                    unknownProps.Add(prop);
                }
            }
            return noProps;
        }

        private const int StateUninitialized = 0; // Just constructed.
        private const int StateHeld = 1;
        private const int StateActivating = 2;
        private const int StateActive = 3;
        private const int StateDeactivating = 4;
        private const int StateDeactivated = 5;
        private const int StateDestroying = 6;
        private const int StateDestroyed = 7;

        private int _state = StateUninitialized;
        private Communicator? _communicator;
        private ThreadPool? _threadPool;
        private ACMConfig _acm;
        private ServantManager _servantManager;
        private readonly string _name;
        private readonly string _id;
        private readonly string _replicaGroupId;
        private Reference? _reference;
        private List<IncomingConnectionFactory>? _incomingConnectionFactories;
        private RouterInfo? _routerInfo;
        private Endpoint[] _publishedEndpoints;
        private LocatorInfo? _locatorInfo;
        private int _directCount;  // The number of direct proxies dispatching on this object adapter.
        private bool _noConfig;
        private int _messageSizeMax;
    }
}
