//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Diagnostics;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Text.RegularExpressions;
using System.Threading;
using System.Text;
using System.Linq;
using System.Globalization;
using IceInternal;

namespace Ice
{
    public enum ToStringMode
    {
        Unicode,
        ASCII,
        Compat
    }

    internal sealed class BufSizeWarnInfo
    {
        // Whether send size warning has been emitted
        public bool sndWarn;

        // The send size for which the warning wwas emitted
        public int sndSize;

        // Whether receive size warning has been emitted
        public bool rcvWarn;

        // The receive size for which the warning wwas emitted
        public int rcvSize;
    }

    public sealed partial class Communicator : IDisposable
    {
        private class ObserverUpdaterI : Ice.Instrumentation.ObserverUpdater
        {
            public ObserverUpdaterI(Communicator communicator)
            {
                _communicator = communicator;
            }

            public void updateConnectionObservers()
            {
                _communicator.updateConnectionObservers();
            }

            public void updateThreadObservers()
            {
                _communicator.updateThreadObservers();
            }

            private readonly Communicator _communicator;
        }

        internal TraceLevels traceLevels()
        {
            // No mutex lock, immutable.
            Debug.Assert(_traceLevels != null);
            return _traceLevels;
        }

        internal DefaultsAndOverrides defaultsAndOverrides()
        {
            // No mutex lock, immutable.
            Debug.Assert(_defaultsAndOverrides != null);
            return _defaultsAndOverrides;
        }

        /// <summary>
        /// Destroy the communicator.
        /// This operation calls shutdown
        /// implicitly.  Calling destroy cleans up memory, and shuts down
        /// this communicator's client functionality and destroys all object
        /// adapters. Subsequent calls to destroy are ignored.
        ///
        /// </summary>
        public void destroy()
        {
            lock (this)
            {
                //
                // If destroy is in progress, wait for it to be done. This
                // is necessary in case destroy() is called concurrently
                // by multiple threads.
                //
                while (_state == StateDestroyInProgress)
                {
                    Monitor.Wait(this);
                }

                if (_state == StateDestroyed)
                {
                    return;
                }
                _state = StateDestroyInProgress;
            }

            //
            // Shutdown and destroy all the incoming and outgoing Ice
            // connections and wait for the connections to be finished.
            //
            if (_objectAdapterFactory != null)
            {
                _objectAdapterFactory.shutdown();
            }

            if (_outgoingConnectionFactory != null)
            {
                _outgoingConnectionFactory.destroy();
            }

            if (_objectAdapterFactory != null)
            {
                _objectAdapterFactory.destroy();
            }

            if (_outgoingConnectionFactory != null)
            {
                _outgoingConnectionFactory.waitUntilFinished();
            }

            if (_retryQueue != null)
            {
                _retryQueue.destroy(); // Must be called before destroying thread pools.
            }

            if (_observer != null)
            {
                _observer.setObserverUpdater(null);
            }

            {
                if (_logger is LoggerAdminLogger)
                {
                    ((LoggerAdminLogger)_logger).destroy();
                }
            }

            //
            // Now, destroy the thread pools. This must be done *only* after
            // all the connections are finished (the connections destruction
            // can require invoking callbacks with the thread pools).
            //
            if (_serverThreadPool != null)
            {
                _serverThreadPool.destroy();
            }
            if (_clientThreadPool != null)
            {
                _clientThreadPool.destroy();
            }
            if (_asyncIOThread != null)
            {
                _asyncIOThread.destroy();
            }
            if (_endpointHostResolver != null)
            {
                _endpointHostResolver.destroy();
            }

            //
            // Wait for all the threads to be finished.
            //
            if (_timer != null)
            {
                _timer.destroy();
            }
            if (_clientThreadPool != null)
            {
                _clientThreadPool.joinWithAllThreads();
            }
            if (_serverThreadPool != null)
            {
                _serverThreadPool.joinWithAllThreads();
            }
            if (_asyncIOThread != null)
            {
                _asyncIOThread.joinWithThread();
            }
            if (_endpointHostResolver != null)
            {
                _endpointHostResolver.joinWithThread();
            }

            if (_routerManager != null)
            {
                _routerManager.destroy();
            }

            if (_locatorManager != null)
            {
                _locatorManager.destroy();
            }

            if (_endpointFactoryManager != null)
            {
                _endpointFactoryManager.destroy();
            }

            if (GetPropertyAsInt("Ice.Warn.UnusedProperties") > 0)
            {
                List<string> unusedProperties = GetUnusedProperties();
                if (unusedProperties.Count != 0)
                {
                    StringBuilder message = new StringBuilder("The following properties were set but never read:");
                    foreach (string s in unusedProperties)
                    {
                        message.Append("\n    ");
                        message.Append(s);
                    }
                    _logger.warning(message.ToString());
                }
            }

            //
            // Destroy last so that a Logger plugin can receive all log/traces before its destruction.
            //
            if (_pluginManager != null)
            {
                _pluginManager.destroy();
            }

            lock (this)
            {
                _objectAdapterFactory = null;
                _outgoingConnectionFactory = null;
                _retryQueue = null;

                _serverThreadPool = null;
                _clientThreadPool = null;
                _asyncIOThread = null;
                _endpointHostResolver = null;
                _timer = null;

                _requestHandlerFactory = null;
                _routerManager = null;
                _locatorManager = null;
                _endpointFactoryManager = null;
                _pluginManager = null;

                _adminAdapter = null;
                _adminFacets.Clear();

                _state = StateDestroyed;
                Monitor.PulseAll(this);
            }

            {
                if (_logger != null && _logger is FileLoggerI)
                {
                    ((FileLoggerI)_logger).destroy();
                }
            }
        }

        /// <summary>
        /// Shuts down this communicator's server functionality, which
        /// includes the deactivation of all object adapters.
        /// Attempts to use a
        /// deactivated object adapter raise ObjectAdapterDeactivatedException.
        /// Subsequent calls to shutdown are ignored.
        ///
        /// After shutdown returns, no new requests are processed. However, requests
        /// that have been started before shutdown was called might still be active.
        /// You can use waitForShutdown to wait for the completion of all
        /// requests.
        ///
        /// </summary>
        public void shutdown()
        {
            try
            {
                objectAdapterFactory().shutdown();
            }
            catch (CommunicatorDestroyedException)
            {
                // Ignore
            }
        }

        /// <summary>
        /// Wait until the application has called shutdown (or destroy).
        /// On the server side, this operation blocks the calling thread
        /// until all currently-executing operations have completed.
        /// On the client side, the operation simply blocks until another
        /// thread has called shutdown or destroy.
        ///
        /// A typical use of this operation is to call it from the main thread,
        /// which then waits until some other thread calls shutdown.
        /// After shut-down is complete, the main thread returns and can do some
        /// cleanup work before it finally calls destroy to shut down
        /// the client functionality, and then exits the application.
        ///
        /// </summary>
        public void waitForShutdown()
        {
            try
            {
                objectAdapterFactory().waitForShutdown();
            }
            catch (CommunicatorDestroyedException)
            {
                // Ignore
            }
        }

        /// <summary>
        /// Check whether communicator has been shut down.
        /// </summary>
        /// <returns>True if the communicator has been shut down; false otherwise.
        ///
        /// </returns>
        public bool isShutdown()
        {
            try
            {
                return objectAdapterFactory().isShutdown();
            }
            catch (CommunicatorDestroyedException)
            {
                return true;
            }
        }

        /// <summary>
        /// Create a new object adapter.
        /// The endpoints for the object
        /// adapter are taken from the property name.Endpoints.
        ///
        /// It is legal to create an object adapter with the empty string as
        /// its name. Such an object adapter is accessible via bidirectional
        /// connections or by collocated invocations that originate from the
        /// same communicator as is used by the adapter.
        ///
        /// Attempts to create a named object adapter for which no configuration
        /// can be found raise InitializationException.
        ///
        /// </summary>
        /// <param name="name">The object adapter name.
        ///
        /// </param>
        /// <returns>The new object adapter.
        ///
        /// </returns>
        public ObjectAdapter createObjectAdapter(string name)
        {
            return objectAdapterFactory().createObjectAdapter(name, null);
        }

        /// <summary>
        /// Create a new object adapter with endpoints.
        /// This operation sets
        /// the property name.Endpoints, and then calls
        /// createObjectAdapter. It is provided as a convenience
        /// function.
        ///
        /// Calling this operation with an empty name will result in a
        /// UUID being generated for the name.
        ///
        /// </summary>
        /// <param name="name">The object adapter name.
        ///
        /// </param>
        /// <param name="endpoints">The endpoints for the object adapter.
        ///
        /// </param>
        /// <returns>The new object adapter.
        ///
        /// </returns>
        public ObjectAdapter createObjectAdapterWithEndpoints(string name, string endpoints)
        {
            if (name.Length == 0)
            {
                name = Guid.NewGuid().ToString();
            }

            SetProperty($"{name}.Endpoints", endpoints);
            return objectAdapterFactory().createObjectAdapter(name, null);
        }

