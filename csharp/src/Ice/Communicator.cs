//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Diagnostics;
using System.Collections.Generic;
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

    public sealed class Communicator : IDisposable
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

            private Communicator _communicator;
        }

        internal InitializationData initializationData()
        {
            //
            // No check for destruction. It must be possible to access the
            // initialization data after destruction.
            //
            // No mutex lock, immutable.
            //
            return _initData;
        }

        internal bool destroyed()
        {
            return _state == StateDestroyed;
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

            if (_initData.observer != null)
            {
                _initData.observer.setObserverUpdater(null);
            }

            {
                LoggerAdminLogger? logger = _initData.logger as LoggerAdminLogger;
                if (logger != null)
                {
                    logger.destroy();
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

            if (_initData.properties.getPropertyAsInt("Ice.Warn.UnusedProperties") > 0)
            {
                List<string> unusedProperties = _initData.properties.getUnusedProperties();
                if (unusedProperties.Count != 0)
                {
                    StringBuilder message = new StringBuilder("The following properties were set but never read:");
                    foreach (string s in unusedProperties)
                    {
                        message.Append("\n    ");
                        message.Append(s);
                    }
                    _initData.logger.warning(message.ToString());
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
                FileLoggerI? logger = _initData.logger as FileLoggerI;
                if (logger != null)
                {
                    logger.destroy();
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
        /// Convert a proxy to a set of proxy properties.
        /// </summary>
        /// <param name="proxy">The proxy.
        ///
        /// </param>
        /// <param name="property">The base property name.
        ///
        /// </param>
        /// <returns>The property set.</returns>
        public Dictionary<string, string> proxyToProperty(IObjectPrx proxy, string property)
        {
            if (proxy != null)
            {
                return proxy.IceReference.toProperty(property);
            }
            else
            {
                return new Dictionary<string, string>();
            }
        }

        /// <summary>
        /// Convert an identity into a string.
        /// </summary>
        /// <param name="ident">The identity to convert into a string.
        ///
        /// </param>
        /// <returns>The "stringified" identity.
        ///
        /// </returns>
        public string identityToString(Identity ident)
        {
            return Util.identityToString(ident, toStringMode());
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

            getProperties().setProperty(name + ".Endpoints", endpoints);
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
            Dictionary<string, string> properties = proxyToProperty(router, name + ".Router");
            foreach (KeyValuePair<string, string> entry in properties)
            {
                getProperties().setProperty(entry.Key, entry.Value);
            }

            return objectAdapterFactory().createObjectAdapter(name, router);
        }

        /// <summary>
        /// Get the properties for this communicator.
        /// </summary>
        /// <returns>This communicator's properties.
        ///
        /// </returns>
        public Properties getProperties()
        {
            return _initData.properties;
        }

        /// <summary>
        /// Get the logger for this communicator.
        /// </summary>
        /// <returns>This communicator's logger.
        ///
        /// </returns>
        public Logger getLogger()
        {
            return _initData.logger;
        }

        /// <summary>
        /// Get the observer resolver object for this communicator.
        /// </summary>
        /// <returns>This communicator's observer resolver object.</returns>
        public Instrumentation.CommunicatorObserver getObserver()
        {
            return _initData.observer;
        }

        /// <summary>
        /// Get the default router this communicator.
        /// </summary>
        /// <returns>The default router for this communicator.
        ///
        /// </returns>
        public RouterPrx getDefaultRouter()
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
        public LocatorPrx getDefaultLocator()
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
        public void setDefaultLocator(LocatorPrx locator)
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

        public int messageSizeMax()
        {
            // No mutex lock, immutable.
            return _messageSizeMax;
        }

        public int classGraphDepthMax()
        {
            // No mutex lock, immutable.
            return _classGraphDepthMax;
        }

        public Ice.ToStringMode
        toStringMode()
        {
            // No mutex lock, immutable
            return _toStringMode;
        }

        public int cacheMessageBuffers()
        {
            // No mutex lock, immutable.
            return _cacheMessageBuffers;
        }

        public ACMConfig clientACM()
        {
            // No mutex lock, immutable.
            return _clientACM;
        }

        public ACMConfig serverACM()
        {
            // No mutex lock, immutable.
            return _serverACM;
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
        public IObjectPrx CreateAdmin(ObjectAdapter adminAdapter, Identity adminIdentity)
        {
            bool createAdapter = (adminAdapter == null);

            lock (this)
            {
                if (_state == StateDestroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }

                if (adminIdentity == null || string.IsNullOrEmpty(adminIdentity.name))
                {
                    throw new Ice.IllegalIdentityException(adminIdentity);
                }

                if (_adminAdapter != null)
                {
                    throw new Ice.InitializationException("Admin already created");
                }

                if (!_adminEnabled)
                {
                    throw new Ice.InitializationException("Admin is disabled");
                }

                if (createAdapter)
                {
                    if (_initData.properties.getProperty("Ice.Admin.Endpoints").Length > 0)
                    {
                        adminAdapter = _objectAdapterFactory.createObjectAdapter("Ice.Admin", null);
                    }
                    else
                    {
                        throw new Ice.InitializationException("Ice.Admin.Endpoints is not set");
                    }
                }
                Debug.Assert(adminAdapter != null);

                _adminIdentity = adminIdentity;
                _adminAdapter = adminAdapter;
                AddAllAdminFacets();
            }

            if (createAdapter)
            {
                try
                {
                    adminAdapter.Activate();
                }
                catch (Ice.LocalException)
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
            }
            setServerProcessProxy(adminAdapter, adminIdentity);
            return adminAdapter.CreateProxy(adminIdentity);
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
        public IObjectPrx getAdmin()
        {
            Ice.ObjectAdapter adminAdapter;
            Ice.Identity adminIdentity;

            lock (this)
            {
                if (_state == StateDestroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }

                if (_adminAdapter != null)
                {
                    return _adminAdapter.CreateProxy(_adminIdentity);
                }
                else if (_adminEnabled)
                {
                    if (_initData.properties.getProperty("Ice.Admin.Endpoints").Length > 0)
                    {
                        adminAdapter = _objectAdapterFactory.createObjectAdapter("Ice.Admin", null);
                    }
                    else
                    {
                        return null;
                    }
                    adminIdentity = new Ice.Identity("admin", _initData.properties.getProperty("Ice.Admin.InstanceName"));
                    if (adminIdentity.category.Length == 0)
                    {
                        adminIdentity.category = System.Guid.NewGuid().ToString();
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
            catch (Ice.LocalException)
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
        public void addAdminFacet<T, Traits>(T servant, string facet) where Traits : struct, IInterfaceTraits<T>
        {
            Traits traits = default;
            Ice.Disp disp = (incoming, current) => traits.Dispatch(servant, incoming, current);
            addAdminFacet(servant, disp, facet);
        }

        public void addAdminFacet(object servant, Disp disp, string facet)
        {
            lock (this)
            {
                if (_state == StateDestroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }

                if (_adminFacetFilter.Count > 0 && !_adminFacetFilter.Contains(facet))
                {
                    throw new ArgumentException($"facet `{facet}' not allow by Ice.Admin.Facets configuration", facet);
                }

                if (_adminFacets.ContainsKey(facet))
                {
                    throw new Ice.AlreadyRegisteredException("facet", facet);
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
        public (object servant, Disp disp) removeAdminFacet(string facet)
        {
            lock (this)
            {
                if (_state == StateDestroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }

                (object servant, Ice.Disp disp) result = default;
                if (!_adminFacets.TryGetValue(facet, out result))
                {
                    throw new Ice.NotRegisteredException("facet", facet);
                }
                _adminFacets.Remove(facet);
                if (_adminAdapter != null)
                {
                    _adminAdapter.Remove(_adminIdentity, facet);
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
        public (object servant, Disp disp) findAdminFacet(string facet)
        {
            lock (this)
            {
                if (_state == StateDestroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }

                (object servant, Ice.Disp disp) result = default;
                try
                {
                    result = _adminFacets[facet];
                }
                catch (KeyNotFoundException)
                {
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
        public Dictionary<string, (object servant, Disp disp)> findAllAdminFacets()
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

        internal Communicator(InitializationData initData)
        {
            _state = StateActive;
            _initData = initData;

            try
            {
                if (_initData.properties == null)
                {
                    _initData.properties = Ice.Util.createProperties();
                }

                lock (_staticLock)
                {
                    if (!_oneOffDone)
                    {
                        string stdOut = _initData.properties.getProperty("Ice.StdOut");
                        string stdErr = _initData.properties.getProperty("Ice.StdErr");

                        System.IO.StreamWriter outStream = null;

                        if (stdOut.Length > 0)
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
                        if (stdErr.Length > 0)
                        {
                            if (stdErr.Equals(stdOut))
                            {
                                Console.SetError(outStream);
                            }
                            else
                            {
                                System.IO.StreamWriter errStream = null;
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

                if (_initData.logger == null)
                {
                    string logfile = _initData.properties.getProperty("Ice.LogFile");
                    if (logfile.Length != 0)
                    {
                        _initData.logger =
                            new FileLoggerI(_initData.properties.getProperty("Ice.ProgramName"), logfile);
                    }
                    else if (Ice.Util.getProcessLogger() is LoggerI)
                    {
                        //
                        // Ice.ConsoleListener is enabled by default.
                        //
                        bool console = _initData.properties.getPropertyAsIntWithDefault("Ice.ConsoleListener", 1) > 0;
                        _initData.logger =
                            new TraceLoggerI(_initData.properties.getProperty("Ice.ProgramName"), console);
                    }
                    else
                    {
                        _initData.logger = Ice.Util.getProcessLogger();
                    }
                }

                _traceLevels = new TraceLevels(_initData.properties);

                _defaultsAndOverrides = new DefaultsAndOverrides(_initData.properties, _initData.logger);

                _clientACM = new ACMConfig(_initData.properties,
                                           _initData.logger,
                                           "Ice.ACM.Client",
                                           new ACMConfig(_initData.properties, _initData.logger, "Ice.ACM",
                                                         new ACMConfig(false)));

                _serverACM = new ACMConfig(_initData.properties,
                                           _initData.logger,
                                           "Ice.ACM.Server",
                                           new ACMConfig(_initData.properties, _initData.logger, "Ice.ACM",
                                                         new ACMConfig(true)));

                {
                    const int defaultMessageSizeMax = 1024;
                    int num =
                        _initData.properties.getPropertyAsIntWithDefault("Ice.MessageSizeMax", defaultMessageSizeMax);
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
                    const int defaultValue = 100;
                    var num = _initData.properties.getPropertyAsIntWithDefault("Ice.ClassGraphDepthMax", defaultValue);
                    if (num < 1 || num > 0x7fffffff)
                    {
                        _classGraphDepthMax = 0x7fffffff;
                    }
                    else
                    {
                        _classGraphDepthMax = num;
                    }
                }

                string toStringModeStr = _initData.properties.getPropertyWithDefault("Ice.ToStringMode", "Unicode");
                if (toStringModeStr == "Unicode")
                {
                    _toStringMode = Ice.ToStringMode.Unicode;
                }
                else if (toStringModeStr == "ASCII")
                {
                    _toStringMode = Ice.ToStringMode.ASCII;
                }
                else if (toStringModeStr == "Compat")
                {
                    _toStringMode = Ice.ToStringMode.Compat;
                }
                else
                {
                    throw new Ice.InitializationException("The value for Ice.ToStringMode must be Unicode, ASCII or Compat");
                }

                _cacheMessageBuffers = _initData.properties.getPropertyAsIntWithDefault("Ice.CacheMessageBuffers", 2);

                _implicitContext = Ice.ImplicitContextI.create(_initData.properties.getProperty("Ice.ImplicitContext"));
                _routerManager = new RouterManager();

                _locatorManager = new LocatorManager(_initData.properties);

                string[] arr = _initData.properties.getPropertyAsList("Ice.RetryIntervals");

                if (arr.Length > 0)
                {
                    _retryIntervals = new int[arr.Length];

                    for (int i = 0; i < arr.Length; i++)
                    {
                        int v;

                        try
                        {
                            v = int.Parse(arr[i], CultureInfo.InvariantCulture);
                        }
                        catch (FormatException)
                        {
                            v = 0;
                        }

                        //
                        // If -1 is the first value, no retry and wait intervals.
                        //
                        if (i == 0 && v == -1)
                        {
                            _retryIntervals = System.Array.Empty<int>();
                            break;
                        }

                        _retryIntervals[i] = v > 0 ? v : 0;
                    }
                }
                else
                {
                    _retryIntervals = new int[1];
                    _retryIntervals[0] = 0;
                }

                _requestHandlerFactory = new RequestHandlerFactory(this);

                bool isIPv6Supported = Network.isIPv6Supported();
                bool ipv4 = _initData.properties.getPropertyAsIntWithDefault("Ice.IPv4", 1) > 0;
                bool ipv6 = _initData.properties.getPropertyAsIntWithDefault("Ice.IPv6", isIPv6Supported ? 1 : 0) > 0;
                if (!ipv4 && !ipv6)
                {
                    throw new Ice.InitializationException("Both IPV4 and IPv6 support cannot be disabled.");
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
                _preferIPv6 = _initData.properties.getPropertyAsInt("Ice.PreferIPv6Address") > 0;

                _networkProxy = createNetworkProxy(_initData.properties, _protocolSupport);

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

                if (_initData.properties.getPropertyAsIntWithDefault("Ice.PreloadAssemblies", 0) > 0)
                {
                    AssemblyUtil.preloadAssemblies();
                }
            }
            catch (Ice.LocalException)
            {
                destroy();
                throw;
            }
        }

        //
        // Certain initialization tasks need to be completed after the
        // constructor.
        //
        internal void finishSetup(ref string[] args)
        {
            try
            {
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

                if (_initData.properties.getProperty("Ice.Admin.Enabled").Length == 0)
                {
                    _adminEnabled = _initData.properties.getProperty("Ice.Admin.Endpoints").Length > 0;
                }
                else
                {
                    _adminEnabled = _initData.properties.getPropertyAsInt("Ice.Admin.Enabled") > 0;
                }

                _adminFacetFilter = new HashSet<string>(_initData.properties.getPropertyAsList("Ice.Admin.Facets").Distinct());

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
                        LoggerAdminLogger logger = new LoggerAdminLoggerI(_initData.properties, _initData.logger);
                        setLogger(logger);
                        Ice.LoggerAdminTraits traits = default;
                        Ice.LoggerAdmin servant = logger.getFacet();
                        Ice.Disp disp = (incoming, current) => traits.Dispatch(servant, incoming, current);
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
                        CommunicatorObserverI observer = new CommunicatorObserverI(_initData);
                        _initData.observer = observer;
                        IceMX.MetricsAdminTraits traits = default;
                        var metricsAdmin = observer.getFacet();
                        Disp disp = (current, incoming) => traits.Dispatch(metricsAdmin, current, incoming);
                        _adminFacets.Add(metricsFacetName, (metricsAdmin, disp));

                        //
                        // Make sure the admin plugin receives property updates.
                        //
                        if (propsAdmin != null)
                        {
                            propsAdmin.addUpdateCallback((Dictionary<string, string> updates) =>
                            {
                                observer.getFacet().updated(updates);
                            });
                        }
                    }
                }

                //
                // Set observer updater
                //
                if (_initData.observer != null)
                {
                    _initData.observer.setObserverUpdater(new ObserverUpdaterI(this));
                }

                //
                // Create threads.
                //
                try
                {
                    _timer = new IceInternal.Timer(this, IceInternal.Util.stringToThreadPriority(
                                                    _initData.properties.getProperty("Ice.ThreadPriority")));
                }
                catch (System.Exception ex)
                {
                    string s = "cannot create thread for timer:\n" + ex;
                    _initData.logger.error(s);
                    throw;
                }

                try
                {
                    _endpointHostResolver = new EndpointHostResolver(this);
                }
                catch (System.Exception ex)
                {
                    string s = "cannot create thread for endpoint host resolver:\n" + ex;
                    _initData.logger.error(s);
                    throw;
                }
                _clientThreadPool = new IceInternal.ThreadPool(this, "Ice.ThreadPool.Client", 0);

                //
                // The default router/locator may have been set during the loading of plugins.
                // Therefore we make sure it is not already set before checking the property.
                //
                if (getDefaultRouter() == null)
                {
                    if (!string.IsNullOrEmpty(_initData.properties.getProperty("Ice.Default.Router")))
                    {
                        setDefaultRouter(RouterPrx.ParseProperty("Ice.Default.Router", this));
                    }
                }

                if (getDefaultLocator() == null)
                {
                    if (!string.IsNullOrEmpty(_initData.properties.getProperty("Ice.Default.Locator")))
                    {
                        setDefaultLocator(LocatorPrx.ParseProperty("Ice.Default.Locator", this));
                    }
                }

                //
                // Show process id if requested (but only once).
                //
                lock (this)
                {
                    if (!_printProcessIdDone && _initData.properties.getPropertyAsInt("Ice.PrintProcessId") > 0)
                    {
                        using (var p = System.Diagnostics.Process.GetCurrentProcess())
                        {
                            Console.WriteLine(p.Id);
                        }
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
                if (_initData.properties.getPropertyAsIntWithDefault("Ice.InitPlugins", 1) > 0)
                {
                    pluginManagerImpl.initializePlugins();
                }

                //
                // This must be done last as this call creates the Ice.Admin object adapter
                // and eventually registers a process proxy with the Ice locator (allowing
                // remote clients to invoke on Ice.Admin facets as soon as it's registered).
                //
                if (_initData.properties.getPropertyAsIntWithDefault("Ice.Admin.DelayCreation", 0) <= 0)
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
                    throw new Ice.CommunicatorDestroyedException();
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
            _initData.logger = logger;
        }

        internal void
        setThreadHook(Action threadStart, Action threadStop)
        {
            //
            // No locking, as it can only be called during plug-in loading
            //
            _initData.threadStart = threadStart;
            _initData.threadStop = threadStop;
        }

        internal string resolveCompactId(int compactId)
        {
            string[] defaultVal = { "IceCompactId" };
            var compactIdNamespaces = new List<string>(defaultVal);

            if (_initData.typeIdNamespaces != null)
            {
                compactIdNamespaces.AddRange(_initData.typeIdNamespaces);
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
                throw new Ice.MarshalException("expected type id but received `" + id + "'");
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
            if (c == null && _initData.typeIdNamespaces != null)
            {
                foreach (var ns in _initData.typeIdNamespaces)
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

        internal int protocolSupport()
        {
            return _protocolSupport;
        }

        internal bool preferIPv6()
        {
            return _preferIPv6;
        }

        internal NetworkProxy networkProxy()
        {
            return _networkProxy;
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
                    int timeout = _initData.properties.getPropertyAsInt("Ice.ServerIdleTime");
                    _serverThreadPool = new IceInternal.ThreadPool(this, "Ice.ThreadPool.Server", timeout);
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
                    _timer.updateObserver(_initData.observer);
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
            IObjectPrx admin = adminAdapter.CreateProxy(adminIdentity);
            LocatorPrx locator = adminAdapter.GetLocator();
            string serverId = _initData.properties.getProperty("Ice.Admin.ServerId");

            if (locator != null && serverId.Length > 0)
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
                        _initData.logger.trace(_traceLevels.locationCat, s.ToString());
                    }

                    throw new InitializationException("Locator knows nothing about server `" + serverId + "'");
                }
                catch (LocalException ex)
                {
                    if (_traceLevels.location >= 1)
                    {
                        StringBuilder s = new StringBuilder();
                        s.Append("couldn't register server `" + serverId + "' with the locator registry:\n" + ex);
                        _initData.logger.trace(_traceLevels.locationCat, s.ToString());
                    }
                    throw; // TODO: Shall we raise a special exception instead of a non obvious local exception?
                }

                if (_traceLevels.location >= 1)
                {
                    StringBuilder s = new StringBuilder();
                    s.Append("registered server `" + serverId + "' with the locator registry");
                    _initData.logger.trace(_traceLevels.locationCat, s.ToString());
                }
            }
        }

        private NetworkProxy createNetworkProxy(Ice.Properties props, int protocolSupport)
        {
            string proxyHost;

            proxyHost = props.getProperty("Ice.SOCKSProxyHost");
            if (proxyHost.Length > 0)
            {
                if (protocolSupport == Network.EnableIPv6)
                {
                    throw new InitializationException("IPv6 only is not supported with SOCKS4 proxies");
                }
                int proxyPort = props.getPropertyAsIntWithDefault("Ice.SOCKSProxyPort", 1080);
                return new SOCKSNetworkProxy(proxyHost, proxyPort);
            }

            proxyHost = props.getProperty("Ice.HTTPProxyHost");
            if (proxyHost.Length > 0)
            {
                return new HTTPNetworkProxy(proxyHost, props.getPropertyAsIntWithDefault("Ice.HTTPProxyPort", 1080));
            }

            return null;
        }

        internal int CheckRetryAfterException(LocalException ex, Reference @ref, ref int cnt)
        {
            Ice.Logger logger = _initData.logger;

            if (@ref.getMode() == InvocationMode.BatchOneway || @ref.getMode() == InvocationMode.BatchDatagram)
            {
                Debug.Assert(false); // batch no longer implemented anyway
                throw ex;
            }

            if (ex is ObjectNotExistException)
            {
                ObjectNotExistException one = (ObjectNotExistException)ex;
                if (@ref.getRouterInfo() != null && one.operation.Equals("ice_add_proxy"))
                {
                    //
                    // If we have a router, an ObjectNotExistException with an
                    // operation name "ice_add_proxy" indicates to the client
                    // that the router isn't aware of the proxy (for example,
                    // because it was evicted by the router). In this case, we
                    // must *always* retry, so that the missing proxy is added
                    // to the router.
                    //

                    @ref.getRouterInfo().clearCache(@ref);

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
                        LocatorInfo li = @ref.getLocatorInfo();
                        if (li != null)
                        {
                            li.clearCache(@ref);
                        }
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
            if (ident.name.Length == 0 && ident.category.Length == 0)
            {
                return null;
            }

            return CreateReference(ident, facet, tmpl.getMode(), tmpl.getSecure(), tmpl.getProtocol(), tmpl.getEncoding(),
                          endpoints, null, null);
        }

        internal Reference
        CreateReference(Identity ident, string facet, Reference tmpl, string adapterId)
        {
            if (ident.name.Length == 0 && ident.category.Length == 0)
            {
                return null;
            }

            //
            // Create new reference
            //
            return CreateReference(ident, facet, tmpl.getMode(), tmpl.getSecure(), tmpl.getProtocol(), tmpl.getEncoding(),
                          null, adapterId, null);
        }

        internal Reference CreateReference(Identity ident, ConnectionI connection)
        {
            if (ident.name.Length == 0 && ident.category.Length == 0)
            {
                return null;
            }

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
            if (s.Length == 0)
            {
                return null;
            }

            const string delim = " \t\n\r";

            int beg;
            int end = 0;

            beg = IceUtilInternal.StringUtil.findFirstNotOf(s, delim, end);
            if (beg == -1)
            {
                ProxyParseException e = new ProxyParseException();
                e.str = "no non-whitespace characters found in `" + s + "'";
                throw e;
            }

            //
            // Extract the identity, which may be enclosed in single
            // or double quotation marks.
            //
            string idstr = null;
            end = IceUtilInternal.StringUtil.checkQuote(s, beg);
            if (end == -1)
            {
                ProxyParseException e = new ProxyParseException();
                e.str = "mismatched quotes around identity in `" + s + "'";
                throw e;
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
                ProxyParseException e = new ProxyParseException();
                e.str = "no identity in `" + s + "'";
                throw e;
            }

            //
            // Parsing the identity may raise IdentityParseException.
            //
            Identity ident = Ice.Util.stringToIdentity(idstr);

            if (ident.name.Length == 0)
            {
                //
                // An identity with an empty name and a non-empty
                // category is illegal.
                //
                if (ident.category.Length > 0)
                {
                    IllegalIdentityException e = new IllegalIdentityException();
                    e.id = ident;
                    throw e;
                }
                //
                // Treat a stringified proxy containing two double
                // quotes ("") the same as an empty string, i.e.,
                // a null proxy, but only if nothing follows the
                // quotes.
                //
                else if (IceUtilInternal.StringUtil.findFirstNotOf(s, delim, end) != -1)
                {
                    ProxyParseException e = new ProxyParseException();
                    e.str = "invalid characters after identity in `" + s + "'";
                    throw e;
                }
                else
                {
                    return null;
                }
            }

            string facet = "";
            InvocationMode mode = InvocationMode.Twoway;
            bool secure = false;
            EncodingVersion encoding = _defaultsAndOverrides.defaultEncoding;
            ProtocolVersion protocol = Ice.Util.Protocol_1_0;
            string adapter = "";

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
                    ProxyParseException e = new ProxyParseException();
                    e.str = "expected a proxy option but found `" + option + "' in `" + s + "'";
                    throw e;
                }

                //
                // Check for the presence of an option argument. The
                // argument may be enclosed in single or double
                // quotation marks.
                //
                string argument = null;
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
                            ProxyParseException e = new ProxyParseException();
                            e.str = "mismatched quotes around value for " + option + " option in `" + s + "'";
                            throw e;
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
                                ProxyParseException e = new ProxyParseException();
                                e.str = "no argument provided for -f option in `" + s + "'";
                                throw e;
                            }

                            try
                            {
                                facet = IceUtilInternal.StringUtil.unescapeString(argument, 0, argument.Length, "");
                            }
                            catch (ArgumentException argEx)
                            {
                                ProxyParseException e = new ProxyParseException();
                                e.str = "invalid facet in `" + s + "': " + argEx.Message;
                                throw e;
                            }
                            break;
                        }

                    case 't':
                        {
                            if (argument != null)
                            {
                                ProxyParseException e = new ProxyParseException();
                                e.str = "unexpected argument `" + argument + "' provided for -t option in `" + s + "'";
                                throw e;
                            }
                            mode = InvocationMode.Twoway;
                            break;
                        }

                    case 'o':
                        {
                            if (argument != null)
                            {
                                ProxyParseException e = new ProxyParseException();
                                e.str = "unexpected argument `" + argument + "' provided for -o option in `" + s + "'";
                                throw e;
                            }
                            mode = InvocationMode.Oneway;
                            break;
                        }

                    case 'O':
                        {
                            if (argument != null)
                            {
                                ProxyParseException e = new ProxyParseException();
                                e.str = "unexpected argument `" + argument + "' provided for -O option in `" + s + "'";
                                throw e;
                            }
                            mode = InvocationMode.BatchOneway;
                            break;
                        }

                    case 'd':
                        {
                            if (argument != null)
                            {
                                ProxyParseException e = new ProxyParseException();
                                e.str = "unexpected argument `" + argument + "' provided for -d option in `" + s + "'";
                                throw e;
                            }
                            mode = InvocationMode.Datagram;
                            break;
                        }

                    case 'D':
                        {
                            if (argument != null)
                            {
                                ProxyParseException e = new ProxyParseException();
                                e.str = "unexpected argument `" + argument + "' provided for -D option in `" + s + "'";
                                throw e;
                            }
                            mode = InvocationMode.BatchDatagram;
                            break;
                        }

                    case 's':
                        {
                            if (argument != null)
                            {
                                ProxyParseException e = new ProxyParseException();
                                e.str = "unexpected argument `" + argument + "' provided for -s option in `" + s + "'";
                                throw e;
                            }
                            secure = true;
                            break;
                        }

                    case 'e':
                        {
                            if (argument == null)
                            {
                                throw new ProxyParseException("no argument provided for -e option `" + s + "'");
                            }

                            try
                            {
                                encoding = Ice.Util.stringToEncodingVersion(argument);
                            }
                            catch (VersionParseException e)
                            {
                                throw new ProxyParseException("invalid encoding version `" + argument + "' in `" + s +
                                                                  "':\n" + e.str);
                            }
                            break;
                        }

                    case 'p':
                        {
                            if (argument == null)
                            {
                                throw new ProxyParseException("no argument provided for -p option `" + s + "'");
                            }

                            try
                            {
                                protocol = Ice.Util.stringToProtocolVersion(argument);
                            }
                            catch (VersionParseException e)
                            {
                                throw new ProxyParseException("invalid protocol version `" + argument + "' in `" + s +
                                                                  "':\n" + e.str);
                            }
                            break;
                        }

                    default:
                        {
                            ProxyParseException e = new ProxyParseException();
                            e.str = "unknown option `" + option + "' in `" + s + "'";
                            throw e;
                        }
                }
            }

            if (beg == -1)
            {
                return CreateReference(ident, facet, mode, secure, protocol, encoding, null, null, propertyPrefix);
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
                    EndpointParseException e2 = new EndpointParseException();
                    e2.str = "invalid endpoint `" + unknownEndpoints[0] + "' in `" + s + "'";
                    throw e2;
                }
                else if (unknownEndpoints.Count != 0 &&
                         _initData.properties.getPropertyAsIntWithDefault("Ice.Warn.Endpoints", 1) > 0)
                {
                    StringBuilder msg = new StringBuilder("Proxy contains unknown endpoints:");
                    int sz = unknownEndpoints.Count;
                    for (int idx = 0; idx < sz; ++idx)
                    {
                        msg.Append(" `");
                        msg.Append(unknownEndpoints[idx]);
                        msg.Append("'");
                    }
                    _initData.logger.warning(msg.ToString());
                }

                EndpointI[] ep = endpoints.ToArray();
                return CreateReference(ident, facet, mode, secure, protocol, encoding, ep, null, propertyPrefix);
            }
            else if (s[beg] == '@')
            {
                beg = IceUtilInternal.StringUtil.findFirstNotOf(s, delim, beg + 1);
                if (beg == -1)
                {
                    ProxyParseException e = new ProxyParseException();
                    e.str = "missing adapter id in `" + s + "'";
                    throw e;
                }

                string adapterstr = null;
                end = IceUtilInternal.StringUtil.checkQuote(s, beg);
                if (end == -1)
                {
                    ProxyParseException e = new ProxyParseException();
                    e.str = "mismatched quotes around adapter id in `" + s + "'";
                    throw e;
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
                    ProxyParseException e = new Ice.ProxyParseException();
                    e.str = "invalid trailing characters after `" + s.Substring(0, end + 1) + "' in `" + s + "'";
                    throw e;
                }

                try
                {
                    adapter = IceUtilInternal.StringUtil.unescapeString(adapterstr, 0, adapterstr.Length, "");
                }
                catch (ArgumentException argEx)
                {
                    ProxyParseException e = new Ice.ProxyParseException();
                    e.str = "invalid adapter id in `" + s + "': " + argEx.Message;
                    throw e;
                }
                if (adapter.Length == 0)
                {
                    ProxyParseException e = new Ice.ProxyParseException();
                    e.str = "empty adapter id in `" + s + "'";
                    throw e;
                }
                return CreateReference(ident, facet, mode, secure, protocol, encoding, null, adapter, propertyPrefix);
            }

            Ice.ProxyParseException ex = new Ice.ProxyParseException();
            ex.str = "malformed proxy `" + s + "'";
            throw ex;
        }

        public Reference CreateReference(Identity ident, InputStream s)
        {
            //
            // Don't read the identity here. Operations calling this
            // constructor read the identity, and pass it as a parameter.
            //

            if (ident.name.Length == 0 && ident.category.Length == 0)
            {
                return null;
            }

            //
            // For compatibility with the old FacetPath.
            //
            string[] facetPath = s.readStringSeq();
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

            int mode = s.readByte();
            if (mode < 0 || mode > (int)InvocationMode.Last)
            {
                throw new ProxyUnmarshalException();
            }

            bool secure = s.readBool();

            ProtocolVersion protocol;
            EncodingVersion encoding;
            if (!s.getEncoding().Equals(Ice.Util.Encoding_1_0))
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

            EndpointI[] endpoints = null;
            string adapterId = "";

            int sz = s.readSize();
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
                adapterId = s.readString();
            }

            return CreateReference(ident, facet, (InvocationMode)mode, secure, protocol, encoding, endpoints, adapterId, null);
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
            Dictionary<string, string> props
                = _initData.properties.getPropertiesForPrefix(prefix + ".");
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
                _initData.logger.warning(message.ToString());
            }
        }

        private Reference CreateReference(Identity ident,
                                 string facet,
                                 InvocationMode mode,
                                 bool secure,
                                 ProtocolVersion protocol,
                                 EncodingVersion encoding,
                                 EndpointI[] endpoints,
                                 string adapterId,
                                 string propertyPrefix)
        {
            //
            // Default local proxy options.
            //
            LocatorInfo locatorInfo = null;
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
            RouterInfo routerInfo = routerManager().get(_defaultRouter);
            bool collocOptimized = _defaultsAndOverrides.defaultCollocationOptimization;
            bool cacheConnection = true;
            bool preferSecure = _defaultsAndOverrides.defaultPreferSecure;
            EndpointSelectionType endpointSelection = _defaultsAndOverrides.defaultEndpointSelection;
            int locatorCacheTimeout = _defaultsAndOverrides.defaultLocatorCacheTimeout;
            int invocationTimeout = _defaultsAndOverrides.defaultInvocationTimeout;
            Dictionary<string, string> context = null;

            //
            // Override the defaults with the proxy properties if a property prefix is defined.
            //
            if (propertyPrefix != null && propertyPrefix.Length > 0)
            {
                Ice.Properties properties = _initData.properties;

                //
                // Warn about unknown properties.
                //
                if (properties.getPropertyAsIntWithDefault("Ice.Warn.UnknownProperties", 1) > 0)
                {
                    checkForUnknownProperties(propertyPrefix);
                }

                string property = propertyPrefix + ".Locator";
                if (!string.IsNullOrEmpty(properties.getProperty(property)))
                {
                    LocatorPrx locator = LocatorPrx.ParseProperty(property, this);
                    if (!locator.IceReference.getEncoding().Equals(encoding))
                    {
                        locatorInfo = locatorManager().get(locator.Clone(encodingVersion: encoding));
                    }
                    else
                    {
                        locatorInfo = locatorManager().get(locator);
                    }
                }

                property = propertyPrefix + ".Router";
                if (!string.IsNullOrEmpty(properties.getProperty(property)))
                {
                    RouterPrx router = RouterPrx.ParseProperty(property, this);
                    if (propertyPrefix.EndsWith(".Router", StringComparison.Ordinal))
                    {
                        string s = "`" + property + "=" + properties.getProperty(property) +
                            "': cannot set a router on a router; setting ignored";
                        _initData.logger.warning(s);
                    }
                    else
                    {
                        routerInfo = routerManager().get(router);
                    }
                }

                property = propertyPrefix + ".CollocationOptimized";
                collocOptimized = properties.getPropertyAsIntWithDefault(property, collocOptimized ? 1 : 0) > 0;

                property = propertyPrefix + ".ConnectionCached";
                cacheConnection = properties.getPropertyAsIntWithDefault(property, cacheConnection ? 1 : 0) > 0;

                property = propertyPrefix + ".PreferSecure";
                preferSecure = properties.getPropertyAsIntWithDefault(property, preferSecure ? 1 : 0) > 0;

                property = propertyPrefix + ".EndpointSelection";
                if (properties.getProperty(property).Length > 0)
                {
                    string type = properties.getProperty(property);
                    if (type.Equals("Random"))
                    {
                        endpointSelection = EndpointSelectionType.Random;
                    }
                    else if (type.Equals("Ordered"))
                    {
                        endpointSelection = EndpointSelectionType.Ordered;
                    }
                    else
                    {
                        throw new EndpointSelectionTypeParseException(
                            $"illegal value `{type}'; expected `Random' or `Ordered'");
                    }
                }

                property = propertyPrefix + ".LocatorCacheTimeout";
                string val = properties.getProperty(property);
                if (val.Length > 0)
                {
                    locatorCacheTimeout = properties.getPropertyAsIntWithDefault(property, locatorCacheTimeout);
                    if (locatorCacheTimeout < -1)
                    {
                        locatorCacheTimeout = -1;

                        StringBuilder msg = new StringBuilder("invalid value for ");
                        msg.Append(property);
                        msg.Append(" `");
                        msg.Append(properties.getProperty(property));
                        msg.Append("': defaulting to -1");
                        _initData.logger.warning(msg.ToString());
                    }
                }

                property = propertyPrefix + ".InvocationTimeout";
                val = properties.getProperty(property);
                if (val.Length > 0)
                {
                    invocationTimeout = properties.getPropertyAsIntWithDefault(property, invocationTimeout);
                    if (invocationTimeout < 1 && invocationTimeout != -1)
                    {
                        invocationTimeout = -1;

                        StringBuilder msg = new StringBuilder("invalid value for ");
                        msg.Append(property);
                        msg.Append(" `");
                        msg.Append(properties.getProperty(property));
                        msg.Append("': defaulting to -1");
                        _initData.logger.warning(msg.ToString());
                    }
                }

                property = propertyPrefix + ".Context.";
                Dictionary<string, string> contexts = properties.getPropertiesForPrefix(property);
                if (contexts.Count != 0)
                {
                    context = new Dictionary<string, string>();
                    foreach (KeyValuePair<string, string> e in contexts)
                    {
                        context.Add(e.Key.Substring(property.Length), e.Value);
                    }
                }
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
        private InitializationData _initData; // Immutable, not reset by destroy().
        private TraceLevels _traceLevels; // Immutable, not reset by destroy().
        private DefaultsAndOverrides _defaultsAndOverrides; // Immutable, not reset by destroy().
        private int _messageSizeMax; // Immutable, not reset by destroy().
        private int _classGraphDepthMax; // Immutable, not reset by destroy().
        private ToStringMode _toStringMode; // Immutable, not reset by destroy().
        private int _cacheMessageBuffers; // Immutable, not reset by destroy().
        private ACMConfig _clientACM; // Immutable, not reset by destroy().
        private ACMConfig _serverACM; // Immutable, not reset by destroy().
        private ImplicitContextI _implicitContext; // Immutable
        private RouterManager _routerManager;
        private LocatorManager _locatorManager;
        private RequestHandlerFactory _requestHandlerFactory;
        private OutgoingConnectionFactory _outgoingConnectionFactory;
        private ObjectAdapterFactory _objectAdapterFactory;
        private int _protocolSupport;
        private bool _preferIPv6;
        private NetworkProxy _networkProxy;
        private IceInternal.ThreadPool _clientThreadPool;
        private IceInternal.ThreadPool _serverThreadPool;
        private AsyncIOThread _asyncIOThread;
        private EndpointHostResolver _endpointHostResolver;
        private IceInternal.Timer _timer;
        private RetryQueue _retryQueue;
        private EndpointFactoryManager _endpointFactoryManager;
        private PluginManager _pluginManager;
        private bool _adminEnabled = false;
        private ObjectAdapter _adminAdapter;
        private Dictionary<string, (object servant, Disp disp)> _adminFacets = new Dictionary<string, (object servant, Disp disp)>();
        private HashSet<string> _adminFacetFilter = new HashSet<string>();
        private Identity _adminIdentity;
        private Dictionary<short, BufSizeWarnInfo> _setBufSizeWarn = new Dictionary<short, BufSizeWarnInfo>();
        private static bool _printProcessIdDone = false;
        private static bool _oneOffDone = false;
        private static object _staticLock = new object();

        private int[] _retryIntervals;

        private RouterPrx _defaultRouter;
        private LocatorPrx _defaultLocator;
    }
}
