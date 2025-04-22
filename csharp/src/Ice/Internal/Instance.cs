// Copyright (c) ZeroC, Inc.

using System.Diagnostics;
using System.Globalization;
using System.Text;

namespace Ice.Internal;

public sealed class BufSizeWarnInfo
{
    // Whether send size warning has been emitted
    public bool sndWarn;

    // The send size for which the warning was emitted
    public int sndSize;

    // Whether receive size warning has been emitted
    public bool rcvWarn;

    // The receive size for which the warning was emitted
    public int rcvSize;
}

public sealed class Instance
{
    private class ObserverUpdaterI : Ice.Instrumentation.ObserverUpdater
    {
        public ObserverUpdaterI(Instance instance) => _instance = instance;

        public void updateConnectionObservers() => _instance.updateConnectionObservers();

        public void updateThreadObservers() => _instance.updateThreadObservers();

        private readonly Instance _instance;
    }

    public Ice.InitializationData initializationData() =>
        //
        // No check for destruction. It must be possible to access the
        // initialization data after destruction.
        //
        // No mutex lock, immutable.
        //
        _initData;

    public TraceLevels traceLevels()
    {
        // No mutex lock, immutable.
        Debug.Assert(_traceLevels != null);
        return _traceLevels;
    }

    public DefaultsAndOverrides defaultsAndOverrides()
    {
        // No mutex lock, immutable.
        Debug.Assert(_defaultsAndOverrides != null);
        return _defaultsAndOverrides;
    }

    public RouterManager routerManager()
    {
        lock (_mutex)
        {
            if (_state == StateDestroyed)
            {
                throw new Ice.CommunicatorDestroyedException();
            }

            Debug.Assert(_routerManager != null);
            return _routerManager;
        }
    }

    public LocatorManager locatorManager()
    {
        lock (_mutex)
        {
            if (_state == StateDestroyed)
            {
                throw new Ice.CommunicatorDestroyedException();
            }

            Debug.Assert(_locatorManager != null);
            return _locatorManager;
        }
    }

    internal ReferenceFactory referenceFactory()
    {
        lock (_mutex)
        {
            if (_state == StateDestroyed)
            {
                throw new Ice.CommunicatorDestroyedException();
            }

            Debug.Assert(_referenceFactory != null);
            return _referenceFactory;
        }
    }

    internal OutgoingConnectionFactory outgoingConnectionFactory()
    {
        lock (_mutex)
        {
            if (_state == StateDestroyed)
            {
                throw new Ice.CommunicatorDestroyedException();
            }

            Debug.Assert(_outgoingConnectionFactory != null);
            return _outgoingConnectionFactory;
        }
    }

    public ObjectAdapterFactory objectAdapterFactory()
    {
        lock (_mutex)
        {
            if (_state == StateDestroyed)
            {
                throw new Ice.CommunicatorDestroyedException();
            }

            Debug.Assert(_objectAdapterFactory != null);
            return _objectAdapterFactory;
        }
    }

    public int protocolSupport() => _protocolSupport;

    public bool preferIPv6() => _preferIPv6;

    public NetworkProxy networkProxy() => _networkProxy;

    public ThreadPool clientThreadPool()
    {
        lock (_mutex)
        {
            if (_state == StateDestroyed)
            {
                throw new Ice.CommunicatorDestroyedException();
            }

            Debug.Assert(_clientThreadPool != null);
            return _clientThreadPool;
        }
    }