        /// <summary>
        /// Create a new object adapter with a router.
        /// This operation
        /// creates a routed object adapter.
        ///
        /// Calling this operation with an empty name will result in a
        /// UUID being generated for the name.
        ///
        /// </summary>
        /// <param name="name">The object adapter name.
        ///
        /// </param>
        /// <param name="router">The router.
        ///
        /// </param>
        /// <returns>The new object adapter.
        ///
        /// </returns>
        public ObjectAdapter createObjectAdapterWithRouter(string name, RouterPrx router)
        {
            if (name.Length == 0)
            {
                name = Guid.NewGuid().ToString();
            }

            //
            // We set the proxy properties here, although we still use the proxy supplied.
            //
            Dictionary<string, string> properties = router.ToProperty($"{name}.Router");
            foreach (KeyValuePair<string, string> entry in properties)
            {
                SetProperty(entry.Key, entry.Value);
            }

            return objectAdapterFactory().createObjectAdapter(name, router);
        }

        /// <summary>
        /// Get the logger for this communicator.
        /// </summary>
        /// <returns>This communicator's logger.
        ///
        /// </returns>
        public Logger Logger
        {
            get
            {
                return _logger;
            }
        }

        public Action<Action, Connection?>? Dispatcher
        {
            get
            {
                return _dispatcher;
            }
        }
        public Instrumentation.CommunicatorObserver? Observer
        {
            get
            {
                return _observer;
            }
        }

        public Action? ThreadStart
        {
            get
            {
                return _threadStart;
            }
        }

        public Action? ThreadStop
        {
            get
            {
                return _threadStop;
            }
        }

        /// <summary>
        /// Get the observer resolver object for this communicator.
        /// </summary>
        /// <returns>This communicator's observer resolver object.</returns>
        public Instrumentation.CommunicatorObserver? getObserver()
        {
            return _observer;
        }

        /// <summary>
        /// Get the default router this communicator.
        /// </summary>
        /// <returns>The default router for this communicator.
        ///
        /// </returns>
        public RouterPrx? getDefaultRouter()
        {
            lock (this)
            {
                if (_state == StateDestroyed)
                {
                    throw new CommunicatorDestroyedException();
                }

                return _defaultRouter;
            }
        }

        /// <summary>
        /// Set a default router for this communicator.
        /// All newly
        /// created proxies will use this default router. To disable the
        /// default router, null can be used. Note that this
        /// operation has no effect on existing proxies.
        ///
        /// You can also set a router for an individual proxy
        /// by calling the operation ice_router on the proxy.
        ///
        /// </summary>
        /// <param name="router">The default router to use for this communicator.
        ///
        /// </param>
        public void setDefaultRouter(RouterPrx router)
        {
            lock (this)
            {
                if (_state == StateDestroyed)
                {
                    throw new CommunicatorDestroyedException();
                }

                _defaultRouter = router;
            }
        }

        /// <summary>
        /// Get the default locator this communicator.
        /// </summary>
        /// <returns>The default locator for this communicator.
        ///
        /// </returns>
        public LocatorPrx? getDefaultLocator()
        {
            lock (this)
            {
                if (_state == StateDestroyed)
                {
                    throw new CommunicatorDestroyedException();
                }
                return _defaultLocator;
            }
        }

        /// <summary>
        /// Set a default Ice locator for this communicator.
        /// All newly
        /// created proxy and object adapters will use this default
        /// locator. To disable the default locator, null can be used.
        /// Note that this operation has no effect on existing proxies or
        /// object adapters.
        ///
        /// You can also set a locator for an individual proxy by calling the
        /// operation ice_locator on the proxy, or for an object adapter
        /// by calling ObjectAdapter.setLocator on the object adapter.
        ///
        /// </summary>
        /// <param name="locator">The default locator to use for this communicator.
        ///
        /// </param>
        public void setDefaultLocator(LocatorPrx? locator)
        {
            lock (this)
            {
                if (_state == StateDestroyed)
                {
                    throw new CommunicatorDestroyedException();
                }
                _defaultLocator = locator;
            }
        }

        /// <summary>
        /// Get the implicit context associated with this communicator.
        /// </summary>
        /// <returns>The implicit context associated with this communicator;
        /// returns null when the property Ice.ImplicitContext is not set
        /// or is set to None.</returns>
        public ImplicitContext getImplicitContext()
        {
            return _implicitContext;
        }

        /// <summary>
        /// Get the plug-in manager for this communicator.
        /// </summary>
        /// <returns>This communicator's plug-in manager.
        ///
        /// </returns>
        public PluginManager getPluginManager()
        {
            lock (this)
            {
                if (_state == StateDestroyed)
                {
                    throw new CommunicatorDestroyedException();
                }

                Debug.Assert(_pluginManager != null);
                return _pluginManager;
            }
        }

        internal int messageSizeMax()
        {
            // No mutex lock, immutable.
            return _messageSizeMax;
        }

        internal int classGraphDepthMax()
        {
            // No mutex lock, immutable.
            return _classGraphDepthMax;
        }

        public ToStringMode ToStringMode
        {
            get
            {
                // No mutex lock, immutable
                return _toStringMode;
            }
        }

        internal int CacheMessageBuffers
        {
            get
            {
                // No mutex lock, immutable.
                return _cacheMessageBuffers;
            }
        }

        internal ACMConfig ClientACM
        {
            get
            {
                // No mutex lock, immutable.
                return _clientACM;
            }
        }

        internal ACMConfig ServerACM
        {
            get
            {
                // No mutex lock, immutable.
                return _serverACM;
            }
        }

        /// <summary>
        /// Add the Admin object with all its facets to the provided object adapter.
        /// If Ice.Admin.ServerId is set and the provided object adapter has a Locator,
        /// createAdmin registers the Admin's Process facet with the Locator's LocatorRegistry.
        ///
        /// createAdmin call only be called once; subsequent calls raise InitializationException.
        ///
        /// </summary>
        /// <param name="adminAdapter">The object adapter used to host the Admin object; if null and
        /// Ice.Admin.Endpoints is set, create, activate and use the Ice.Admin object adapter.
        ///
        /// </param>
        /// <param name="adminIdentity">The identity of the Admin object.
        ///
        /// </param>
        /// <returns>A proxy to the main ("") facet of the Admin object. Never returns a null proxy.
        ///
        /// </returns>
        public IObjectPrx CreateAdmin(ObjectAdapter? adminAdapter, Identity adminIdentity)
        {
            lock (this)
            {
                if (_state == StateDestroyed)
                {
                    throw new CommunicatorDestroyedException();
                }

                if (_adminAdapter != null)
                {
                    throw new InitializationException("Admin already created");
                }

                if (!_adminEnabled)
                {
                    throw new InitializationException("Admin is disabled");
                }

                _adminIdentity = adminIdentity;
                if (adminAdapter == null)
                {
                    if (GetProperty("Ice.Admin.Endpoints") != null)
                    {
                        adminAdapter = _objectAdapterFactory!.createObjectAdapter("Ice.Admin", null);
                    }
                    else
                    {
                        throw new InitializationException("Ice.Admin.Endpoints is not set");
                    }
                }
                else
                {
                    _adminAdapter = adminAdapter;
                }
                Debug.Assert(_adminAdapter != null);
                AddAllAdminFacets();
            }

            if (adminAdapter == null)
            {
                try
                {
                    _adminAdapter.Activate();
                }
                catch (LocalException)
                {
                    //
                    // We cleanup _adminAdapter, however this error is not recoverable
                    // (can't call again getAdmin() after fixing the problem)
                    // since all the facets (servants) in the adapter are lost
                    //
                    _adminAdapter.Destroy();
                    lock (this)
                    {
                        _adminAdapter = null;
                    }
                    throw;
                }
            }
            setServerProcessProxy(_adminAdapter, adminIdentity);
            return _adminAdapter.CreateProxy(adminIdentity);
        }

        /// <summary>
        /// Get a proxy to the main facet of the Admin object.
        /// getAdmin also creates the Admin object and creates and activates the Ice.Admin object
        /// adapter to host this Admin object if Ice.Admin.Enpoints is set. The identity of the Admin
        /// object created by getAdmin is {value of Ice.Admin.InstanceName}/admin, or {UUID}/admin
        /// when Ice.Admin.InstanceName is not set.
        ///
        /// If Ice.Admin.DelayCreation is 0 or not set, getAdmin is called by the communicator
        /// initialization, after initialization of all plugins.
        ///
        /// </summary>
        /// <returns>A proxy to the main ("") facet of the Admin object, or a null proxy if no
        /// Admin object is configured.
        ///
        /// </returns>
        public IObjectPrx? getAdmin()
        {
            ObjectAdapter adminAdapter;
            Identity adminIdentity;

            lock (this)
            {
                if (_state == StateDestroyed)
                {
                    throw new CommunicatorDestroyedException();
                }

                if (_adminAdapter != null)
                {
                    Debug.Assert(_adminIdentity != null);
                    return _adminAdapter.CreateProxy(_adminIdentity.Value);
                }
                else if (_adminEnabled)
                {
                    if (GetProperty("Ice.Admin.Endpoints") != null)
                    {
                        adminAdapter = _objectAdapterFactory!.createObjectAdapter("Ice.Admin", null);
                    }
                    else
                    {
                        return null;
                    }
                    adminIdentity = new Identity("admin", GetProperty("Ice.Admin.InstanceName") ?? "");
                    if (adminIdentity.category.Length == 0)
                    {
                        adminIdentity.category = Guid.NewGuid().ToString();
                    }

                    _adminIdentity = adminIdentity;
                    _adminAdapter = adminAdapter;
                    AddAllAdminFacets();
                    // continue below outside synchronization
                }
                else
                {
                    return null;
                }
            }

            try
            {
                adminAdapter.Activate();
            }
            catch (LocalException)
            {
                //
                // We cleanup _adminAdapter, however this error is not recoverable
                // (can't call again getAdmin() after fixing the problem)
                // since all the facets (servants) in the adapter are lost
                //
                adminAdapter.Destroy();
                lock (this)
                {
                    _adminAdapter = null;
                }
                throw;
            }

            setServerProcessProxy(adminAdapter, adminIdentity);
            return adminAdapter.CreateProxy(adminIdentity);
        }

