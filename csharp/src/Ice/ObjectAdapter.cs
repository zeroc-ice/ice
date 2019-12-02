//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Text;
    using System.Threading.Tasks;
    using IceInternal;

    public delegate Task<OutputStream> Disp(Incoming inS, Current current);

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
        public Communicator GetCommunicator()
        {
            return _communicator;
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
                    foreach (IncomingConnectionFactory icf in _incomingConnectionFactories)
                    {
                        icf.activate();
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
                    Properties properties = _communicator.initializationData().properties;
                    printAdapterReady = properties.getPropertyAsInt("Ice.PrintAdapterReady") > 0;
                }
            }

            try
            {
                Identity dummy = new Identity();
                dummy.name = "dummy";
                UpdateLocatorRegistry(locatorInfo, CreateDirectProxy(dummy));
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
                Console.Out.WriteLine(_name + " ready");
            }

            lock (this)
            {
                Debug.Assert(_state == StateActivating);

                foreach (IncomingConnectionFactory icf in _incomingConnectionFactories)
                {
                    icf.activate();
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
                foreach (IncomingConnectionFactory factory in _incomingConnectionFactories)
                {
                    factory.hold();
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
                factory.waitUntilHolding();
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
                    _communicator.routerManager().erase(_routerInfo.getRouter());

                    //
                    // Clear this object adapter with the router.
                    //
                    _routerInfo.setAdapter(null);
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

            foreach (IncomingConnectionFactory factory in _incomingConnectionFactories)
            {
                factory.destroy();
            }

            _communicator.outgoingConnectionFactory().removeAdapter(this);

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
            IncomingConnectionFactory[] incomingConnectionFactories = null;
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
            _servantManager.destroy();

            //
            // Destroy the thread pool.
            //
            if (_threadPool != null)
            {
                _threadPool.destroy();
                _threadPool.joinWithAllThreads();
            }

            if (_objectAdapterFactory != null)
            {
                _objectAdapterFactory.removeObjectAdapter(this);
            }

            lock (this)
            {
                //
                // We're done, now we can throw away all incoming connection
                // factories.
                //
                _incomingConnectionFactories.Clear();

                //
                // Remove object references (some of them cyclic).
                //
                _communicator = null;
                _threadPool = null;
                _routerInfo = null;
                _publishedEndpoints = Array.Empty<EndpointI>();
                _locatorInfo = null;
                _reference = null;
                _objectAdapterFactory = null;

                _state = StateDestroyed;
                System.Threading.Monitor.PulseAll(this);
            }
        }

        /// <summary>
        /// Add a servant to this object adapter's Active Servant Map.
        /// Note
        /// that one servant can implement several Ice objects by registering
        /// the servant with multiple identities. Adding a servant with an
        /// identity that is in the map already throws AlreadyRegisteredException.
        ///
        /// </summary>
        /// <param name="disp">The dispatcher object to add.
        ///
        /// </param>
        /// <param name="id">The identity of the Ice object that is implemented by
        /// the servant.
        ///
        /// </param>
        /// <param name="facet">The facet. An empty facet means the default facet.
        ///
        /// </param>
        /// <returns>A proxy that matches the given identity and this object
        /// adapter.
        ///
        /// </returns>
        public IObjectPrx Add(Disp disp, string id, string facet = "")
        {
            return Add(disp, Util.stringToIdentity(id), facet);
        }

        /// <summary>
        /// Add a servant to this object adapter's Active Servant Map.
        /// Note
        /// that one servant can implement several Ice objects by registering
        /// the servant with multiple identities. Adding a servant with an
        /// identity that is in the map already throws AlreadyRegisteredException.
        ///
        /// </summary>
        /// <param name="disp">The dispatcher object to add.
        ///
        /// </param>
        /// <param name="id">The identity of the Ice object that is implemented by
        /// the servant.
        ///
        /// </param>
        /// <param name="facet">The facet. An empty facet means the default facet.
        ///
        /// </param>
        /// <returns>A proxy that matches the given identity and this object
        /// adapter.
        ///
        /// </returns>
        public IObjectPrx Add(Disp disp, Identity? ident = null, string facet = "")
        {
            lock (this)
            {
                ident = ident ?? new Identity(Guid.NewGuid().ToString(), "");
                checkForDeactivation();
                checkIdentity(ident);
                CheckServant(disp);

                //
                // Create a copy of the Identity argument, in case the caller
                // reuses it.
                //
                // TODO remove this copy once we make Identity a value type
                //
                Identity id = new Identity();
                id.category = ident.category;
                id.name = ident.name;

                _servantManager.addServant(disp, id, facet);

                return newProxy(id, facet);
            }
        }

        /// <summary>
        /// Add a default servant to handle requests for a specific
        /// category.
        /// Adding a default servant for a category for
        /// which a default servant is already registered throws
        /// AlreadyRegisteredException. To dispatch operation
        /// calls on servants, the object adapter tries to find a servant
        /// for a given Ice object identity and facet in the following
        /// order:
        ///
        ///
        ///
        /// The object adapter tries to find a servant for the identity
        /// and facet in the Active Servant Map.
        ///
        /// If no servant has been found in the Active Servant Map, the
        /// object adapter tries to find a default servant for the category
        /// component of the identity.
        ///
        /// If no servant has been found by any of the preceding steps,
        /// the object adapter tries to find a default servant for an empty
        /// category, regardless of the category contained in the identity.
        ///
        /// If no servant has been found by any of the preceding steps,
        /// the object adapter gives up and the caller receives
        /// ObjectNotExistException or FacetNotExistException.
        ///
        ///
        ///
        /// </summary>
        /// <param name="servant">The default servant.
        ///
        /// </param>
        /// <param name="category">The category for which the default servant is
        /// registered. An empty category means it will handle all categories.
        ///
        /// </param>
        public void AddDefaultServant(Disp servant, string category)
        {
            CheckServant(servant);

            lock (this)
            {
                checkForDeactivation();

                _servantManager.addDefaultServant(servant, category);
            }
        }

        /// <summary>
        /// Remove a servant (that is, the default facet) from the object
        /// adapter's Active Servant Map.
        /// </summary>
        /// <param name="id">The identity of the Ice object that is implemented by
        /// the servant. If the servant implements multiple Ice objects,
        /// remove has to be called for all those Ice objects.
        /// Removing an identity that is not in the map throws
        /// NotRegisteredException.</param>
        /// <returns>The removed servant.</returns>
        public Disp Remove(string id, string facet = "")
        {
            return Remove(Util.stringToIdentity(id), facet);
        }

        public Disp Remove(Identity ident, string facet = "")
        {
            lock (this)
            {
                checkForDeactivation();
                checkIdentity(ident);

                return _servantManager.removeServant(ident, facet);
            }
        }

        /// <summary>
        /// Remove all facets with the given identity from the Active
        /// Servant Map.
        /// The operation completely removes the Ice object,
        /// including its default facet. Removing an identity that
        /// is not in the map throws NotRegisteredException.
        ///
        /// </summary>
        /// <param name="id">The identity of the Ice object to be removed.
        ///
        /// </param>
        /// <returns>A collection containing all the facet names and
        /// servants of the removed Ice object.
        ///
        /// </returns>
        public Dictionary<string, Disp> RemoveAllFacets(string ident)
        {
            return RemoveAllFacets(Util.stringToIdentity(ident));
        }

        public Dictionary<string, Disp> RemoveAllFacets(Identity ident)
        {
            lock (this)
            {
                checkForDeactivation();
                checkIdentity(ident);

                return _servantManager.removeAllFacets(ident);
            }
        }

        /// <summary>
        /// Remove the default servant for a specific category.
        /// Attempting
        /// to remove a default servant for a category that is not
        /// registered throws NotRegisteredException.
        ///
        /// </summary>
        /// <param name="category">The category of the default servant to remove.
        ///
        /// </param>
        /// <returns>The default servant.
        ///
        /// </returns>
        public Disp RemoveDefaultServant(string category)
        {
            lock (this)
            {
                checkForDeactivation();

                return _servantManager.removeDefaultServant(category);
            }
        }

        /// <summary>
        /// Look up a servant in this object adapter's Active Servant Map
        /// by the identity of the Ice object it implements.
        /// This operation only tries to look up a servant in
        /// the Active Servant Map. It does not attempt to find a servant
        /// by using any installed ServantLocator.
        ///
        /// </summary>
        /// <param name="id">The identity of the Ice object for which the servant
        /// should be returned.
        ///
        /// </param>
        /// <returns>The servant that implements the Ice object with the
        /// given identity, or null if no such servant has been found.
        ///
        /// </returns>
        public Disp Find(Identity ident, string facet = "")
        {
            lock (this)
            {
                checkForDeactivation();
                checkIdentity(ident);

                return _servantManager.findServant(ident, facet);
            }
        }

        /// <summary>
        /// Find all facets with the given identity in the Active Servant
        /// Map.
        /// </summary>
        /// <param name="id">The identity of the Ice object for which the facets
        /// should be returned.
        ///
        /// </param>
        /// <returns>A collection containing all the facet names and
        /// servants that have been found, or an empty map if there is no
        /// facet for the given identity.
        ///
        /// </returns>
        public Dictionary<string, Disp> FindAllFacets(string ident)
        {
            return FindAllFacets(Util.stringToIdentity(ident));
        }

        /// <summary>
        /// Find all facets with the given identity in the Active Servant
        /// Map.
        /// </summary>
        /// <param name="id">The identity of the Ice object for which the facets
        /// should be returned.
        ///
        /// </param>
        /// <returns>A collection containing all the facet names and
        /// servants that have been found, or an empty map if there is no
        /// facet for the given identity.
        ///
        /// </returns>
        public Dictionary<string, Disp> FindAllFacets(Identity ident)
        {
            lock (this)
            {
                checkForDeactivation();
                checkIdentity(ident);

                return _servantManager.findAllFacets(ident);
            }
        }

        /// <summary>
        /// Find the default servant for a specific category.
        /// </summary>
        /// <param name="category">The category of the default servant to find.
        ///
        /// </param>
        /// <returns>The default servant or null if no default servant was
        /// registered for the category.
        ///
        /// </returns>
        public Disp? FindDefaultServant(string category)
        {
            lock (this)
            {
                checkForDeactivation();

                return _servantManager.findDefaultServant(category);
            }
        }

        /// <summary>
        /// Add a Servant Locator to this object adapter.
        /// Adding a servant
        /// locator for a category for which a servant locator is already
        /// registered throws AlreadyRegisteredException. To dispatch
        /// operation calls on servants, the object adapter tries to find a
        /// servant for a given Ice object identity and facet in the
        /// following order:
        ///
        ///
        ///
        /// The object adapter tries to find a servant for the identity
        /// and facet in the Active Servant Map.
        ///
        /// If no servant has been found in the Active Servant Map,
        /// the object adapter tries to find a servant locator for the
        /// category component of the identity. If a locator is found, the
        /// object adapter tries to find a servant using this locator.
        ///
        /// If no servant has been found by any of the preceding steps,
        /// the object adapter tries to find a locator for an empty category,
        /// regardless of the category contained in the identity. If a
        /// locator is found, the object adapter tries to find a servant
        /// using this locator.
        ///
        /// If no servant has been found by any of the preceding steps,
        /// the object adapter gives up and the caller receives
        /// ObjectNotExistException or FacetNotExistException.
        ///
        ///
        ///
        /// Only one locator for the empty category can be
        /// installed.
        ///
        /// </summary>
        /// <param name="locator">The locator to add.
        ///
        /// </param>
        /// <param name="category">The category for which the Servant Locator can
        /// locate servants, or an empty string if the Servant Locator does
        /// not belong to any specific category.
        ///
        /// </param>
        public void AddServantLocator(ServantLocator locator, string prefix)
        {
            lock (this)
            {
                checkForDeactivation();

                _servantManager.addServantLocator(locator, prefix);
            }
        }

        /// <summary>
        /// Remove a Servant Locator from this object adapter.
        /// </summary>
        /// <param name="category">The category for which the Servant Locator can
        /// locate servants, or an empty string if the Servant Locator does
        /// not belong to any specific category.
        ///
        /// </param>
        /// <returns>The Servant Locator, or throws NotRegisteredException
        /// if no Servant Locator was found for the given category.
        ///
        /// </returns>
        public ServantLocator RemoveServantLocator(string prefix)
        {
            lock (this)
            {
                checkForDeactivation();

                return _servantManager.removeServantLocator(prefix);
            }
        }

        /// <summary>
        /// Find a Servant Locator installed with this object adapter.
        /// </summary>
        /// <param name="category">The category for which the Servant Locator can
        /// locate servants, or an empty string if the Servant Locator does
        /// not belong to any specific category.
        ///
        /// </param>
        /// <returns>The Servant Locator, or null if no Servant Locator was
        /// found for the given category.
        ///
        /// </returns>
        public ServantLocator FindServantLocator(string category)
        {
            lock (this)
            {
                checkForDeactivation();

                return _servantManager.findServantLocator(category);
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
        /// <param name="id">The object's identity.
        ///
        /// </param>
        /// <returns>A proxy for the object with the given identity.
        ///
        /// </returns>
        public IObjectPrx CreateProxy(string ident)
        {
            return CreateProxy(Util.stringToIdentity(ident));
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
        /// <param name="id">The object's identity.
        ///
        /// </param>
        /// <returns>A proxy for the object with the given identity.
        ///
        /// </returns>
        public IObjectPrx CreateProxy(Identity ident)
        {
            lock (this)
            {
                checkForDeactivation();
                checkIdentity(ident);

                return newProxy(ident, "");
            }
        }

        /// <summary>
        /// Create a direct proxy for the object with the given identity.
        /// The returned proxy contains this object adapter's published
        /// endpoints.
        ///
        /// </summary>
        /// <param name="id">The object's identity.
        ///
        /// </param>
        /// <returns>A proxy for the object with the given identity.
        ///
        /// </returns>
        public IObjectPrx CreateDirectProxy(string ident)
        {
            return CreateDirectProxy(Util.stringToIdentity(ident));
        }

        /// <summary>
        /// Create a direct proxy for the object with the given identity.
        /// The returned proxy contains this object adapter's published
        /// endpoints.
        ///
        /// </summary>
        /// <param name="id">The object's identity.
        ///
        /// </param>
        /// <returns>A proxy for the object with the given identity.
        ///
        /// </returns>
        public IObjectPrx CreateDirectProxy(Identity ident)
        {
            lock (this)
            {
                checkForDeactivation();
                checkIdentity(ident);

                return newDirectProxy(ident, "");
            }
        }

        /// <summary>
        /// Create an indirect proxy for the object with the given identity.
        /// If this object adapter is configured with an adapter id, the
        /// return value refers to the adapter id. Otherwise, the return
        /// value contains only the object identity.
        ///
        /// </summary>
        /// <param name="id">The object's identity.
        ///
        /// </param>
        /// <returns>A proxy for the object with the given identity.
        ///
        /// </returns>
        public IObjectPrx CreateIndirectProxy(string ident)
        {
            return CreateIndirectProxy(Util.stringToIdentity(ident));
        }

        /// <summary>
        /// Create an indirect proxy for the object with the given identity.
        /// If this object adapter is configured with an adapter id, the
        /// return value refers to the adapter id. Otherwise, the return
        /// value contains only the object identity.
        ///
        /// </summary>
        /// <param name="id">The object's identity.
        ///
        /// </param>
        /// <returns>A proxy for the object with the given identity.
        ///
        /// </returns>
        public IObjectPrx CreateIndirectProxy(Identity ident)
        {
            lock (this)
            {
                checkForDeactivation();
                checkIdentity(ident);

                return newIndirectProxy(ident, "", _id);
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
        /// <param name="loc">The locator used by this object adapter.
        ///
        /// </param>
        public void SetLocator(LocatorPrx locator)
        {
            lock (this)
            {
                checkForDeactivation();

                if (locator != null)
                {
                    _locatorInfo = _communicator.locatorManager().get(locator);
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
        public LocatorPrx? GetLocator()
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
                    return _locatorInfo.getLocator();
                }
            }
        }

        /// <summary>
        /// Get the set of endpoints configured with this object adapter.
        /// </summary>
        /// <returns>The set of endpoints.
        ///
        /// </returns>
        public Endpoint[] GetEndpoints()
        {
            lock (this)
            {
                List<Endpoint> endpoints = new List<Endpoint>();
                foreach (IncomingConnectionFactory factory in _incomingConnectionFactories)
                {
                    endpoints.Add(factory.endpoint());
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
            LocatorInfo locatorInfo = null;
            EndpointI[] oldPublishedEndpoints;

            lock (this)
            {
                checkForDeactivation();

                oldPublishedEndpoints = _publishedEndpoints;
                _publishedEndpoints = ComputePublishedEndpoints();

                locatorInfo = _locatorInfo;
            }

            try
            {
                Identity dummy = new Identity();
                dummy.name = "dummy";
                UpdateLocatorRegistry(locatorInfo, CreateDirectProxy(dummy));
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
        public Endpoint[] GetPublishedEndpoints()
        {
            lock (this)
            {
                return (Endpoint[])_publishedEndpoints.Clone();
            }
        }

        /// <summary>
        /// Set of the endpoints that proxies created by this object
        /// adapter will contain.
        /// </summary>
        /// <param name="newEndpoints">The new set of endpoints that the object adapter will embed in proxies.
        ///
        /// </param>
        public void SetPublishedEndpoints(Endpoint[] newEndpoints)
        {
            LocatorInfo locatorInfo = null;
            EndpointI[] oldPublishedEndpoints;

            lock (this)
            {
                checkForDeactivation();
                if (_routerInfo != null)
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
                Identity dummy = new Identity();
                dummy.name = "dummy";
                UpdateLocatorRegistry(locatorInfo, CreateDirectProxy(dummy));
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
                return r.getAdapterId().Equals(_id) || r.getAdapterId().Equals(_replicaGroupId);
            }
            else
            {
                EndpointI[] endpoints = r.getEndpoints();

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
                        foreach (EndpointI endpoint in _publishedEndpoints)
                        {
                            if (endpoints[i].equivalent(endpoint))
                            {
                                return true;
                            }
                        }
                        foreach (IncomingConnectionFactory factory in _incomingConnectionFactories)
                        {
                            if (factory.isLocal(endpoints[i]))
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
                p.updateConnectionObservers();
            }
        }

        public void updateThreadObservers()
        {
            ThreadPool threadPool = null;
            lock (this)
            {
                threadPool = _threadPool;
            }

            if (threadPool != null)
            {
                threadPool.updateObservers();
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
                return _communicator.serverThreadPool();
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

        internal void setAdapterOnConnection(Ice.ConnectionI connection)
        {
            lock (this)
            {
                checkForDeactivation();
                connection.setAdapterAndServantManager(this, _servantManager);
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
        internal ObjectAdapter(Communicator communicator,
                               ObjectAdapterFactory objectAdapterFactory, string name,
                               RouterPrx router, bool noConfig)
        {
            _communicator = communicator;
            _objectAdapterFactory = objectAdapterFactory;
            _servantManager = new ServantManager(communicator, name);
            _name = name;
            _incomingConnectionFactories = new List<IncomingConnectionFactory>();
            _publishedEndpoints = Array.Empty<EndpointI>();
            _routerInfo = null;
            _directCount = 0;
            _noConfig = noConfig;

            if (_noConfig)
            {
                _id = "";
                _replicaGroupId = "";
                _reference = _communicator.CreateReference("dummy -t", "");
                _acm = _communicator.serverACM();
                return;
            }

            Properties properties = _communicator.initializationData().properties;
            List<string> unknownProps = new List<string>();
            bool noProps = FilterProperties(unknownProps);

            //
            // Warn about unknown object adapter properties.
            //
            if (unknownProps.Count != 0 && properties.getPropertyAsIntWithDefault("Ice.Warn.UnknownProperties", 1) > 0)
            {
                StringBuilder message = new StringBuilder("found unknown properties for object adapter `");
                message.Append(_name);
                message.Append("':");
                foreach (string s in unknownProps)
                {
                    message.Append("\n    ");
                    message.Append(s);
                }
                _communicator.initializationData().logger.warning(message.ToString());
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

                InitializationException ex = new InitializationException();
                ex.reason = "object adapter `" + _name + "' requires configuration";
                throw ex;
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
                _reference = _communicator.CreateReference($"dummy {proxyOptions}", "");
            }
            catch (ProxyParseException)
            {
                InitializationException ex = new InitializationException();
                ex.reason = "invalid proxy options `" + proxyOptions + "' for object adapter `" + _name + "'";
                throw ex;
            }

            _acm = new ACMConfig(properties, communicator.getLogger(), _name + ".ACM", _communicator.serverACM());

            {
                int defaultMessageSizeMax = communicator.messageSizeMax() / 1024;
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
                    _threadPool = new ThreadPool(_communicator, _name + ".ThreadPool", 0);
                }

                string property = _name + ".Router";
                if (router == null && !string.IsNullOrEmpty(properties.getProperty(property)))
                {
                    router = RouterPrx.ParseProperty(property, communicator);
                }

                if (router != null)
                {
                    _routerInfo = _communicator.routerManager().get(router);
                    Debug.Assert(_routerInfo != null);

                    //
                    // Make sure this router is not already registered with another adapter.
                    //
                    if (_routerInfo.getAdapter() != null)
                    {
                        AlreadyRegisteredException ex = new AlreadyRegisteredException();
                        ex.kindOfObject = "object adapter with router";
                        ex.id = Util.identityToString(router.Identity, _communicator.toStringMode());
                        throw ex;
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
                    _communicator.outgoingConnectionFactory().setRouterInfo(_routerInfo);
                }
                else
                {
                    //
                    // Parse the endpoints, but don't store them in the adapter. The connection
                    // factory might change it, for example, to fill in the real port number.
                    //
                    List<EndpointI> endpoints = ParseEndpoints(properties.getProperty(_name + ".Endpoints"), true);
                    foreach (EndpointI endp in endpoints)
                    {
                        EndpointI publishedEndpoint;
                        foreach (IceInternal.EndpointI expanded in endp.expandHost(out publishedEndpoint))
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
                        TraceLevels tl = _communicator.traceLevels();
                        if (tl.network >= 2)
                        {
                            _communicator.initializationData().logger.trace(tl.networkCat, "created adapter `" + _name +
                                                                        "' without endpoints");
                        }
                    }
                }

                //
                // Parse published endpoints.
                //
                _publishedEndpoints = ComputePublishedEndpoints();
                property = _name + ".Locator";
                if (!string.IsNullOrEmpty(properties.getProperty(property)))
                {
                    SetLocator(LocatorPrx.ParseProperty(property, communicator));
                }
                else
                {
                    SetLocator(_communicator.getDefaultLocator());
                }
            }
            catch (LocalException)
            {
                Destroy();
                throw;
            }
        }

        private IObjectPrx newProxy(Identity ident, string facet)
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

        private IObjectPrx newDirectProxy(Identity ident, string facet)
        {
            //
            // Create a reference and return a proxy for this reference.
            //
            return new ObjectPrx(_communicator.CreateReference(ident, facet, _reference, _publishedEndpoints));
        }

        private IObjectPrx newIndirectProxy(Identity ident, string facet, string id)
        {
            //
            // Create a reference with the adapter id and return a
            // proxy for the reference.
            //
            return new ObjectPrx(_communicator.CreateReference(ident, facet, _reference, id));
        }

        private void checkForDeactivation()
        {
            if (_state >= StateDeactivating)
            {
                ObjectAdapterDeactivatedException ex = new ObjectAdapterDeactivatedException();
                ex.name = GetName();
                throw ex;
            }
        }

        private static void checkIdentity(Identity ident)
        {
            if (ident.name == null || ident.name.Length == 0)
            {
                throw new IllegalIdentityException(ident);
            }
            if (ident.category == null)
            {
                ident.category = "";
            }
        }

        private static void CheckServant(Disp servant)
        {
            if (servant == null)
            {
                throw new IllegalServantException("cannot add null servant to Object Adapter");
            }
        }

        private List<EndpointI> ParseEndpoints(string endpts, bool oaEndpoints)
        {
            int beg;
            int end = 0;

            string delim = " \t\n\r";

            List<EndpointI> endpoints = new List<EndpointI>();
            while (end < endpts.Length)
            {
                beg = IceUtilInternal.StringUtil.findFirstNotOf(endpts, delim, end);
                if (beg == -1)
                {
                    if (endpoints.Count != 0)
                    {
                        throw new EndpointParseException("invalid empty object adapter endpoint");
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
                    throw new EndpointParseException("invalid empty object adapter endpoint");
                }

                string s = endpts.Substring(beg, (end) - (beg));
                EndpointI endp = _communicator.endpointFactoryManager().create(s, oaEndpoints);
                if (endp == null)
                {
                    throw new EndpointParseException("invalid object adapter endpoint `" + s + "'");
                }
                endpoints.Add(endp);

                ++end;
            }

            return endpoints;
        }

        private EndpointI[] ComputePublishedEndpoints()
        {
            List<EndpointI> endpoints;
            if (_routerInfo != null)
            {
                //
                // Get the router's server proxy endpoints and use them as the published endpoints.
                //
                endpoints = new List<EndpointI>();
                foreach (EndpointI endpt in _routerInfo.getServerEndpoints())
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
                string endpts = _communicator.initializationData().properties.getProperty(_name + ".PublishedEndpoints");
                endpoints = ParseEndpoints(endpts, false);
                if (endpoints.Count == 0)
                {
                    //
                    // If the PublishedEndpoints property isn't set, we compute the published enpdoints
                    // from the OA endpoints, expanding any endpoints that may be listening on INADDR_ANY
                    // to include actual addresses in the published endpoints.
                    //
                    foreach (IncomingConnectionFactory factory in _incomingConnectionFactories)
                    {
                        foreach (EndpointI endpt in factory.endpoint().expandIfWildcard())
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

            if (_communicator.traceLevels().network >= 1 && endpoints.Count > 0)
            {
                StringBuilder s = new StringBuilder("published endpoints for object adapter `");
                s.Append(_name);
                s.Append("':\n");
                bool first = true;
                foreach (EndpointI endpoint in endpoints)
                {
                    if (!first)
                    {
                        s.Append(":");
                    }
                    s.Append(endpoint.ToString());
                    first = false;
                }
                _communicator.initializationData().logger.trace(_communicator.traceLevels().networkCat, s.ToString());
            }

            return endpoints.ToArray();
        }

        private void UpdateLocatorRegistry(LocatorInfo locatorInfo, IObjectPrx proxy)
        {
            if (_id.Length == 0 || locatorInfo == null)
            {
                return; // Nothing to update.
            }

            //
            // Call on the locator registry outside the synchronization to
            // blocking other threads that need to lock this OA.
            //
            LocatorRegistryPrx locatorRegistry = locatorInfo.getLocatorRegistry();
            if (locatorRegistry == null)
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
                if (_communicator.traceLevels().location >= 1)
                {
                    StringBuilder s = new StringBuilder();
                    s.Append("couldn't update object adapter `" + _id + "' endpoints with the locator registry:\n");
                    s.Append("the object adapter is not known to the locator registry");
                    _communicator.initializationData().logger.trace(_communicator.traceLevels().locationCat, s.ToString());
                }

                NotRegisteredException ex1 = new NotRegisteredException();
                ex1.kindOfObject = "object adapter";
                ex1.id = _id;
                throw ex1;
            }
            catch (InvalidReplicaGroupIdException)
            {
                if (_communicator.traceLevels().location >= 1)
                {
                    StringBuilder s = new StringBuilder();
                    s.Append("couldn't update object adapter `" + _id + "' endpoints with the locator registry:\n");
                    s.Append("the replica group `" + _replicaGroupId + "' is not known to the locator registry");
                    _communicator.initializationData().logger.trace(_communicator.traceLevels().locationCat, s.ToString());
                }

                NotRegisteredException ex1 = new NotRegisteredException();
                ex1.kindOfObject = "replica group";
                ex1.id = _replicaGroupId;
                throw ex1;
            }
            catch (AdapterAlreadyActiveException)
            {
                if (_communicator.traceLevels().location >= 1)
                {
                    StringBuilder s = new StringBuilder();
                    s.Append("couldn't update object adapter `" + _id + "' endpoints with the locator registry:\n");
                    s.Append("the object adapter endpoints are already set");
                    _communicator.initializationData().logger.trace(_communicator.traceLevels().locationCat, s.ToString());
                }

                ObjectAdapterIdInUseException ex1 = new ObjectAdapterIdInUseException();
                ex1.id = _id;
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
                if (_communicator.traceLevels().location >= 1)
                {
                    StringBuilder s = new StringBuilder();
                    s.Append("couldn't update object adapter `" + _id + "' endpoints with the locator registry:\n");
                    s.Append(e.ToString());
                    _communicator.initializationData().logger.trace(_communicator.traceLevels().locationCat, s.ToString());
                }
                throw; // TODO: Shall we raise a special exception instead of a non obvious local exception?
            }

            if (_communicator.traceLevels().location >= 1)
            {
                StringBuilder s = new StringBuilder();
                s.Append("updated object adapter `" + _id + "' endpoints with the locator registry\n");
                s.Append("endpoints = ");
                if (proxy != null)
                {
                    Endpoint[] endpoints = proxy.Endpoints;
                    for (int i = 0; i < endpoints.Length; i++)
                    {
                        s.Append(endpoints[i].ToString());
                        if (i + 1 < endpoints.Length)
                        {
                            s.Append(":");
                        }
                    }
                }
                _communicator.initializationData().logger.trace(_communicator.traceLevels().locationCat, s.ToString());
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
            Dictionary<string, string> props =
                _communicator.initializationData().properties.getPropertiesForPrefix(prefix);
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
        private ObjectAdapterFactory _objectAdapterFactory;
        private ThreadPool _threadPool;
        private ACMConfig _acm;
        private ServantManager _servantManager;
        private readonly string _name;
        private readonly string _id;
        private readonly string _replicaGroupId;
        private Reference _reference;
        private List<IncomingConnectionFactory> _incomingConnectionFactories;
        private RouterInfo _routerInfo;
        private EndpointI[] _publishedEndpoints;
        private LocatorInfo? _locatorInfo;
        private int _directCount;  // The number of direct proxies dispatching on this object adapter.
        private bool _noConfig;
        private int _messageSizeMax;
    }
}
