//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace IceInternal
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Text;
    using System.Threading;
    using System.Linq;

    public sealed class BufSizeWarnInfo
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

    public sealed class Instance
    {
        private class ObserverUpdaterI : Ice.Instrumentation.ObserverUpdater
        {
            public ObserverUpdaterI(Instance instance)
            {
                _instance = instance;
            }

            public void updateConnectionObservers()
            {
                _instance.updateConnectionObservers();
            }

            public void updateThreadObservers()
            {
                _instance.updateThreadObservers();
            }

            private Instance _instance;
        }

        public bool destroyed()
        {
            return _state == StateDestroyed;
        }

        public Ice.InitializationData initializationData()
        {
            //
            // No check for destruction. It must be possible to access the
            // initialization data after destruction.
            //
            // No mutex lock, immutable.
            //
            return _initData;
        }

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
            lock (this)
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
            lock (this)
            {
                if (_state == StateDestroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }

                Debug.Assert(_locatorManager != null);
                return _locatorManager;
            }
        }

        public ReferenceFactory referenceFactory()
        {
            lock (this)
            {
                if (_state == StateDestroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }

                Debug.Assert(_referenceFactory != null);
                return _referenceFactory;
            }
        }

        public RequestHandlerFactory requestHandlerFactory()
        {
            lock (this)
            {
                if (_state == StateDestroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }

                Debug.Assert(_requestHandlerFactory != null);
                return _requestHandlerFactory;
            }
        }

        public ProxyFactory proxyFactory()
        {
            lock (this)
            {
                if (_state == StateDestroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }

                Debug.Assert(_proxyFactory != null);
                return _proxyFactory;
            }
        }

        public OutgoingConnectionFactory outgoingConnectionFactory()
        {
            lock (this)
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

        public int protocolSupport()
        {
            return _protocolSupport;
        }

        public bool preferIPv6()
        {
            return _preferIPv6;
        }

        public NetworkProxy networkProxy()
        {
            return _networkProxy;
        }

        public ThreadPool clientThreadPool()
        {
            lock (this)
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
            lock (this)
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
                    int timeout = _initData.properties.getPropertyAsInt("Ice.ServerIdleTime");
                    _serverThreadPool = new ThreadPool(this, "Ice.ThreadPool.Server", timeout);
                }

                return _serverThreadPool;
            }
        }

        public AsyncIOThread
        asyncIOThread()
        {
            lock (this)
            {
                if (_state == StateDestroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }

                if (_asyncIOThread == null) // Lazy initialization.
                {
                    _asyncIOThread = new AsyncIOThread(this);
                }

                return _asyncIOThread;
            }
        }

        public EndpointHostResolver endpointHostResolver()
        {
            lock (this)
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
            lock (this)
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
            lock (this)
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
            lock (this)
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
            lock (this)
            {
                if (_state == StateDestroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
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

        public Ice.ImplicitContextI getImplicitContext()
        {
            return _implicitContext;
        }

        public Ice.ObjectPrx
        createAdmin(Ice.ObjectAdapter adminAdapter, Ice.Identity adminIdentity)
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

                _adminIdentity = adminIdentity;
                _adminAdapter = adminAdapter;
                addAllAdminFacets();
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

        public Ice.ObjectPrx
        getAdmin()
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

        public void
        addAdminFacet<T, Traits>(T servant, string facet) where Traits : struct, Ice.IInterfaceTraits<T>
        {
            Traits traits = default;
            Ice.Disp disp = (incoming, current) => traits.Dispatch(servant, incoming, current);
            addAdminFacet(servant, disp, facet);
        }

        public void
        addAdminFacet(object servant, Ice.Disp disp, string facet)
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

        public (object servant, Ice.Disp disp)
        removeAdminFacet(string facet)
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

        public (object servant, Ice.Disp disp)
        findAdminFacet(string facet)
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

        public Dictionary<string, (object servant, Ice.Disp disp)>
        findAllAdminFacets()
        {
            lock (this)
            {
                if (_state == StateDestroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }
                return new Dictionary<string, (object servant, Ice.Disp disp)>(_adminFacets);
            }
        }

        public void
        setDefaultLocator(Ice.LocatorPrx locator)
        {
            lock (this)
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
            lock (this)
            {
                if (_state == StateDestroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }

                _referenceFactory = _referenceFactory.setDefaultRouter(router);
            }
        }

        public void
        setLogger(Ice.Logger logger)
        {
            //
            // No locking, as it can only be called during plug-in loading
            //
            _initData.logger = logger;
        }

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
        // Return the C# class associated with this Slice type-id
        // Used for both non-local Slice classes and exceptions
        //
        public Type resolveClass(string id)
        {
            // First attempt corresponds to no cs:namespace metadata in the
            // enclosing top-level module
            //
            string className = typeToClass(id);
            Type c = AssemblyUtil.findType(className);

            //
            // If this fails, look for helper classes in the typeIdNamespaces namespace(s)
            //
            if (c == null && _initData.typeIdNamespaces != null)
            {
                foreach (var ns in _initData.typeIdNamespaces)
                {
                    Type helper = AssemblyUtil.findType(ns + "." + className);
                    if (helper != null)
                    {
                        try
                        {
                            c = helper.GetProperty("targetClass").PropertyType;
                            break; // foreach
                        }
                        catch (Exception)
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

        public string resolveCompactId(int compactId)
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
                    Type c = AssemblyUtil.findType(className);
                    if (c != null)
                    {
                        result = (string)c.GetField("typeId").GetValue(null);
                        break; // foreach
                    }
                }
                catch (Exception)
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
        // Only for use by Ice.CommunicatorI
        //
        public Instance(Ice.Communicator communicator, Ice.InitializationData initData)
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
                            new Ice.FileLoggerI(_initData.properties.getProperty("Ice.ProgramName"), logfile);
                    }
                    else if (Ice.Util.getProcessLogger() is Ice.LoggerI)
                    {
                        //
                        // Ice.ConsoleListener is enabled by default.
                        //
                        bool console = _initData.properties.getPropertyAsIntWithDefault("Ice.ConsoleListener", 1) > 0;
                        _initData.logger =
                            new Ice.TraceLoggerI(_initData.properties.getProperty("Ice.ProgramName"), console);
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

                _referenceFactory = new ReferenceFactory(this, communicator);

                _proxyFactory = new ProxyFactory(this);

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

                ProtocolInstance tcpInstance = new ProtocolInstance(this, Ice.TCPEndpointType.value, "tcp", false);
                _endpointFactoryManager.add(new TcpEndpointFactory(tcpInstance));

                ProtocolInstance udpInstance = new ProtocolInstance(this, Ice.UDPEndpointType.value, "udp", false);
                _endpointFactoryManager.add(new UdpEndpointFactory(udpInstance));

                ProtocolInstance wsInstance = new ProtocolInstance(this, Ice.WSEndpointType.value, "ws", false);
                _endpointFactoryManager.add(new WSEndpointFactory(wsInstance, Ice.TCPEndpointType.value));

                ProtocolInstance wssInstance = new ProtocolInstance(this, Ice.WSSEndpointType.value, "wss", true);
                _endpointFactoryManager.add(new WSEndpointFactory(wssInstance, Ice.SSLEndpointType.value));

                _pluginManager = new Ice.PluginManagerI(communicator);

                _outgoingConnectionFactory = new OutgoingConnectionFactory(communicator, this);

                _objectAdapterFactory = new ObjectAdapterFactory(this, communicator);

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

        public void finishSetup(ref string[] args, Ice.Communicator communicator)
        {
            //
            // Load plug-ins.
            //
            Debug.Assert(_serverThreadPool == null);
            Ice.PluginManagerI pluginManagerImpl = (Ice.PluginManagerI)_pluginManager;
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
                    Ice.ProcessTraits traits = default;
                    Ice.Process process = new ProcessI(communicator);
                    Ice.Disp disp = (current, incoming) => traits.Dispatch(process, current, incoming);
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
                PropertiesAdminI propsAdmin = null;
                if (_adminFacetFilter.Count == 0 || _adminFacetFilter.Contains(propertiesFacetName))
                {
                    propsAdmin = new PropertiesAdminI(this);
                    Ice.PropertiesAdminTraits traits = default;
                    Ice.Disp disp = (current, incoming) => traits.Dispatch(propsAdmin, current, incoming);
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
                    Ice.Disp disp = (current, incoming) => traits.Dispatch(metricsAdmin, current, incoming);
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
                _timer = new Timer(this, Util.stringToThreadPriority(
                                                initializationData().properties.getProperty("Ice.ThreadPriority")));
            }
            catch (Exception ex)
            {
                string s = "cannot create thread for timer:\n" + ex;
                _initData.logger.error(s);
                throw;
            }

            try
            {
                _endpointHostResolver = new EndpointHostResolver(this);
            }
            catch (Exception ex)
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
                    _proxyFactory.propertyToProxy("Ice.Default.Router"));
                if (r != null)
                {
                    _referenceFactory = _referenceFactory.setDefaultRouter(r);
                }
            }

            if (_referenceFactory.getDefaultLocator() == null)
            {
                Ice.LocatorPrx l = Ice.LocatorPrxHelper.uncheckedCast(
                    _proxyFactory.propertyToProxy("Ice.Default.Locator"));
                if (l != null)
                {
                    _referenceFactory = _referenceFactory.setDefaultLocator(l);
                }
            }

            //
            // Show process id if requested (but only once).
            //
            lock (this)
            {
                if (!_printProcessIdDone && _initData.properties.getPropertyAsInt("Ice.PrintProcessId") > 0)
                {
                    using (Process p = Process.GetCurrentProcess())
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

        //
        // Only for use by Ice.CommunicatorI
        //
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
                List<string> unusedProperties = ((Ice.PropertiesI)_initData.properties).getUnusedProperties();
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

                _referenceFactory = null;
                _requestHandlerFactory = null;
                _proxyFactory = null;
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
                Ice.FileLoggerI logger = _initData.logger as Ice.FileLoggerI;
                if (logger != null)
                {
                    logger.destroy();
                }
            }
        }

        public BufSizeWarnInfo getBufSizeWarn(short type)
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
            catch (Ice.CommunicatorDestroyedException)
            {
            }
        }

        internal void addAllAdminFacets()
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

        internal void setServerProcessProxy(Ice.ObjectAdapter adminAdapter, Ice.Identity adminIdentity)
        {
            Ice.ObjectPrx admin = adminAdapter.CreateProxy(adminIdentity);
            Ice.LocatorPrx locator = adminAdapter.GetLocator();
            string serverId = _initData.properties.getProperty("Ice.Admin.ServerId");

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
                        System.Text.StringBuilder s = new System.Text.StringBuilder();
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
                        System.Text.StringBuilder s = new System.Text.StringBuilder();
                        s.Append("couldn't register server `" + serverId + "' with the locator registry:\n" + ex);
                        _initData.logger.trace(_traceLevels.locationCat, s.ToString());
                    }
                    throw; // TODO: Shall we raise a special exception instead of a non obvious local exception?
                }

                if (_traceLevels.location >= 1)
                {
                    System.Text.StringBuilder s = new System.Text.StringBuilder();
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
                    throw new Ice.InitializationException("IPv6 only is not supported with SOCKS4 proxies");
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

        private const int StateActive = 0;
        private const int StateDestroyInProgress = 1;
        private const int StateDestroyed = 2;
        private int _state;
        private Ice.InitializationData _initData; // Immutable, not reset by destroy().
        private TraceLevels _traceLevels; // Immutable, not reset by destroy().
        private DefaultsAndOverrides _defaultsAndOverrides; // Immutable, not reset by destroy().
        private int _messageSizeMax; // Immutable, not reset by destroy().
        private int _classGraphDepthMax; // Immutable, not reset by destroy().
        private Ice.ToStringMode _toStringMode; // Immutable, not reset by destroy().
        private int _cacheMessageBuffers; // Immutable, not reset by destroy().
        private ACMConfig _clientACM; // Immutable, not reset by destroy().
        private ACMConfig _serverACM; // Immutable, not reset by destroy().
        private Ice.ImplicitContextI _implicitContext; // Immutable
        private RouterManager _routerManager;
        private LocatorManager _locatorManager;
        private ReferenceFactory _referenceFactory;
        private RequestHandlerFactory _requestHandlerFactory;
        private ProxyFactory _proxyFactory;
        private OutgoingConnectionFactory _outgoingConnectionFactory;
        private ObjectAdapterFactory _objectAdapterFactory;
        private int _protocolSupport;
        private bool _preferIPv6;
        private NetworkProxy _networkProxy;
        private ThreadPool _clientThreadPool;
        private ThreadPool _serverThreadPool;
        private AsyncIOThread _asyncIOThread;
        private EndpointHostResolver _endpointHostResolver;
        private Timer _timer;
        private RetryQueue _retryQueue;
        private EndpointFactoryManager _endpointFactoryManager;
        private Ice.PluginManager _pluginManager;
        private bool _adminEnabled = false;
        private Ice.ObjectAdapter _adminAdapter;
        private Dictionary<string, (object servant, Ice.Disp disp)> _adminFacets = new Dictionary<string, (object servant, Ice.Disp disp)>();
        private HashSet<string> _adminFacetFilter = new HashSet<string>();
        private Ice.Identity _adminIdentity;
        private Dictionary<short, BufSizeWarnInfo> _setBufSizeWarn = new Dictionary<short, BufSizeWarnInfo>();
        private static bool _printProcessIdDone = false;
        private static bool _oneOffDone = false;
        private static object _staticLock = new object();
    }
}