        /// <summary>
        /// Add a new facet to the Admin object.
        /// Adding a servant with a facet that is already registered
        /// throws AlreadyRegisteredException.
        ///
        /// </summary>
        /// <param name="servant">The servant that implements the new Admin facet.
        /// </param>
        /// <param name="facet">The name of the new Admin facet.</param>
        public void AddAdminFacet<T, Traits>(T servant, string facet) where Traits : struct, IInterfaceTraits<T>
        {
            Traits traits = default;
            Disp disp = (incoming, current) => traits.Dispatch(servant, incoming, current);
            Debug.Assert(servant != null);
            AddAdminFacet(servant, disp, facet);
        }

        public void AddAdminFacet(object servant, Disp disp, string facet)
        {
            lock (this)
            {
                if (_state == StateDestroyed)
                {
                    throw new CommunicatorDestroyedException();
                }

                if (_adminFacetFilter.Count > 0 && !_adminFacetFilter.Contains(facet))
                {
                    throw new ArgumentException($"facet `{facet}' not allow by Ice.Admin.Facets configuration", nameof(facet));
                }

                if (_adminFacets.ContainsKey(facet))
                {
                    throw new ArgumentException($"A facet `{facet}' is already registered", nameof(facet));
                }
                _adminFacets.Add(facet, (servant, disp));
                if (_adminAdapter != null)
                {
                    _adminAdapter.Add(disp, _adminIdentity, facet);
                }
            }
        }

        /// <summary>
        /// Remove the following facet to the Admin object.
        /// Removing a facet that was not previously registered throws
        /// NotRegisteredException.
        ///
        /// </summary>
        /// <param name="facet">The name of the Admin facet.
        /// </param>
        /// <returns>The servant associated with this Admin facet.</returns>
        public (object servant, Disp disp) RemoveAdminFacet(string facet)
        {
            lock (this)
            {
                if (_state == StateDestroyed)
                {
                    throw new CommunicatorDestroyedException();
                }

                (object servant, Disp disp) result = default;
                if (!_adminFacets.TryGetValue(facet, out result))
                {
                    throw new Ice.NotRegisteredException("facet", facet);
                }
                _adminFacets.Remove(facet);
                if (_adminAdapter != null)
                {
                    Debug.Assert(_adminIdentity != null);
                    _adminAdapter.Remove(_adminIdentity.Value, facet);
                }
                return result;
            }
        }

        /// <summary>
        /// Returns a facet of the Admin object.
        /// </summary>
        /// <param name="facet">The name of the Admin facet.
        /// </param>
        /// <returns>The servant associated with this Admin facet, or
        /// null if no facet is registered with the given name.</returns>
        public (object servant, Disp disp) FindAdminFacet(string facet)
        {
            lock (this)
            {
                if (_state == StateDestroyed)
                {
                    throw new CommunicatorDestroyedException();
                }

                (object servant, Disp disp) result;
                if (!_adminFacets.TryGetValue(facet, out result))
                {
                    return default;
                }
                return result;
            }
        }

        /// <summary>
        /// Returns a map of all facets of the Admin object.
        /// </summary>
        /// <returns>A collection containing all the facet names and
        /// servants of the Admin object.
        ///
        /// </returns>
        public Dictionary<string, (object servant, Disp disp)> FindAllAdminFacets()
        {
            lock (this)
            {
                if (_state == StateDestroyed)
                {
                    throw new CommunicatorDestroyedException();
                }
                return new Dictionary<string, (object servant, Disp disp)>(_adminFacets);
            }
        }

        public void Dispose()
        {
            destroy();
        }

        public Communicator(Dictionary<string, string>? properties,
                            Func<int, string>? compactIdResolver = null,
                            Action<Action, Connection?>? dispatcher = null,
                            Logger? logger = null,
                            Instrumentation.CommunicatorObserver? observer = null,
                            Action? threadStart = null,
                            Action? threadStop = null,
                            string[]? typeIdNamespaces = null) :
            this(ref _emptyArgs,
                 null,
                 properties,
                 compactIdResolver,
                 dispatcher,
                 logger,
                 observer,
                 threadStart,
                 threadStop,
                 typeIdNamespaces)
        {
        }

        public Communicator(ref string[] args,
                            Dictionary<string, string>? properties,
                            Func<int, string>? compactIdResolver = null,
                            Action<Action, Connection?>? dispatcher = null,
                            Logger? logger = null,
                            Instrumentation.CommunicatorObserver? observer = null,
                            Action? threadStart = null,
                            Action? threadStop = null,
                            string[]? typeIdNamespaces = null) :
            this(ref args,
                 null,
                 properties,
                 compactIdResolver,
                 dispatcher,
                 logger,
                 observer,
                 threadStart,
                 threadStop,
                 typeIdNamespaces)
        {
        }

        public Communicator(NameValueCollection? appSettings = null,
                            Dictionary<string, string>? properties = null,
                            Func<int, string>? compactIdResolver = null,
                            Action<Action, Connection?>? dispatcher = null,
                            Logger? logger = null,
                            Instrumentation.CommunicatorObserver? observer = null,
                            Action? threadStart = null,
                            Action? threadStop = null,
                            string[]? typeIdNamespaces = null) :
            this(ref _emptyArgs,
                 appSettings,
                 properties,
                 compactIdResolver,
                 dispatcher,
                 logger,
                 observer,
                 threadStart,
                 threadStop,
                 typeIdNamespaces)
        {
        }