    public ThreadPool serverThreadPool()
    {
        lock (_mutex)
        {
            if (_state == StateDestroyed)
            {
                throw new Ice.CommunicatorDestroyedException();
            }

            if (_serverThreadPool == null) // Lazy initialization.
            {
                if (_state == StateDestroyInProgress)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }
                int timeout = _initData.properties.getIcePropertyAsInt("Ice.ServerIdleTime");
                _serverThreadPool = new ThreadPool(this, "Ice.ThreadPool.Server", timeout);
            }

            return _serverThreadPool;
        }
    }

    public EndpointHostResolver endpointHostResolver()
    {
        lock (_mutex)
        {
            if (_state == StateDestroyed)
            {
                throw new Ice.CommunicatorDestroyedException();
            }

            Debug.Assert(_endpointHostResolver != null);
            return _endpointHostResolver;
        }
    }

    public RetryQueue
    retryQueue()
    {
        lock (_mutex)
        {
            if (_state == StateDestroyed)
            {
                throw new Ice.CommunicatorDestroyedException();
            }

            Debug.Assert(_retryQueue != null);
            return _retryQueue;
        }
    }

    public Timer
    timer()
    {
        lock (_mutex)
        {
            if (_state == StateDestroyed)
            {
                throw new Ice.CommunicatorDestroyedException();
            }

            Debug.Assert(_timer != null);
            return _timer;
        }
    }

    public EndpointFactoryManager endpointFactoryManager()
    {
        lock (_mutex)
        {
            if (_state == StateDestroyed)
            {
                throw new Ice.CommunicatorDestroyedException();
            }

            Debug.Assert(_endpointFactoryManager != null);
            return _endpointFactoryManager;
        }
    }

    public Ice.PluginManager pluginManager()
    {
        lock (_mutex)
        {
            if (_state == StateDestroyed)
            {
                throw new Ice.CommunicatorDestroyedException();
            }

            Debug.Assert(_pluginManager != null);
            return _pluginManager;
        }
    }

    public int messageSizeMax() =>
        // No mutex lock, immutable.
        _messageSizeMax;

    public int batchAutoFlushSize() =>
        // No mutex lock, immutable.
        _batchAutoFlushSize;

    public int classGraphDepthMax() =>
        // No mutex lock, immutable.
        _classGraphDepthMax;

    public Ice.ToStringMode
    toStringMode() =>
        // No mutex lock, immutable
        _toStringMode;

    public int cacheMessageBuffers() =>
        // No mutex lock, immutable.
        _cacheMessageBuffers;

    public Ice.ImplicitContextI getImplicitContext() => _implicitContext;

    public Ice.ObjectPrx createAdmin(Ice.ObjectAdapter adminAdapter, Ice.Identity adminIdentity)
    {
        bool createAdapter = adminAdapter == null;

        lock (_mutex)
        {
            if (_state == StateDestroyed)
            {
                throw new Ice.CommunicatorDestroyedException();
            }

            if (adminIdentity == null || adminIdentity.name.Length == 0)
            {
                throw new ArgumentException("The admin identity is not valid", nameof(adminIdentity));
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
                if (_initData.properties.getIceProperty("Ice.Admin.Endpoints").Length > 0)
                {
                    adminAdapter = _objectAdapterFactory.createObjectAdapter(
                        "Ice.Admin",
                        router: null,
                        serverAuthenticationOptions: null);
                }
                else
                {
                    throw new Ice.InitializationException("Ice.Admin.Endpoints is not set");
                }
            }

            _adminIdentity = adminIdentity;
            _adminAdapter = adminAdapter;
            addAllAdminFacets();
        }

        if (createAdapter)
        {
            try
            {
                adminAdapter.activate();
            }
            catch (Ice.LocalException)
            {
                //
                // We cleanup _adminAdapter, however this error is not recoverable
                // (can't call again getAdmin() after fixing the problem)
                // since all the facets (servants) in the adapter are lost
                //
                adminAdapter.destroy();
                lock (_mutex)
                {
                    _adminAdapter = null;
                }
                throw;
            }
        }
        setServerProcessProxy(adminAdapter, adminIdentity);
        return adminAdapter.createProxy(adminIdentity);
    }

    public Ice.ObjectPrx
    getAdmin()
    {
        Ice.ObjectAdapter adminAdapter;
        Ice.Identity adminIdentity;

        lock (_mutex)
        {
            if (_state == StateDestroyed)
            {
                throw new Ice.CommunicatorDestroyedException();
            }

            if (_adminAdapter != null)
            {
                return _adminAdapter.createProxy(_adminIdentity);
            }
            else if (_adminEnabled)
            {
                if (_initData.properties.getIceProperty("Ice.Admin.Endpoints").Length > 0)
                {
                    adminAdapter = _objectAdapterFactory.createObjectAdapter(
                        "Ice.Admin",
                        router: null,
                        serverAuthenticationOptions: null);
                }
                else
                {
                    return null;
                }
                adminIdentity = new Ice.Identity(
                    "admin",
                    _initData.properties.getIceProperty("Ice.Admin.InstanceName"));
                if (adminIdentity.category.Length == 0)
                {
                    adminIdentity.category = System.Guid.NewGuid().ToString();
                }

                _adminIdentity = adminIdentity;
                _adminAdapter = adminAdapter;
                addAllAdminFacets();
                // continue below outside synchronization
            }
            else
            {
                return null;
            }
        }

        try
        {
            adminAdapter.activate();
        }
        catch (Ice.LocalException)
        {
            //
            // We cleanup _adminAdapter, however this error is not recoverable
            // (can't call again getAdmin() after fixing the problem)
            // since all the facets (servants) in the adapter are lost
            //
            adminAdapter.destroy();
            lock (_mutex)
            {
                _adminAdapter = null;
            }
            throw;
        }

        setServerProcessProxy(adminAdapter, adminIdentity);
        return adminAdapter.createProxy(adminIdentity);
    }

    public void
    addAdminFacet(Ice.Object servant, string facet)
    {
        lock (_mutex)
        {
            if (_state == StateDestroyed)
            {
                throw new Ice.CommunicatorDestroyedException();
            }

            if (_adminAdapter == null || (_adminFacetFilter.Count > 0 && !_adminFacetFilter.Contains(facet)))
            {
                if (_adminFacets.ContainsKey(facet))
                {
                    throw new Ice.AlreadyRegisteredException("facet", facet);
                }
                _adminFacets.Add(facet, servant);
            }
            else
            {
                _adminAdapter.addFacet(servant, _adminIdentity, facet);
            }
        }
    }

    public Ice.Object
    removeAdminFacet(string facet)
    {
        lock (_mutex)
        {
            if (_state == StateDestroyed)
            {
                throw new Ice.CommunicatorDestroyedException();
            }

            Ice.Object result = null;
            if (_adminAdapter == null || (_adminFacetFilter.Count > 0 && !_adminFacetFilter.Contains(facet)))
            {
                try
                {
                    result = _adminFacets[facet];
                }
                catch (KeyNotFoundException)
                {
                    throw new Ice.NotRegisteredException("facet", facet);
                }

                _adminFacets.Remove(facet);
            }
            else
            {
                result = _adminAdapter.removeFacet(_adminIdentity, facet);
            }
            return result;
        }
    }

    public Ice.Object
    findAdminFacet(string facet)
    {
        lock (_mutex)
        {
            if (_state == StateDestroyed)
            {
                throw new Ice.CommunicatorDestroyedException();
            }

            Ice.Object result = null;
            if (_adminAdapter == null || (_adminFacetFilter.Count > 0 && !_adminFacetFilter.Contains(facet)))
            {
                try
                {
                    result = _adminFacets[facet];
                }
                catch (KeyNotFoundException)
                {
                }
            }
            else
            {
                result = _adminAdapter.findFacet(_adminIdentity, facet);
            }
            return result;
        }
    }

    public Dictionary<string, Ice.Object>
    findAllAdminFacets()
    {
        lock (_mutex)
        {
            if (_state == StateDestroyed)
            {
                throw new Ice.CommunicatorDestroyedException();
            }

            if (_adminAdapter == null)
            {
                return new Dictionary<string, Ice.Object>(_adminFacets);
            }
            else
            {
                Dictionary<string, Ice.Object> result = _adminAdapter.findAllFacets(_adminIdentity);
                if (_adminFacets.Count > 0)
                {
                    foreach (KeyValuePair<string, Ice.Object> p in _adminFacets)
                    {
                        result.Add(p.Key, p.Value);
                    }
                }
                return result;
            }
        }
    }

    public void
    setDefaultLocator(Ice.LocatorPrx locator)
    {
        lock (_mutex)
        {
            if (_state == StateDestroyed)
            {
                throw new Ice.CommunicatorDestroyedException();
            }

            _referenceFactory = _referenceFactory.setDefaultLocator(locator);
        }
    }

    public void
    setDefaultRouter(Ice.RouterPrx router)
    {
        lock (_mutex)
        {
            if (_state == StateDestroyed)
            {
                throw new Ice.CommunicatorDestroyedException();
            }

            _referenceFactory = _referenceFactory.setDefaultRouter(router);
        }
    }

    public void
    setLogger(Ice.Logger logger) =>
        //
        // No locking, as it can only be called during plug-in loading
        //
        _initData.logger = logger;

    public void
    setThreadHook(System.Action threadStart, System.Action threadStop)
    {
        //
        // No locking, as it can only be called during plug-in loading
        //
        _initData.threadStart = threadStart;
        _initData.threadStop = threadStop;
    }

    //
    // Only for use by Ice.Communicator
    //
    internal void initialize(Ice.Communicator communicator, Ice.InitializationData initData)
    {
        _state = StateActive;
        _initData = initData;

        try
        {
            _initData.properties ??= new Ice.Properties();

            lock (_staticLock)
            {
                if (!_oneOffDone)
                {
                    string stdOut = _initData.properties.getIceProperty("Ice.StdOut");
                    string stdErr = _initData.properties.getIceProperty("Ice.StdErr");

                    System.IO.StreamWriter outStream = null;

                    if (stdOut.Length > 0)
                    {
                        try
                        {
                            outStream = System.IO.File.AppendText(stdOut);
                        }
                        catch (IOException ex)
                        {
                            throw new FileException($"Cannot append to '{stdOut}'", ex);
                        }
                        outStream.AutoFlush = true;
                        Console.Out.Close();
                        Console.SetOut(outStream);
                    }
                    if (stdErr.Length > 0)
                    {
                        if (stdErr.Equals(stdOut, StringComparison.Ordinal))
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
                            catch (IOException ex)
                            {
                                throw new FileException($"Cannot append to '{stdErr}'", ex);
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
                string logfile = _initData.properties.getIceProperty("Ice.LogFile");
                if (logfile.Length != 0)
                {
                    _initData.logger =
                        new Ice.FileLoggerI(_initData.properties.getIceProperty("Ice.ProgramName"), logfile);
                }
                else if (Ice.Util.getProcessLogger() is Ice.LoggerI)
                {
                    //
                    // Ice.ConsoleListener is enabled by default.
                    //
                    bool console = _initData.properties.getIcePropertyAsInt("Ice.ConsoleListener") > 0;
                    _initData.logger =
                        new Ice.TraceLoggerI(_initData.properties.getIceProperty("Ice.ProgramName"), console);
                }
                else
                {
                    _initData.logger = Ice.Util.getProcessLogger();
                }
            }

            _traceLevels = new TraceLevels(_initData.properties);

            _defaultsAndOverrides = new DefaultsAndOverrides(_initData.properties, _initData.logger);

            clientConnectionOptions = readConnectionOptions("Ice.Connection.Client");
            _serverConnectionOptions = readConnectionOptions("Ice.Connection.Server");

            int messageSizeMax = _initData.properties.getIcePropertyAsInt("Ice.MessageSizeMax");
            if (messageSizeMax < 1 || messageSizeMax > 0x7fffffff / 1024)
            {
                _messageSizeMax = 0x7fffffff;
            }
            else
            {
                _messageSizeMax = messageSizeMax * 1024; // Property is in kilobytes, _messageSizeMax in bytes
            }

            if (_initData.properties.getIceProperty("Ice.BatchAutoFlushSize").Length == 0 &&
               _initData.properties.getIceProperty("Ice.BatchAutoFlush").Length > 0)
            {
                if (_initData.properties.getIcePropertyAsInt("Ice.BatchAutoFlush") > 0)
                {
                    _batchAutoFlushSize = _messageSizeMax;
                }
            }
            else
            {
                int batchAutoFlushSize = _initData.properties.getIcePropertyAsInt("Ice.BatchAutoFlushSize");
                if (batchAutoFlushSize < 1)
                {
                    _batchAutoFlushSize = batchAutoFlushSize;
                }
                else if (batchAutoFlushSize > 0x7fffffff / 1024)
                {
                    _batchAutoFlushSize = 0x7fffffff;
                }
                else
                {
                    // Property is in kilobytes, _batchAutoFlushSize in bytes
                    _batchAutoFlushSize = batchAutoFlushSize * 1024;
                }
            }

            int classGraphDepthMax = _initData.properties.getIcePropertyAsInt("Ice.ClassGraphDepthMax");
            if (classGraphDepthMax < 1 || classGraphDepthMax > 0x7fffffff)
            {
                _classGraphDepthMax = 0x7fffffff;
            }
            else
            {
                _classGraphDepthMax = classGraphDepthMax;
            }

            // Update _initData.sliceLoader

            // We create a lazy Slice loader that searches assemblies the first time we unmarshal a class or
            // exception. At that time, all the assemblies should have been loaded.
            var lazySliceLoader = new LazySliceLoader(
                () => SliceLoader.fromAssemblies(AppDomain.CurrentDomain.GetAssemblies()));

            if (_initData.sliceLoader is null)
            {
                _initData.sliceLoader = lazySliceLoader;
            }
            else
            {
                var compositeSliceLoader = new CompositeSliceLoader();
                compositeSliceLoader.add(_initData.sliceLoader);
                compositeSliceLoader.add(lazySliceLoader);
                _initData.sliceLoader = compositeSliceLoader;
            }

            string toStringModeStr = _initData.properties.getIceProperty("Ice.ToStringMode");
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
                throw new Ice.InitializationException(
                    "The value for Ice.ToStringMode must be Unicode, ASCII or Compat");
            }

            _cacheMessageBuffers = _initData.properties.getIcePropertyAsInt("Ice.CacheMessageBuffers");

            _implicitContext = Ice.ImplicitContextI.create(_initData.properties.getIceProperty("Ice.ImplicitContext"));
            _routerManager = new RouterManager();

            _locatorManager = new LocatorManager(_initData.properties);

            _referenceFactory = new ReferenceFactory(this, communicator);

            bool isIPv6Supported = Network.isIPv6Supported();
            bool ipv4 = _initData.properties.getIcePropertyAsInt("Ice.IPv4") > 0;
            bool ipv6 = isIPv6Supported && _initData.properties.getIcePropertyAsInt("Ice.IPv6") > 0;
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
            _preferIPv6 = _initData.properties.getIcePropertyAsInt("Ice.PreferIPv6Address") > 0;

            _networkProxy = createNetworkProxy(_initData.properties, _protocolSupport);

            _sslEngine = new Ice.SSL.SSLEngine(communicator);

            _endpointFactoryManager = new EndpointFactoryManager(this);

            var tcpInstance = new ProtocolInstance(this, Ice.TCPEndpointType.value, "tcp", false);
            _endpointFactoryManager.add(new TcpEndpointFactory(tcpInstance));

            var udpInstance = new ProtocolInstance(this, Ice.UDPEndpointType.value, "udp", false);
            _endpointFactoryManager.add(new UdpEndpointFactory(udpInstance));

            var wsInstance = new ProtocolInstance(this, Ice.WSEndpointType.value, "ws", false);
            _endpointFactoryManager.add(new WSEndpointFactory(wsInstance, Ice.TCPEndpointType.value));

            var sslInstance = new Ice.SSL.Instance(_sslEngine, Ice.SSLEndpointType.value, "ssl");
            _endpointFactoryManager.add(new Ice.SSL.EndpointFactoryI(sslInstance, Ice.TCPEndpointType.value));

            var wssInstance = new ProtocolInstance(this, Ice.WSSEndpointType.value, "wss", true);
            _endpointFactoryManager.add(new WSEndpointFactory(wssInstance, Ice.SSLEndpointType.value));

            _pluginManager = new Ice.PluginManagerI(communicator);

            _outgoingConnectionFactory = new OutgoingConnectionFactory(this);

            _objectAdapterFactory = new ObjectAdapterFactory(this, communicator);

            _retryQueue = new RetryQueue(this);

            string[] retryValues = _initData.properties.getIcePropertyAsList("Ice.RetryIntervals");
            if (retryValues.Length == 0)
            {
                retryIntervals = [0];
            }
            else
            {
                retryIntervals = new int[retryValues.Length];

                for (int i = 0; i < retryValues.Length; i++)
                {
                    int v;

                    try
                    {
                        v = int.Parse(retryValues[i], CultureInfo.InvariantCulture);
                    }
                    catch (System.FormatException)
                    {
                        v = 0;
                    }

                    //
                    // If -1 is the first value, no retry and wait intervals.
                    //
                    if (i == 0 && v == -1)
                    {
                        retryIntervals = [];
                        break;
                    }

                    retryIntervals[i] = v > 0 ? v : 0;
                }
            }

            if (_initData.properties.getIcePropertyAsInt("Ice.PreloadAssemblies") > 0)
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

    public void finishSetup(ref string[] args, Ice.Communicator communicator)
    {
        //
        // Load plug-ins.
        //
        Debug.Assert(_serverThreadPool == null);
        var pluginManagerImpl = (Ice.PluginManagerI)_pluginManager;
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

        if (_initData.properties.getIceProperty("Ice.Admin.Enabled").Length == 0)
        {
            _adminEnabled = _initData.properties.getIceProperty("Ice.Admin.Endpoints").Length > 0;
        }
        else
        {
            _adminEnabled = _initData.properties.getIcePropertyAsInt("Ice.Admin.Enabled") > 0;
        }

        string[] facetFilter = _initData.properties.getIcePropertyAsList("Ice.Admin.Facets");
        if (facetFilter.Length > 0)
        {
            foreach (string s in facetFilter)
            {
                _adminFacetFilter.Add(s);
            }
        }

        if (_adminEnabled)
        {
            //
            // Process facet
            //
            string processFacetName = "Process";
            if (_adminFacetFilter.Count == 0 || _adminFacetFilter.Contains(processFacetName))
            {
                _adminFacets.Add(processFacetName, new ProcessI(communicator));
            }

            //
            // Logger facet
            //
            string loggerFacetName = "Logger";
            if (_adminFacetFilter.Count == 0 || _adminFacetFilter.Contains(loggerFacetName))
            {
                LoggerAdminLogger logger = new LoggerAdminLoggerI(_initData.properties, _initData.logger);
                setLogger(logger);
                _adminFacets.Add(loggerFacetName, logger.getFacet());
            }

            //
            // Properties facet
            //
            string propertiesFacetName = "Properties";
            PropertiesAdminI propsAdmin = null;
            if (_adminFacetFilter.Count == 0 || _adminFacetFilter.Contains(propertiesFacetName))
            {
                propsAdmin = new PropertiesAdminI(this);
                _adminFacets.Add(propertiesFacetName, propsAdmin);
            }

            //
            // Metrics facet
            //
            string metricsFacetName = "Metrics";
            if (_adminFacetFilter.Count == 0 || _adminFacetFilter.Contains(metricsFacetName))
            {
                var observer = new CommunicatorObserverI(_initData);
                _initData.observer = observer;
                _adminFacets.Add(metricsFacetName, observer.getFacet());

                //
                // Make sure the admin plugin receives property updates.
                //
                propsAdmin?.addUpdateCallback(observer.getFacet().updated);
            }
        }

        //
        // Set observer updater
        //
        _initData.observer?.setObserverUpdater(new ObserverUpdaterI(this));

        //
        // Create threads.
        //
        try
        {
            _timer = new Timer(this, Util.stringToThreadPriority(
                                            initializationData().properties.getIceProperty("Ice.ThreadPriority")));
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
        _clientThreadPool = new ThreadPool(this, "Ice.ThreadPool.Client", 0);

        //
        // The default router/locator may have been set during the loading of plugins.
        // Therefore we make sure it is not already set before checking the property.
        //
        if (_referenceFactory.getDefaultRouter() == null)
        {
            Ice.RouterPrx r = Ice.RouterPrxHelper.uncheckedCast(
                communicator.propertyToProxy("Ice.Default.Router"));
            if (r != null)
            {
                _referenceFactory = _referenceFactory.setDefaultRouter(r);
            }
        }

        if (_referenceFactory.getDefaultLocator() == null)
        {
            Ice.LocatorPrx l = Ice.LocatorPrxHelper.uncheckedCast(
                communicator.propertyToProxy("Ice.Default.Locator"));
            if (l != null)
            {
                _referenceFactory = _referenceFactory.setDefaultLocator(l);
            }
        }

        //
        // Show process id if requested (but only once).
        //
        lock (_mutex)
        {
            if (!_printProcessIdDone && _initData.properties.getIcePropertyAsInt("Ice.PrintProcessId") > 0)
            {
                using var p = System.Diagnostics.Process.GetCurrentProcess();
                Console.WriteLine(p.Id);
                _printProcessIdDone = true;
            }
        }

        // SslEngine initialization
        _sslEngine.initialize();

        //
        // Server thread pool initialization is lazy in serverThreadPool().
        //

        //
        // An application can set Ice.InitPlugins=0 if it wants to postpone
        // initialization until after it has interacted directly with the
        // plug-ins.
        //
        if (_initData.properties.getIcePropertyAsInt("Ice.InitPlugins") > 0)
        {
            pluginManagerImpl.initializePlugins();
        }

        //
        // This must be done last as this call creates the Ice.Admin object adapter
        // and eventually registers a process proxy with the Ice locator (allowing
        // remote clients to invoke on Ice.Admin facets as soon as it's registered).
        //
        if (_initData.properties.getIcePropertyAsInt("Ice.Admin.DelayCreation") <= 0)
        {
            getAdmin();
        }
    }

    //
    // Only for use by Ice.Communicator
    //
    public void destroy()
    {
        lock (_mutex)
        {
            //
            // If destroy is in progress, wait for it to be done. This
            // is necessary in case destroy() is called concurrently
            // by multiple threads.
            //
            while (_state == StateDestroyInProgress)
            {
                Monitor.Wait(_mutex);
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
        _objectAdapterFactory?.shutdown();

        _outgoingConnectionFactory?.destroy();

        _objectAdapterFactory?.destroy();

        _outgoingConnectionFactory?.waitUntilFinished();

        _retryQueue?.destroy(); // Must be called before destroying thread pools.

        _initData.observer?.setObserverUpdater(null);

        if (_initData.logger is LoggerAdminLogger loggerAdminLogger)
        {
            loggerAdminLogger.destroy();
        }

        //
        // Now, destroy the thread pools. This must be done *only* after
        // all the connections are finished (the connections destruction
        // can require invoking callbacks with the thread pools).
        //
        _serverThreadPool?.destroy();
        _clientThreadPool?.destroy();
        _endpointHostResolver?.destroy();

        //
        // Wait for all the threads to be finished.
        //
        _timer?.destroy();
        _clientThreadPool?.joinWithAllThreads();
        _serverThreadPool?.joinWithAllThreads();
        _endpointHostResolver?.joinWithThread();

        _routerManager?.destroy();

        _locatorManager?.destroy();

        if (_initData.properties.getIcePropertyAsInt("Ice.Warn.UnusedProperties") > 0)
        {
            List<string> unusedProperties = _initData.properties.getUnusedProperties();
            if (unusedProperties.Count != 0)
            {
                var message = new StringBuilder("The following properties were set but never read:");
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
        _pluginManager?.destroy();

        lock (_mutex)
        {
            _objectAdapterFactory = null;
            _outgoingConnectionFactory = null;
            _retryQueue = null;

            _serverThreadPool = null;
            _clientThreadPool = null;
            _endpointHostResolver = null;
            _timer = null;

            _referenceFactory = null;
            _routerManager = null;
            _locatorManager = null;
            _endpointFactoryManager = null;
            _pluginManager = null;

            _adminAdapter = null;
            _adminFacets.Clear();

            _state = StateDestroyed;
            Monitor.PulseAll(_mutex);
        }

        if (_initData.logger is FileLoggerI fileLogger)
        {
            fileLogger.destroy();
        }
    }

    public BufSizeWarnInfo getBufSizeWarn(short type)
    {
        lock (_setBufSizeWarn)
        {
            BufSizeWarnInfo info;
            if (!_setBufSizeWarn.TryGetValue(type, out BufSizeWarnInfo value))
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

    public void setSndBufSizeWarn(short type, int size)
    {
        lock (_setBufSizeWarn)
        {
            BufSizeWarnInfo info = getBufSizeWarn(type);
            info.sndWarn = true;
            info.sndSize = size;
            _setBufSizeWarn[type] = info;
        }
    }

    public void setRcvBufSizeWarn(short type, int size)
    {
        lock (_setBufSizeWarn)
        {
            BufSizeWarnInfo info = getBufSizeWarn(type);
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
        catch (Ice.CommunicatorDestroyedException)
        {
        }
    }

    internal void updateThreadObservers()
    {
        try
        {
            _clientThreadPool?.updateObservers();
            _serverThreadPool?.updateObservers();
            Debug.Assert(_objectAdapterFactory != null);
            _objectAdapterFactory.updateThreadObservers();
            _endpointHostResolver?.updateObserver();
            _timer?.updateObserver(_initData.observer);
        }
        catch (Ice.CommunicatorDestroyedException)
        {
        }
    }

    internal void addAllAdminFacets()
    {
        lock (_mutex)
        {
            var filteredFacets = new Dictionary<string, Ice.Object>();

            foreach (KeyValuePair<string, Ice.Object> entry in _adminFacets)
            {
                if (_adminFacetFilter.Count == 0 || _adminFacetFilter.Contains(entry.Key))
                {
                    _adminAdapter.addFacet(entry.Value, _adminIdentity, entry.Key);
                }
                else
                {
                    filteredFacets.Add(entry.Key, entry.Value);
                }
            }
            _adminFacets = filteredFacets;
        }
    }

    internal void setServerProcessProxy(Ice.ObjectAdapter adminAdapter, Ice.Identity adminIdentity)
    {
        Ice.ObjectPrx admin = adminAdapter.createProxy(adminIdentity);
        Ice.LocatorPrx locator = adminAdapter.getLocator();
        string serverId = _initData.properties.getIceProperty("Ice.Admin.ServerId");

        if (locator != null && serverId.Length > 0)
        {
            Ice.ProcessPrx process = Ice.ProcessPrxHelper.uncheckedCast(admin.ice_facet("Process"));
            try
            {
                //
                // Note that as soon as the process proxy is registered, the communicator might be
                // shutdown by a remote client and admin facets might start receiving calls.
                //
                locator.getRegistry().setServerProcessProxy(serverId, process);
            }
            catch (Ice.ServerNotFoundException)
            {
                if (_traceLevels.location >= 1)
                {
                    var s = new System.Text.StringBuilder();
                    s.Append("couldn't register server `" + serverId + "' with the locator registry:\n");
                    s.Append("the server is not known to the locator registry");
                    _initData.logger.trace(_traceLevels.locationCat, s.ToString());
                }

                throw new Ice.InitializationException("Locator knows nothing about server `" + serverId + "'");
            }
            catch (Ice.LocalException ex)
            {
                if (_traceLevels.location >= 1)
                {
                    var s = new System.Text.StringBuilder();
                    s.Append("couldn't register server `" + serverId + "' with the locator registry:\n" + ex);
                    _initData.logger.trace(_traceLevels.locationCat, s.ToString());
                }
                throw; // TODO: Shall we raise a special exception instead of a non obvious local exception?
            }

            if (_traceLevels.location >= 1)
            {
                var s = new System.Text.StringBuilder();
                s.Append("registered server `" + serverId + "' with the locator registry");
                _initData.logger.trace(_traceLevels.locationCat, s.ToString());
            }
        }
    }

    internal ConnectionOptions serverConnectionOptions(string adapterName)
    {
        Debug.Assert(adapterName.Length > 0);
        Properties properties = _initData.properties;
        string propertyPrefix = $"{adapterName}.Connection";

        return new(
            connectTimeout: TimeSpan.FromSeconds(properties.getPropertyAsIntWithDefault(
                $"{propertyPrefix}.ConnectTimeout",
                (int)_serverConnectionOptions.connectTimeout.TotalSeconds)),

            closeTimeout: TimeSpan.FromSeconds(properties.getPropertyAsIntWithDefault(
                $"{propertyPrefix}.CloseTimeout",
                (int)_serverConnectionOptions.closeTimeout.TotalSeconds)),

            idleTimeout: TimeSpan.FromSeconds(properties.getPropertyAsIntWithDefault(
                $"{propertyPrefix}.IdleTimeout",
                (int)_serverConnectionOptions.idleTimeout.TotalSeconds)),

            enableIdleCheck: properties.getPropertyAsIntWithDefault(
                $"{propertyPrefix}.EnableIdleCheck",
                _serverConnectionOptions.enableIdleCheck ? 1 : 0) > 0,

            inactivityTimeout: TimeSpan.FromSeconds(properties.getPropertyAsIntWithDefault(
                $"{propertyPrefix}.InactivityTimeout",
                (int)_serverConnectionOptions.inactivityTimeout.TotalSeconds)),

            maxDispatches: properties.getPropertyAsIntWithDefault(
                $"{propertyPrefix}.MaxDispatches",
                _serverConnectionOptions.maxDispatches));
    }

    internal SliceLoader sliceLoader => _initData.sliceLoader; // set in initialize

    internal ConnectionOptions clientConnectionOptions { get; private set; } = null!; // set in initialize

    internal int[] retryIntervals { get; private set; }

    private NetworkProxy createNetworkProxy(Ice.Properties props, int protocolSupport)
    {
        string proxyHost;

        proxyHost = props.getIceProperty("Ice.SOCKSProxyHost");
        if (proxyHost.Length > 0)
        {
            if (protocolSupport == Network.EnableIPv6)
            {
                throw new Ice.InitializationException("IPv6 only is not supported with SOCKS4 proxies");
            }
            int proxyPort = props.getIcePropertyAsInt("Ice.SOCKSProxyPort");
            return new SOCKSNetworkProxy(proxyHost, proxyPort);
        }

        proxyHost = props.getIceProperty("Ice.HTTPProxyHost");
        if (proxyHost.Length > 0)
        {
            return new HTTPNetworkProxy(proxyHost, props.getIcePropertyAsInt("Ice.HTTPProxyPort"));
        }

        return null;
    }

    private ConnectionOptions readConnectionOptions(string propertyPrefix)
    {
        Properties properties = _initData.properties;

        // The TimeSpan value can be <= 0. In this case, the timeout is considered infinite.
        return new(
            connectTimeout: TimeSpan.FromSeconds(properties.getIcePropertyAsInt($"{propertyPrefix}.ConnectTimeout")),
            closeTimeout: TimeSpan.FromSeconds(properties.getIcePropertyAsInt($"{propertyPrefix}.CloseTimeout")),
            idleTimeout: TimeSpan.FromSeconds(properties.getIcePropertyAsInt($"{propertyPrefix}.IdleTimeout")),
            enableIdleCheck: properties.getIcePropertyAsInt($"{propertyPrefix}.EnableIdleCheck") > 0,
            inactivityTimeout:
                TimeSpan.FromSeconds(properties.getIcePropertyAsInt($"{propertyPrefix}.InactivityTimeout")),
            maxDispatches: properties.getIcePropertyAsInt($"{propertyPrefix}.MaxDispatches"));
    }

    private static bool _printProcessIdDone;
    private static bool _oneOffDone;
    private static readonly object _staticLock = new object();

    private const int StateActive = 0;
    private const int StateDestroyInProgress = 1;
    private const int StateDestroyed = 2;

    private int _state;
    private Ice.InitializationData _initData; // Immutable, not reset by destroy().
    private TraceLevels _traceLevels; // Immutable, not reset by destroy().
    private DefaultsAndOverrides _defaultsAndOverrides; // Immutable, not reset by destroy().
    private int _messageSizeMax; // Immutable, not reset by destroy().
    private int _batchAutoFlushSize; // Immutable, not reset by destroy().
    private int _classGraphDepthMax; // Immutable, not reset by destroy().
    private Ice.ToStringMode _toStringMode; // Immutable, not reset by destroy().
    private int _cacheMessageBuffers; // Immutable, not reset by destroy().
    private Ice.ImplicitContextI _implicitContext; // Immutable
    private RouterManager _routerManager;
    private LocatorManager _locatorManager;
    private ReferenceFactory _referenceFactory;
    private OutgoingConnectionFactory _outgoingConnectionFactory;
    private ObjectAdapterFactory _objectAdapterFactory;
    private int _protocolSupport;
    private bool _preferIPv6;
    private NetworkProxy _networkProxy;
    private ThreadPool _clientThreadPool;
    private ThreadPool _serverThreadPool;
    private EndpointHostResolver _endpointHostResolver;
    private Timer _timer;
    private RetryQueue _retryQueue;
    private EndpointFactoryManager _endpointFactoryManager;
    private Ice.PluginManager _pluginManager;
    private bool _adminEnabled;
    private Ice.ObjectAdapter _adminAdapter;
    private Dictionary<string, Ice.Object> _adminFacets = new();
    private readonly HashSet<string> _adminFacetFilter = new();
    private Ice.Identity _adminIdentity;
    private readonly Dictionary<short, BufSizeWarnInfo> _setBufSizeWarn = new();
    private ConnectionOptions _serverConnectionOptions; // set in initialize
    private Ice.SSL.SSLEngine _sslEngine;
    private readonly object _mutex = new object();
}