        public Communicator(ref string[] args,
                            NameValueCollection? appSettings,
                            Dictionary<string, string>? properties = null,
                            Func<int, string>? compactIdResolver = null,
                            Action<Action, Connection?>? dispatcher = null,
                            Logger? logger = null,
                            Instrumentation.CommunicatorObserver? observer = null,
                            Action? threadStart = null,
                            Action? threadStop = null,
                            string[]? typeIdNamespaces = null)
        {
            _state = StateActive;
            _compactIdResolver = compactIdResolver;
            _dispatcher = dispatcher;
            _logger = logger ?? Util.getProcessLogger();
            _observer = observer;
            _threadStart = threadStart;
            _threadStop = threadStop;
            _typeIdNamespaces = typeIdNamespaces ?? new string[] { "Ice.TypeId" };

            properties ??= new Dictionary<string, string>();

            if (appSettings != null)
            {
                foreach (var key in appSettings.AllKeys)
                {
                    string[]? values = appSettings.GetValues(key);
                    if (values == null)
                    {
                        properties[key] = "";
                    }
                    else
                    {
                        // TODO: this join is not sufficient to create a string
                        // compatible with GetPropertyAsList
                        properties[key] = string.Join(",", values); 
                    }
                }
            }

            if (!properties.ContainsKey("Ice.ProgramName"))
            {
                properties["Ice.ProgramName"] = AppDomain.CurrentDomain.FriendlyName;
            }

            properties.ParseIceArgs(ref args);
            _ = SetProperties(properties);

            try
            {
                lock (_staticLock)
                {
                    if (!_oneOffDone)
                    {
                        string? stdOut = GetProperty("Ice.StdOut");

                        System.IO.StreamWriter? outStream = null;

                        if (stdOut != null)
                        {
                            try
                            {
                                outStream = System.IO.File.AppendText(stdOut);
                            }
                            catch (System.IO.IOException ex)
                            {
                                Ice.FileException fe = new Ice.FileException(ex);
                                fe.path = stdOut;
                                throw fe;
                            }
                            outStream.AutoFlush = true;
                            Console.Out.Close();
                            Console.SetOut(outStream);
                        }

                        string? stdErr = GetProperty("Ice.StdErr");
                        if (stdErr != null)
                        {
                            if (stdErr.Equals(stdOut))
                            {
                                Console.SetError(outStream);
                            }
                            else
                            {
                                System.IO.StreamWriter errStream;
                                try
                                {
                                    errStream = System.IO.File.AppendText(stdErr);
                                }
                                catch (System.IO.IOException ex)
                                {
                                    Ice.FileException fe = new Ice.FileException(ex);
                                    fe.path = stdErr;
                                    throw fe;
                                }
                                errStream.AutoFlush = true;
                                Console.Error.Close();
                                Console.SetError(errStream);
                            }
                        }

                        _oneOffDone = true;
                    }
                }

                if (logger == null)
                {
                    string? logfile = GetProperty("Ice.LogFile");
                    string? programName = GetProperty("Ice.ProgramName");
                    Debug.Assert(programName != null);
                    if (logfile != null)
                    {
                        _logger = new FileLoggerI(programName, logfile);
                    }
                    else if (Util.getProcessLogger() is LoggerI)
                    {
                        //
                        // Ice.ConsoleListener is enabled by default.
                        //
                        _logger = new TraceLoggerI(programName, (GetPropertyAsInt("Ice.ConsoleListener") ?? 1) > 0);
                    }
                    // else already set to process logger
                }

                _traceLevels = new TraceLevels(this);

                _defaultsAndOverrides = new DefaultsAndOverrides(this, _logger);

                _clientACM = new ACMConfig(this, _logger, "Ice.ACM.Client",
                                           new ACMConfig(this, _logger, "Ice.ACM", new ACMConfig(false)));

                _serverACM = new ACMConfig(this, _logger, "Ice.ACM.Server",
                                           new ACMConfig(this, _logger, "Ice.ACM", new ACMConfig(true)));

                {
                    int num = GetPropertyAsInt("Ice.MessageSizeMax") ?? 1024;
                    if (num < 1 || num > 0x7fffffff / 1024)
                    {
                        _messageSizeMax = 0x7fffffff;
                    }
                    else
                    {
                        _messageSizeMax = num * 1024; // Property is in kilobytes, _messageSizeMax in bytes
                    }
                }

                {
                    var num = GetPropertyAsInt("Ice.ClassGraphDepthMax") ?? 100;
                    if (num < 1 || num > 0x7fffffff)
                    {
                        _classGraphDepthMax = 0x7fffffff;
                    }
                    else
                    {
                        _classGraphDepthMax = num;
                    }
                }

                _toStringMode = Enum.Parse<ToStringMode>(GetProperty("Ice.ToStringMode") ?? "Unicode");

                _cacheMessageBuffers = GetPropertyAsInt("Ice.CacheMessageBuffers") ?? 2;

                _implicitContext = ImplicitContextI.Create(GetProperty("Ice.ImplicitContext"));
                _routerManager = new RouterManager();

                _locatorManager = new LocatorManager(this);

                string[]? arr = GetPropertyAsList("Ice.RetryIntervals");

                if (arr == null)
                {
                    _retryIntervals = new int[] { 0 };
                }
                else
                {
                    _retryIntervals = new int[arr.Length];
                    for (int i = 0; i < arr.Length; i++)
                    {
                        int v = int.Parse(arr[i], CultureInfo.InvariantCulture);
                        //
                        // If -1 is the first value, no retry and wait intervals.
                        //
                        if (i == 0 && v == -1)
                        {
                            _retryIntervals = Array.Empty<int>();
                            break;
                        }

                        _retryIntervals[i] = v > 0 ? v : 0;
                    }
                }

                _requestHandlerFactory = new RequestHandlerFactory(this);

                bool isIPv6Supported = Network.isIPv6Supported();
                bool ipv4 = (GetPropertyAsInt("Ice.IPv4") ?? 1) > 0;
                bool ipv6 = (GetPropertyAsInt("Ice.IPv6") ?? (isIPv6Supported ? 1 : 0)) > 0;
                if (!ipv4 && !ipv6)
                {
                    throw new InitializationException("Both IPV4 and IPv6 support cannot be disabled.");
                }
                else if (ipv4 && ipv6)
                {
                    _protocolSupport = Network.EnableBoth;
                }
                else if (ipv4)
                {
                    _protocolSupport = Network.EnableIPv4;
                }
                else
                {
                    _protocolSupport = Network.EnableIPv6;
                }
                _preferIPv6 = GetPropertyAsInt("Ice.PreferIPv6Address") > 0;

                _networkProxy = createNetworkProxy(_protocolSupport);

                _endpointFactoryManager = new EndpointFactoryManager(this);

                ProtocolInstance tcpInstance = new ProtocolInstance(this, TCPEndpointType.value, "tcp", false);
                _endpointFactoryManager.add(new TcpEndpointFactory(tcpInstance));

                ProtocolInstance udpInstance = new ProtocolInstance(this, UDPEndpointType.value, "udp", false);
                _endpointFactoryManager.add(new UdpEndpointFactory(udpInstance));

                ProtocolInstance wsInstance = new ProtocolInstance(this, WSEndpointType.value, "ws", false);
                _endpointFactoryManager.add(new WSEndpointFactory(wsInstance, TCPEndpointType.value));

                ProtocolInstance wssInstance = new ProtocolInstance(this, WSSEndpointType.value, "wss", true);
                _endpointFactoryManager.add(new WSEndpointFactory(wssInstance, SSLEndpointType.value));

                _pluginManager = new Ice.PluginManagerI(this);

                _outgoingConnectionFactory = new OutgoingConnectionFactory(this);

                _objectAdapterFactory = new ObjectAdapterFactory(this);

                _retryQueue = new RetryQueue(this);

                if (GetPropertyAsInt("Ice.PreloadAssemblies") > 0)
                {
                    AssemblyUtil.preloadAssemblies();
                }

                //
                // Load plug-ins.
                //
                Debug.Assert(_serverThreadPool == null);
                PluginManagerI pluginManagerImpl = (PluginManagerI)_pluginManager;
                pluginManagerImpl.loadPlugins(ref args);

                //
                // Initialize the endpoint factories once all the plugins are loaded. This gives
                // the opportunity for the endpoint factories to find underyling factories.
                //
                _endpointFactoryManager.initialize();

                //
                // Create Admin facets, if enabled.
                //
                // Note that any logger-dependent admin facet must be created after we load all plugins,
                // since one of these plugins can be a Logger plugin that sets a new logger during loading
                //

                if (GetProperty("Ice.Admin.Enabled") == null)
                {
                    _adminEnabled = GetProperty("Ice.Admin.Endpoints") != null;
                }
                else
                {
                    _adminEnabled = GetPropertyAsInt("Ice.Admin.Enabled") > 0;
                }

                _adminFacetFilter = new HashSet<string>(
                    (GetPropertyAsList("Ice.Admin.Facets") ?? Array.Empty<string>()).Distinct());

                if (_adminEnabled)
                {
                    //
                    // Process facet
                    //
                    string processFacetName = "Process";
                    if (_adminFacetFilter.Count == 0 || _adminFacetFilter.Contains(processFacetName))
                    {
                        ProcessTraits traits = default;
                        Process process = new ProcessI(this);
                        Disp disp = (current, incoming) => traits.Dispatch(process, current, incoming);
                        _adminFacets.Add(processFacetName, (process, disp));
                    }

                    //
                    // Logger facet
                    //
                    string loggerFacetName = "Logger";
                    if (_adminFacetFilter.Count == 0 || _adminFacetFilter.Contains(loggerFacetName))
                    {
                        LoggerAdminLogger loggerAdminLogger = new LoggerAdminLoggerI(this, _logger);
                        setLogger(loggerAdminLogger);
                        LoggerAdminTraits traits = default;
                        LoggerAdmin servant = loggerAdminLogger.getFacet();
                        Disp disp = (incoming, current) => traits.Dispatch(servant, incoming, current);
                        _adminFacets.Add(loggerFacetName, (servant, disp));
                    }

                    //
                    // Properties facet
                    //
                    string propertiesFacetName = "Properties";
                    PropertiesAdminI? propsAdmin = null;
                    if (_adminFacetFilter.Count == 0 || _adminFacetFilter.Contains(propertiesFacetName))
                    {
                        propsAdmin = new PropertiesAdminI(this);
                        PropertiesAdminTraits traits = default;
                        Disp disp = (current, incoming) => traits.Dispatch(propsAdmin, current, incoming);
                        _adminFacets.Add(propertiesFacetName, (propsAdmin, disp));
                    }

                    //
                    // Metrics facet
                    //
                    string metricsFacetName = "Metrics";
                    if (_adminFacetFilter.Count == 0 || _adminFacetFilter.Contains(metricsFacetName))
                    {
                        var communicatorObserver = new CommunicatorObserverI(this, _logger);
                        _observer = communicatorObserver;
                        IceMX.MetricsAdminTraits traits = default;
                        var metricsAdmin = communicatorObserver.getFacet();
                        Disp disp = (current, incoming) => traits.Dispatch(metricsAdmin, current, incoming);
                        _adminFacets.Add(metricsFacetName, (metricsAdmin, disp));

                        //
                        // Make sure the admin plugin receives property updates.
                        //
                        if (propsAdmin != null)
                        {
                            propsAdmin.addUpdateCallback((Dictionary<string, string> updates) =>
                            {
                                communicatorObserver.getFacet().updated(updates);
                            });
                        }
                    }
                }

                //
                // Set observer updater
                //
                if (_observer != null)
                {
                    _observer.setObserverUpdater(new ObserverUpdaterI(this));
                }

                //
                // Create threads.
                //
                try
                {
                    _timer = new IceInternal.Timer(this, IceInternal.Util.stringToThreadPriority(
                                                   GetProperty("Ice.ThreadPriority")));
                }
                catch (System.Exception ex)
                {
                    Logger.error($"cannot create thread for timer:\n{ex}");
                    throw;
                }

                try
                {
                    _endpointHostResolver = new EndpointHostResolver(this);
                }
                catch (System.Exception ex)
                {
                    Logger.error($"cannot create thread for endpoint host resolver:\n{ex}");
                    throw;
                }
                _clientThreadPool = new IceInternal.ThreadPool(this, "Ice.ThreadPool.Client", 0);

                //
                // The default router/locator may have been set during the loading of plugins.
                // Therefore we make sure it is not already set before checking the property.
                //
                if (getDefaultRouter() == null)
                {
                    RouterPrx? router = GetPropertyAsProxy("Ice.Default.Router", RouterPrx.Factory);
                    if (router != null)
                    {
                        setDefaultRouter(router);
                    }
                }

                if (getDefaultLocator() == null)
                {
                    LocatorPrx? locator = GetPropertyAsProxy("Ice.Default.Locator", LocatorPrx.Factory);
                    if (locator != null)
                    {
                        setDefaultLocator(locator);
                    }
                }

                //
                // Show process id if requested (but only once).
                //
                lock (this)
                {
                    if (!_printProcessIdDone && GetPropertyAsInt("Ice.PrintProcessId") > 0)
                    {
                        using var p = System.Diagnostics.Process.GetCurrentProcess();
                        Console.WriteLine(p.Id);
                        _printProcessIdDone = true;
                    }
                }

                //
                // Server thread pool initialization is lazy in serverThreadPool().
                //

                //
                // An application can set Ice.InitPlugins=0 if it wants to postpone
                // initialization until after it has interacted directly with the
                // plug-ins.
                //
                if ((GetPropertyAsInt("Ice.InitPlugins") ?? 1) > 0)
                {
                    pluginManagerImpl.initializePlugins();
                }

                //
                // This must be done last as this call creates the Ice.Admin object adapter
                // and eventually registers a process proxy with the Ice locator (allowing
                // remote clients to invoke on Ice.Admin facets as soon as it's registered).
                //
                if ((GetPropertyAsInt("Ice.Admin.DelayCreation") ?? 0) <= 0)
                {
                    getAdmin();
                }
            }
            catch (System.Exception)
            {
                destroy();
                throw;
            }
        }

        public ObjectAdapterFactory objectAdapterFactory()
        {
            lock (this)
            {
                if (_state == StateDestroyed)
                {
                    throw new CommunicatorDestroyedException();
                }

                Debug.Assert(_objectAdapterFactory != null);
                return _objectAdapterFactory;
            }
        }

        internal void
       setLogger(Logger logger)
        {
            //
            // No locking, as it can only be called during plug-in loading
            //
            _logger = logger;
        }

        internal void
        setThreadHook(Action threadStart, Action threadStop)
        {
            //
            // No locking, as it can only be called during plug-in loading
            //
            _threadStart = threadStart;
            _threadStop = threadStop;
        }

        internal string resolveCompactId(int compactId)
        {
            string[] defaultVal = { "IceCompactId" };
            var compactIdNamespaces = new List<string>(defaultVal);

            if (_typeIdNamespaces != null)
            {
                compactIdNamespaces.AddRange(_typeIdNamespaces);
            }

            string result = "";

            foreach (var ns in compactIdNamespaces)
            {
                string className = ns + ".TypeId_" + compactId;
                try
                {
                    Type? c = AssemblyUtil.findType(className);
                    if (c != null)
                    {
                        result = (string)c.GetField("typeId").GetValue(null);
                        break; // foreach
                    }
                }
                catch (System.Exception)
                {
                }
            }
            return result;
        }

        private static string typeToClass(string id)
        {
            if (!id.StartsWith("::", StringComparison.Ordinal))
            {
                throw new MarshalException("expected type id but received `" + id + "'");
            }
            return id.Substring(2).Replace("::", ".");
        }

        //
        // Return the C# class associated with this Slice type-id
        // Used for both non-local Slice classes and exceptions
        //
        internal Type? resolveClass(string id)
        {
            // First attempt corresponds to no cs:namespace metadata in the
            // enclosing top-level module
            //
            string className = typeToClass(id);
            Type? c = AssemblyUtil.findType(className);

            //
            // If this fails, look for helper classes in the typeIdNamespaces namespace(s)
            //
            if (c == null && _typeIdNamespaces != null)
            {
                foreach (var ns in _typeIdNamespaces)
                {
                    Type? helper = AssemblyUtil.findType(ns + "." + className);
                    if (helper != null)
                    {
                        try
                        {
                            c = helper.GetProperty("targetClass").PropertyType;
                            break; // foreach
                        }
                        catch (System.Exception)
                        {
                        }
                    }
                }
            }

            //
            // Ensure the class is instantiable.
            //
            if (c != null && !c.IsAbstract && !c.IsInterface)
            {
                return c;
            }

            return null;
        }

        public RouterManager routerManager()
        {
            lock (this)
            {
                if (_state == StateDestroyed)
                {
                    throw new CommunicatorDestroyedException();
                }

                Debug.Assert(_routerManager != null);
                return _routerManager;
            }
        }

        public LocatorManager locatorManager()
        {
            lock (this)
            {
                if (_state == StateDestroyed)
                {
                    throw new CommunicatorDestroyedException();
                }

                Debug.Assert(_locatorManager != null);
                return _locatorManager;
            }
        }

        internal RequestHandlerFactory requestHandlerFactory()
        {
            lock (this)
            {
                if (_state == StateDestroyed)
                {
                    throw new CommunicatorDestroyedException();
                }

                Debug.Assert(_requestHandlerFactory != null);
                return _requestHandlerFactory;
            }
        }

        internal OutgoingConnectionFactory outgoingConnectionFactory()
        {
            lock (this)
            {
                if (_state == StateDestroyed)
                {
                    throw new CommunicatorDestroyedException();
                }

                Debug.Assert(_outgoingConnectionFactory != null);
                return _outgoingConnectionFactory;
            }
        }

        internal int ProtocolSupport
        {
            get
            {
                return _protocolSupport;
            }
        }

        internal bool PreferIPv6
        {
            get
            {
                return _preferIPv6;
            }
        }

        internal NetworkProxy? NetworkProxy
        {
            get
            {
                return _networkProxy;
            }
        }

        internal IceInternal.ThreadPool clientThreadPool()
        {
            lock (this)
            {
                if (_state == StateDestroyed)
                {
                    throw new CommunicatorDestroyedException();
                }

                Debug.Assert(_clientThreadPool != null);
                return _clientThreadPool;
            }
        }

        internal IceInternal.ThreadPool serverThreadPool()
        {
            lock (this)
            {
                if (_state == StateDestroyed)
                {
                    throw new CommunicatorDestroyedException();
                }

                if (_serverThreadPool == null) // Lazy initialization.
                {
                    if (_state == StateDestroyInProgress)
                    {
                        throw new CommunicatorDestroyedException();
                    }
                    _serverThreadPool = new IceInternal.ThreadPool(this, "Ice.ThreadPool.Server",
                        GetPropertyAsInt("Ice.ServerIdleTime") ?? 0);
                }

                return _serverThreadPool;
            }
        }

        internal AsyncIOThread
        asyncIOThread()
        {
            lock (this)
            {
                if (_state == StateDestroyed)
                {
                    throw new CommunicatorDestroyedException();
                }

                if (_asyncIOThread == null) // Lazy initialization.
                {
                    _asyncIOThread = new AsyncIOThread(this);
                }

                return _asyncIOThread;
            }
        }

        internal EndpointHostResolver endpointHostResolver()
        {
            lock (this)
            {
                if (_state == StateDestroyed)
                {
                    throw new CommunicatorDestroyedException();
                }

                Debug.Assert(_endpointHostResolver != null);
                return _endpointHostResolver;
            }
        }

        internal RetryQueue
        retryQueue()
        {
            lock (this)
            {
                if (_state == StateDestroyed)
                {
                    throw new CommunicatorDestroyedException();
                }

                Debug.Assert(_retryQueue != null);
                return _retryQueue;
            }
        }

        public IceInternal.Timer
        timer()
        {
            lock (this)
            {
                if (_state == StateDestroyed)
                {
                    throw new CommunicatorDestroyedException();
                }

                Debug.Assert(_timer != null);
                return _timer;
            }
        }

        internal EndpointFactoryManager endpointFactoryManager()
        {
            lock (this)
            {
                if (_state == StateDestroyed)
                {
                    throw new CommunicatorDestroyedException();
                }

                Debug.Assert(_endpointFactoryManager != null);
                return _endpointFactoryManager;
            }
        }

        internal BufSizeWarnInfo GetBufSizeWarn(short type)
        {
            lock (_setBufSizeWarn)
            {
                BufSizeWarnInfo info;
                if (!_setBufSizeWarn.ContainsKey(type))
                {
                    info = new BufSizeWarnInfo();
                    info.sndWarn = false;
                    info.sndSize = -1;
                    info.rcvWarn = false;
                    info.rcvSize = -1;
                    _setBufSizeWarn.Add(type, info);
                }
                else
                {
                    info = _setBufSizeWarn[type];
                }
                return info;
            }
        }

        internal void SetSndBufSizeWarn(short type, int size)
        {
            lock (_setBufSizeWarn)
            {
                BufSizeWarnInfo info = GetBufSizeWarn(type);
                info.sndWarn = true;
                info.sndSize = size;
                _setBufSizeWarn[type] = info;
            }
        }

        internal void SetRcvBufSizeWarn(short type, int size)
        {
            lock (_setBufSizeWarn)
            {
                BufSizeWarnInfo info = GetBufSizeWarn(type);
                info.rcvWarn = true;
                info.rcvSize = size;
                _setBufSizeWarn[type] = info;
            }
        }

        internal void updateConnectionObservers()
        {
            try
            {
                Debug.Assert(_outgoingConnectionFactory != null);
                _outgoingConnectionFactory.updateConnectionObservers();
                Debug.Assert(_objectAdapterFactory != null);
                _objectAdapterFactory.updateConnectionObservers();
            }
            catch (CommunicatorDestroyedException)
            {
            }
        }

        internal void updateThreadObservers()
        {
            try
            {
                if (_clientThreadPool != null)
                {
                    _clientThreadPool.updateObservers();
                }
                if (_serverThreadPool != null)
                {
                    _serverThreadPool.updateObservers();
                }
                Debug.Assert(_objectAdapterFactory != null);
                _objectAdapterFactory.updateThreadObservers();
                if (_endpointHostResolver != null)
                {
                    _endpointHostResolver.updateObserver();
                }
                if (_asyncIOThread != null)
                {
                    _asyncIOThread.updateObserver();
                }
                if (_timer != null)
                {
                    Debug.Assert(_observer != null);
                    _timer.updateObserver(_observer);
                }
            }
            catch (CommunicatorDestroyedException)
            {
            }
        }

        private void AddAllAdminFacets()
        {
            lock (this)
            {
                Debug.Assert(_adminAdapter != null);
                foreach (var entry in _adminFacets)
                {
                    if (_adminFacetFilter.Count == 0 || _adminFacetFilter.Contains(entry.Key))
                    {
                        _adminAdapter.Add(entry.Value.disp, _adminIdentity, entry.Key);
                    }
                }
            }
        }

        internal void setServerProcessProxy(ObjectAdapter adminAdapter, Identity adminIdentity)
        {
            IObjectPrx? admin = adminAdapter.CreateProxy(adminIdentity);
            LocatorPrx? locator = adminAdapter.GetLocator();
            string? serverId = GetProperty("Ice.Admin.ServerId");

            if (locator != null && serverId != null)
            {
                ProcessPrx process = ProcessPrx.UncheckedCast(admin.Clone(facet: "Process"));
                try
                {
                    //
                    // Note that as soon as the process proxy is registered, the communicator might be
                    // shutdown by a remote client and admin facets might start receiving calls.
                    //
                    locator.getRegistry().setServerProcessProxy(serverId, process);
                }
                catch (ServerNotFoundException)
                {
                    if (_traceLevels.location >= 1)
                    {
                        StringBuilder s = new StringBuilder();
                        s.Append("couldn't register server `" + serverId + "' with the locator registry:\n");
                        s.Append("the server is not known to the locator registry");
                        Logger.trace(_traceLevels.locationCat, s.ToString());
                    }

                    throw new InitializationException("Locator knows nothing about server `" + serverId + "'");
                }
                catch (LocalException ex)
                {
                    if (_traceLevels.location >= 1)
                    {
                        StringBuilder s = new StringBuilder();
                        s.Append("couldn't register server `" + serverId + "' with the locator registry:\n" + ex);
                        Logger.trace(_traceLevels.locationCat, s.ToString());
                    }
                    throw; // TODO: Shall we raise a special exception instead of a non obvious local exception?
                }

                if (_traceLevels.location >= 1)
                {
                    StringBuilder s = new StringBuilder();
                    s.Append("registered server `" + serverId + "' with the locator registry");
                    Logger.trace(_traceLevels.locationCat, s.ToString());
                }
            }
        }

        private NetworkProxy? createNetworkProxy(int protocolSupport)
        {
            string? proxyHost = GetProperty("Ice.SOCKSProxyHost");
            if (proxyHost != null)
            {
                if (protocolSupport == Network.EnableIPv6)
                {
                    throw new InitializationException("IPv6 only is not supported with SOCKS4 proxies");
                }
                return new SOCKSNetworkProxy(proxyHost, GetPropertyAsInt("Ice.SOCKSProxyPort") ?? 1080);
            }

            proxyHost = GetProperty("Ice.HTTPProxyHost");
            if (proxyHost != null)
            {
                return new HTTPNetworkProxy(proxyHost, GetPropertyAsInt("Ice.HTTPProxyPort") ?? 1080);
            }

            return null;
        }

        internal int CheckRetryAfterException(LocalException ex, Reference @ref, ref int cnt)
        {
            Ice.Logger logger = Logger;

            if (@ref.getMode() == InvocationMode.BatchOneway || @ref.getMode() == InvocationMode.BatchDatagram)
            {
                Debug.Assert(false); // batch no longer implemented anyway
                throw ex;
            }

            if (ex is ObjectNotExistException)
            {
                ObjectNotExistException one = (ObjectNotExistException)ex;
                RouterInfo? ri = @ref.getRouterInfo();
                if (ri != null && one.operation.Equals("ice_add_proxy"))
                {
                    //
                    // If we have a router, an ObjectNotExistException with an
                    // operation name "ice_add_proxy" indicates to the client
                    // that the router isn't aware of the proxy (for example,
                    // because it was evicted by the router). In this case, we
                    // must *always* retry, so that the missing proxy is added
                    // to the router.
                    //

                    ri.clearCache(@ref);

                    if (_traceLevels.retry >= 1)
                    {
                        string s = "retrying operation call to add proxy to router\n" + ex;
                        logger.trace(_traceLevels.retryCat, s);
                    }
                    return 0; // We must always retry, so we don't look at the retry count.
                }
                else if (@ref.isIndirect())
                {
                    //
                    // We retry ObjectNotExistException if the reference is
                    // indirect.
                    //

                    if (@ref.isWellKnown())
                    {
                        @ref.getLocatorInfo()?.clearCache(@ref);
                    }
                }
                else
                {
                    //
                    // For all other cases, we don't retry ObjectNotExistException.
                    //
                    throw ex;
                }
            }
            else if (ex is RequestFailedException)
            {
                throw ex;
            }

            //
            // There is no point in retrying an operation that resulted in a
            // MarshalException. This must have been raised locally (because if
            // it happened in a server it would result in an UnknownLocalException
            // instead), which means there was a problem in this process that will
            // not change if we try again.
            //
            if (ex is MarshalException)
            {
                throw ex;
            }

            //
            // Don't retry if the communicator is destroyed, object adapter is deactivated,
            // or connection is manually closed.
            //
            if (ex is CommunicatorDestroyedException ||
                ex is ObjectAdapterDeactivatedException ||
                ex is ConnectionManuallyClosedException)
            {
                throw ex;
            }

            //
            // Don't retry invocation timeouts.
            //
            if (ex is InvocationTimeoutException || ex is InvocationCanceledException)
            {
                throw ex;
            }

            ++cnt;
            Debug.Assert(cnt > 0);

            int interval;
            if (cnt == (_retryIntervals.Length + 1) && ex is Ice.CloseConnectionException)
            {
                //
                // A close connection exception is always retried at least once, even if the retry
                // limit is reached.
                //
                interval = 0;
            }
            else if (cnt > _retryIntervals.Length)
            {
                if (_traceLevels.retry >= 1)
                {
                    string s = "cannot retry operation call because retry limit has been exceeded\n" + ex;
                    logger.trace(_traceLevels.retryCat, s);
                }
                throw ex;
            }
            else
            {
                interval = _retryIntervals[cnt - 1];
            }

            if (_traceLevels.retry >= 1)
            {
                string s = "retrying operation call";
                if (interval > 0)
                {
                    s += " in " + interval + "ms";
                }
                s += " because of exception\n" + ex;
                logger.trace(_traceLevels.retryCat, s);
            }

            return interval;
        }

        internal Reference
        CreateReference(Identity ident, string facet, Reference tmpl, EndpointI[] endpoints)
        {
            return CreateReference(ident, facet, tmpl.getMode(), tmpl.getSecure(), tmpl.getProtocol(), tmpl.getEncoding(),
                          endpoints, null, null);
        }

        internal Reference
        CreateReference(Identity ident, string facet, Reference tmpl, string adapterId)
        {
            //
            // Create new reference
            //
            return CreateReference(ident, facet, tmpl.getMode(), tmpl.getSecure(), tmpl.getProtocol(), tmpl.getEncoding(),
                          Array.Empty<EndpointI>(), adapterId, null);
        }

        internal Reference CreateReference(Identity ident, ConnectionI connection)
        {
            //
            // Create new reference
            //
            return new FixedReference(
                this,
                ident,
                "", // Facet
                connection.endpoint().datagram() ? Ice.InvocationMode.Datagram : Ice.InvocationMode.Twoway,
                connection.endpoint().secure(),
                Util.Protocol_1_0,
                _defaultsAndOverrides.defaultEncoding,
                connection,
                -1,
                null,
                null);
        }

        public Reference CreateReference(string s, string? propertyPrefix = null)
        {
            const string delim = " \t\n\r";

            int beg;
            int end = 0;

            beg = IceUtilInternal.StringUtil.findFirstNotOf(s, delim, end);
            if (beg == -1)
            {
                throw new FormatException($"no non-whitespace characters found in `{s}'");
            }

            //
            // Extract the identity, which may be enclosed in single
            // or double quotation marks.
            //
            string idstr;
            end = IceUtilInternal.StringUtil.checkQuote(s, beg);
            if (end == -1)
            {
                throw new FormatException($"mismatched quotes around identity in `{s} '");
            }
            else if (end == 0)
            {
                end = IceUtilInternal.StringUtil.findFirstOf(s, delim + ":@", beg);
                if (end == -1)
                {
                    end = s.Length;
                }
                idstr = s.Substring(beg, end - beg);
            }
            else
            {
                beg++; // Skip leading quote
                idstr = s.Substring(beg, end - beg);
                end++; // Skip trailing quote
            }

            if (beg == end)
            {
                throw new FormatException($"no identity in `{s}'");
            }

            //
            // Parsing the identity may raise FormatException.
            //
            Identity ident = Identity.Parse(idstr);

            string facet = "";
            InvocationMode mode = InvocationMode.Twoway;
            bool secure = false;
            EncodingVersion encoding = _defaultsAndOverrides.defaultEncoding;
            ProtocolVersion protocol = Ice.Util.Protocol_1_0;
            string adapter;

            while (true)
            {
                beg = IceUtilInternal.StringUtil.findFirstNotOf(s, delim, end);
                if (beg == -1)
                {
                    break;
                }

                if (s[beg] == ':' || s[beg] == '@')
                {
                    break;
                }

                end = IceUtilInternal.StringUtil.findFirstOf(s, delim + ":@", beg);
                if (end == -1)
                {
                    end = s.Length;
                }

                if (beg == end)
                {
                    break;
                }

                string option = s.Substring(beg, end - beg);
                if (option.Length != 2 || option[0] != '-')
                {
                    throw new FormatException("expected a proxy option but found `{option}' in `{s}'");
                }

                //
                // Check for the presence of an option argument. The
                // argument may be enclosed in single or double
                // quotation marks.
                //
                string? argument = null;
                int argumentBeg = IceUtilInternal.StringUtil.findFirstNotOf(s, delim, end);
                if (argumentBeg != -1)
                {
                    char ch = s[argumentBeg];
                    if (ch != '@' && ch != ':' && ch != '-')
                    {
                        beg = argumentBeg;
                        end = IceUtilInternal.StringUtil.checkQuote(s, beg);
                        if (end == -1)
                        {
                            throw new FormatException($"mismatched quotes around value for {option} option in `{s}'");
                        }
                        else if (end == 0)
                        {
                            end = IceUtilInternal.StringUtil.findFirstOf(s, delim + ":@", beg);
                            if (end == -1)
                            {
                                end = s.Length;
                            }
                            argument = s.Substring(beg, end - beg);
                        }
                        else
                        {
                            beg++; // Skip leading quote
                            argument = s.Substring(beg, end - beg);
                            end++; // Skip trailing quote
                        }
                    }
                }

                //
                // If any new options are added here,
                // IceInternal::Reference::toString() and its derived classes must be updated as well.
                //
                switch (option[1])
                {
                    case 'f':
                        {
                            if (argument == null)
                            {
                                throw new FormatException($"no argument provided for -f option in `{s}'");
                            }

                            facet = IceUtilInternal.StringUtil.unescapeString(argument, 0, argument.Length, "");
                            break;
                        }

                    case 't':
                        {
                            if (argument != null)
                            {
                                throw new FormatException(
                                    $"unexpected argument `{argument}' provided for -t option in `{s}'");
                            }
                            mode = InvocationMode.Twoway;
                            break;
                        }

                    case 'o':
                        {
                            if (argument != null)
                            {
                                throw new FormatException(
                                    $"unexpected argument `{argument}' provided for -o option in `{s}'");
                            }
                            mode = InvocationMode.Oneway;
                            break;
                        }

                    case 'O':
                        {
                            if (argument != null)
                            {
                                throw new FormatException(
                                    $"unexpected argument `{argument}' provided for -O option in `{s}'");
                            }
                            mode = InvocationMode.BatchOneway;
                            break;
                        }

                    case 'd':
                        {
                            if (argument != null)
                            {
                                throw new FormatException(
                                    $"unexpected argument `{argument}' provided for -d option in `{s}'");
                            }
                            mode = InvocationMode.Datagram;
                            break;
                        }

                    case 'D':
                        {
                            if (argument != null)
                            {
                                throw new FormatException(
                                    $"unexpected argument `{argument}' provided for -D option in `{s}'");
                            }
                            mode = InvocationMode.BatchDatagram;
                            break;
                        }

                    case 's':
                        {
                            if (argument != null)
                            {
                                throw new FormatException(
                                    $"unexpected argument `{argument}' provided for -s option in `{s}'");
                            }
                            secure = true;
                            break;
                        }

                    case 'e':
                        {
                            if (argument == null)
                            {
                                throw new FormatException($"no argument provided for -e option in `{s}'");
                            }

                            encoding = Ice.Util.stringToEncodingVersion(argument);
                            break;
                        }

                    case 'p':
                        {
                            if (argument == null)
                            {
                                throw new FormatException($"no argument provided for -p option `{s}'");
                            }

                            protocol = Ice.Util.stringToProtocolVersion(argument);
                            break;
                        }

                    default:
                        {
                            throw new FormatException("unknown option `{option}' in `{s}'");
                        }
                }
            }

            if (beg == -1)
            {
                return CreateReference(ident, facet, mode, secure, protocol, encoding, Array.Empty<EndpointI>(),
                    null, propertyPrefix);
            }

            List<EndpointI> endpoints = new List<EndpointI>();

            if (s[beg] == ':')
            {
                List<string> unknownEndpoints = new List<string>();
                end = beg;

                while (end < s.Length && s[end] == ':')
                {
                    beg = end + 1;

                    end = beg;
                    while (true)
                    {
                        end = s.IndexOf(':', end);
                        if (end == -1)
                        {
                            end = s.Length;
                            break;
                        }
                        else
                        {
                            bool quoted = false;
                            int quote = beg;
                            while (true)
                            {
                                quote = s.IndexOf('\"', quote);
                                if (quote == -1 || end < quote)
                                {
                                    break;
                                }
                                else
                                {
                                    quote = s.IndexOf('\"', ++quote);
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

                    string es = s.Substring(beg, end - beg);
                    EndpointI endp = endpointFactoryManager().create(es, false);
                    if (endp != null)
                    {
                        endpoints.Add(endp);
                    }
                    else
                    {
                        unknownEndpoints.Add(es);
                    }
                }
                if (endpoints.Count == 0)
                {
                    Debug.Assert(unknownEndpoints.Count > 0);
                    throw new FormatException($"invalid endpoint `{unknownEndpoints[0]}' in `{s}'");
                }
                else if (unknownEndpoints.Count != 0 && (GetPropertyAsInt("Ice.Warn.Endpoints") ?? 1) > 0)
                {
                    StringBuilder msg = new StringBuilder("Proxy contains unknown endpoints:");
                    int sz = unknownEndpoints.Count;
                    for (int idx = 0; idx < sz; ++idx)
                    {
                        msg.Append(" `");
                        msg.Append(unknownEndpoints[idx]);
                        msg.Append("'");
                    }
                    Logger.warning(msg.ToString());
                }

                EndpointI[] ep = endpoints.ToArray();
                return CreateReference(ident, facet, mode, secure, protocol, encoding, ep, null, propertyPrefix);
            }
            else if (s[beg] == '@')
            {
                beg = IceUtilInternal.StringUtil.findFirstNotOf(s, delim, beg + 1);
                if (beg == -1)
                {
                    throw new ArgumentException($"missing adapter id in `{s}'");
                }

                string adapterstr;
                end = IceUtilInternal.StringUtil.checkQuote(s, beg);
                if (end == -1)
                {
                    throw new ArgumentException($"mismatched quotes around adapter id in `{s}'");
                }
                else if (end == 0)
                {
                    end = IceUtilInternal.StringUtil.findFirstOf(s, delim, beg);
                    if (end == -1)
                    {
                        end = s.Length;
                    }
                    adapterstr = s.Substring(beg, end - beg);
                }
                else
                {
                    beg++; // Skip leading quote
                    adapterstr = s.Substring(beg, end - beg);
                    end++; // Skip trailing quote
                }

                if (end != s.Length && IceUtilInternal.StringUtil.findFirstNotOf(s, delim, end) != -1)
                {
                    throw new ArgumentException(
                        $"invalid trailing characters after `{s.Substring(0, end + 1)}' in `{s}'");
                }

                adapter = IceUtilInternal.StringUtil.unescapeString(adapterstr, 0, adapterstr.Length, "");

                if (adapter.Length == 0)
                {
                    throw new ArgumentException($"empty adapter id in `{s}'");
                }
                return CreateReference(ident, facet, mode, secure, protocol, encoding, Array.Empty<EndpointI>(),
                    adapter, propertyPrefix);
            }

            throw new ArgumentException($"malformed proxy `{s}'");
        }

        public Reference CreateReference(Identity ident, InputStream s)
        {
            //
            // Don't read the identity here. Operations calling this
            // constructor read the identity, and pass it as a parameter.
            //

            //
            // For compatibility with the old FacetPath.
            //
            string[] facetPath = s.ReadStringSeq();
            string facet;
            if (facetPath.Length > 0)
            {
                if (facetPath.Length > 1)
                {
                    throw new ProxyUnmarshalException();
                }
                facet = facetPath[0];
            }
            else
            {
                facet = "";
            }

            int mode = s.ReadByte();
            if (mode < 0 || mode > (int)InvocationMode.Last)
            {
                throw new ProxyUnmarshalException();
            }

            bool secure = s.ReadBool();

            ProtocolVersion protocol;
            EncodingVersion encoding;
            if (!s.GetEncoding().Equals(Ice.Util.Encoding_1_0))
            {
                protocol = new ProtocolVersion();
                protocol.ice_readMembers(s);
                encoding = new EncodingVersion();
                encoding.ice_readMembers(s);
            }
            else
            {
                protocol = Ice.Util.Protocol_1_0;
                encoding = Ice.Util.Encoding_1_0;
            }

            EndpointI[] endpoints;
            string adapterId = "";

            int sz = s.ReadSize();
            if (sz > 0)
            {
                endpoints = new EndpointI[sz];
                for (int i = 0; i < sz; i++)
                {
                    endpoints[i] = endpointFactoryManager().read(s);
                }
            }
            else
            {
                endpoints = Array.Empty<EndpointI>();
                adapterId = s.ReadString();
            }

            return CreateReference(ident, facet, (InvocationMode)mode, secure, protocol, encoding, endpoints, adapterId,
                                   null);
        }

        private static readonly string[] _suffixes =
        {
            "EndpointSelection",
            "ConnectionCached",
            "PreferSecure",
            "LocatorCacheTimeout",
            "InvocationTimeout",
            "Locator",
            "Router",
            "CollocationOptimized",
            "Context\\..*"
        };

        private void
        checkForUnknownProperties(string prefix)
        {
            //
            // Do not warn about unknown properties if Ice prefix, ie Ice, Glacier2, etc
            //
            foreach (string name in PropertyNames.clPropNames)
            {
                if (prefix.StartsWith(string.Format("{0}.", name), StringComparison.Ordinal))
                {
                    return;
                }
            }

            List<string> unknownProps = new List<string>();
            Dictionary<string, string> props = GetProperties(forPrefix: $"{prefix}.");
            foreach (string prop in props.Keys)
            {
                bool valid = false;
                for (int i = 0; i < _suffixes.Length; ++i)
                {
                    string pattern = "^" + Regex.Escape(prefix + ".") + _suffixes[i] + "$";
                    if (new Regex(pattern).Match(prop).Success)
                    {
                        valid = true;
                        break;
                    }
                }

                if (!valid)
                {
                    unknownProps.Add(prop);
                }
            }

            if (unknownProps.Count != 0)
            {
                StringBuilder message = new StringBuilder("found unknown properties for proxy '");
                message.Append(prefix);
                message.Append("':");
                foreach (string s in unknownProps)
                {
                    message.Append("\n    ");
                    message.Append(s);
                }
                Logger.warning(message.ToString());
            }
        }

        private Reference CreateReference(
            Identity ident,
            string facet,
            InvocationMode mode,
            bool secure,
            ProtocolVersion protocol,
            EncodingVersion encoding,
            EndpointI[] endpoints,
            string? adapterId,
            string? propertyPrefix)
        {
            //
            // Default local proxy options.
            //
            LocatorInfo? locatorInfo = null;
            if (_defaultLocator != null)
            {
                if (!_defaultLocator.IceReference.getEncoding().Equals(encoding))
                {
                    locatorInfo = locatorManager().get(_defaultLocator.Clone(encodingVersion: encoding));
                }
                else
                {
                    locatorInfo = locatorManager().get(_defaultLocator);
                }
            }
            RouterInfo? routerInfo = null;
            if (_defaultRouter != null)
            {
                routerInfo = routerManager().get(_defaultRouter);
            }
            bool collocOptimized = _defaultsAndOverrides.defaultCollocationOptimization;
            bool cacheConnection = true;
            bool preferSecure = _defaultsAndOverrides.defaultPreferSecure;
            EndpointSelectionType endpointSelection = _defaultsAndOverrides.defaultEndpointSelection;
            int locatorCacheTimeout = _defaultsAndOverrides.defaultLocatorCacheTimeout;
            int invocationTimeout = _defaultsAndOverrides.defaultInvocationTimeout;
            Dictionary<string, string>? context = null;

            //
            // Override the defaults with the proxy properties if a property prefix is defined.
            //
            if (propertyPrefix != null && propertyPrefix.Length > 0)
            {
                //
                // Warn about unknown properties.
                //
                if ((GetPropertyAsInt("Ice.Warn.UnknownProperties") ?? 1) > 0)
                {
                    checkForUnknownProperties(propertyPrefix);
                }

                string property = $"{propertyPrefix}.Locator";
                LocatorPrx? locator = GetPropertyAsProxy(property, LocatorPrx.Factory);
                if (locator != null)
                {
                    if (!locator.IceReference.getEncoding().Equals(encoding))
                    {
                        locatorInfo = locatorManager().get(locator.Clone(encodingVersion: encoding));
                    }
                    else
                    {
                        locatorInfo = locatorManager().get(locator);
                    }
                }

                property = $"{propertyPrefix}.Router";
                RouterPrx? router = GetPropertyAsProxy(property, RouterPrx.Factory);
                if (router != null)
                {
                    if (propertyPrefix.EndsWith(".Router", StringComparison.Ordinal))
                    {
                        Logger.warning($"`{property}={GetProperty(property)}': cannot set a router on a router; setting ignored");
                    }
                    else
                    {
                        routerInfo = routerManager().get(router);
                    }
                }

                property = $"{propertyPrefix}.CollocationOptimized";
                collocOptimized = (GetPropertyAsInt(property) ?? (collocOptimized ? 1 : 0)) > 0;

                property = $"{propertyPrefix}.ConnectionCached";
                cacheConnection = (GetPropertyAsInt(property) ?? (cacheConnection ? 1 : 0)) > 0;

                property = $"{propertyPrefix}.PreferSecure";
                preferSecure = (GetPropertyAsInt(property) ?? (preferSecure ? 1 : 0)) > 0;

                property = propertyPrefix + ".EndpointSelection";
                string? val = GetProperty(property);
                if (val != null)
                {
                    endpointSelection = Enum.Parse<EndpointSelectionType>(val);
                }

                property = $"{propertyPrefix}.LocatorCacheTimeout";
                val = GetProperty(property);
                if (val != null)
                {
                    locatorCacheTimeout = GetPropertyAsInt(property) ?? locatorCacheTimeout;
                    if (locatorCacheTimeout < -1)
                    {
                        locatorCacheTimeout = -1;
                        Logger.warning($"invalid value for {property} `{val}': defaulting to -1");
                    }
                }

                property = $"{propertyPrefix}.InvocationTimeout";
                val = GetProperty(property);
                if (val != null)
                {
                    invocationTimeout = GetPropertyAsInt(property) ?? invocationTimeout;
                    if (invocationTimeout < 1 && invocationTimeout != -1)
                    {
                        invocationTimeout = -1;
                        Logger.warning($"invalid value for {property} `{val}': defaulting to -1");
                    }
                }

                property = $"{propertyPrefix}.Context.";
                context = GetProperties(forPrefix: property).ToDictionary(e => e.Key.Substring(property.Length),
                                                                          e => e.Value);
            }

            //
            // Create new reference
            //
            return new RoutableReference(this,
                                         ident,
                                         facet,
                                         mode,
                                         secure,
                                         protocol,
                                         encoding,
                                         endpoints,
                                         adapterId,
                                         locatorInfo,
                                         routerInfo,
                                         collocOptimized,
                                         cacheConnection,
                                         preferSecure,
                                         endpointSelection,
                                         locatorCacheTimeout,
                                         invocationTimeout,
                                         context);
        }

        private const int StateActive = 0;
        private const int StateDestroyInProgress = 1;
        private const int StateDestroyed = 2;
        private int _state;

        private Ice.Logger _logger;
        private Instrumentation.CommunicatorObserver? _observer;
        private Action? _threadStart;
        private Action? _threadStop;
        private Action<Action, Connection?>? _dispatcher;
        private string[] _typeIdNamespaces = { "Ice.TypeId" };
        private Func<int, string>? _compactIdResolver;

        private readonly TraceLevels _traceLevels; // Immutable, not reset by destroy().
        private readonly DefaultsAndOverrides _defaultsAndOverrides; // Immutable, not reset by destroy().
        private readonly int _messageSizeMax; // Immutable, not reset by destroy().
        private readonly int _classGraphDepthMax; // Immutable, not reset by destroy().
        private readonly ToStringMode _toStringMode; // Immutable, not reset by destroy().
        private readonly int _cacheMessageBuffers; // Immutable, not reset by destroy().
        private readonly ACMConfig _clientACM; // Immutable, not reset by destroy().
        private readonly ACMConfig _serverACM; // Immutable, not reset by destroy().
        private readonly ImplicitContextI? _implicitContext; // Immutable
        private RouterManager? _routerManager;
        private LocatorManager? _locatorManager;
        private RequestHandlerFactory? _requestHandlerFactory;
        private OutgoingConnectionFactory? _outgoingConnectionFactory;
        private ObjectAdapterFactory? _objectAdapterFactory;
        private readonly int _protocolSupport;
        private readonly bool _preferIPv6;
        private readonly NetworkProxy? _networkProxy;
        private IceInternal.ThreadPool? _clientThreadPool;
        private IceInternal.ThreadPool? _serverThreadPool;
        private AsyncIOThread? _asyncIOThread;
        private EndpointHostResolver? _endpointHostResolver;
        private IceInternal.Timer? _timer;
        private RetryQueue? _retryQueue;
        private EndpointFactoryManager? _endpointFactoryManager;
        private PluginManager? _pluginManager;
        private readonly bool _adminEnabled = false;
        private ObjectAdapter? _adminAdapter;
        private readonly Dictionary<string, (object servant, Disp disp)> _adminFacets =
            new Dictionary<string, (object servant, Disp disp)>();
        private readonly HashSet<string> _adminFacetFilter = new HashSet<string>();
        private Identity? _adminIdentity;
        private readonly Dictionary<short, BufSizeWarnInfo> _setBufSizeWarn = new Dictionary<short, BufSizeWarnInfo>();
        private static bool _printProcessIdDone = false;
        private static bool _oneOffDone = false;
        private static readonly object _staticLock = new object();

        private readonly int[] _retryIntervals;

        private RouterPrx? _defaultRouter;
        private LocatorPrx? _defaultLocator;

        private static string[] _emptyArgs = Array.Empty<string>();
    }
}
