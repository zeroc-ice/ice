// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{
    using System.Collections;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Text;
    using System.Threading;
    using System;

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
            lock(this)
            {
                if(_state == StateDestroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }

                Debug.Assert(_routerManager != null);
                return _routerManager;
            }
        }

        public LocatorManager locatorManager()
        {
            lock(this)
            {
                if(_state == StateDestroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }

                Debug.Assert(_locatorManager != null);
                return _locatorManager;
            }
        }

        public ReferenceFactory referenceFactory()
        {
            lock(this)
            {
                if(_state == StateDestroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }

                Debug.Assert(_referenceFactory != null);
                return _referenceFactory;
            }
        }

        public RequestHandlerFactory requestHandlerFactory()
        {
            lock(this)
            {
                if(_state == StateDestroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }

                Debug.Assert(_requestHandlerFactory != null);
                return _requestHandlerFactory;
            }
        }

        public ProxyFactory proxyFactory()
        {
            lock(this)
            {
                if(_state == StateDestroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }

                Debug.Assert(_proxyFactory != null);
                return _proxyFactory;
            }
        }

        public OutgoingConnectionFactory outgoingConnectionFactory()
        {
            lock(this)
            {
                if(_state == StateDestroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }

                Debug.Assert(_outgoingConnectionFactory != null);
                return _outgoingConnectionFactory;
            }
        }

        public ObjectAdapterFactory objectAdapterFactory()
        {
            lock(this)
            {
                if(_state == StateDestroyed)
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
            lock(this)
            {
                if(_state == StateDestroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }

                Debug.Assert(_clientThreadPool != null);
                return _clientThreadPool;
            }
        }

        public ThreadPool serverThreadPool()
        {
            lock(this)
            {
                if(_state == StateDestroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }

                if(_serverThreadPool == null) // Lazy initialization.
                {
                    if(_state == StateDestroyInProgress)
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
            lock(this)
            {
                if(_state == StateDestroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }

                if(_asyncIOThread == null) // Lazy initialization.
                {
                    _asyncIOThread = new AsyncIOThread(this);
                }

                return _asyncIOThread;
            }
        }

        public EndpointHostResolver endpointHostResolver()
        {
            lock(this)
            {
                if(_state == StateDestroyed)
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
            lock(this)
            {
                if(_state == StateDestroyed)
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
            lock(this)
            {
                if(_state == StateDestroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }

                Debug.Assert(_timer != null);
                return _timer;
            }
        }

        public EndpointFactoryManager endpointFactoryManager()
        {
            lock(this)
            {
                if(_state == StateDestroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }

                Debug.Assert(_endpointFactoryManager != null);
                return _endpointFactoryManager;
            }
        }

        public Ice.PluginManager pluginManager()
        {
            lock(this)
            {
                if(_state == StateDestroyed)
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

        public int batchAutoFlushSize()
        {
            // No mutex lock, immutable.
            return _batchAutoFlushSize;
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

            lock(this)
            {
                if(_state == StateDestroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }

                if(adminIdentity == null || string.IsNullOrEmpty(adminIdentity.name))
                {
                    throw new Ice.IllegalIdentityException(adminIdentity);
                }

                if(_adminAdapter != null)
                {
                    throw new Ice.InitializationException("Admin already created");
                }

                if(!_adminEnabled)
                {
                    throw new Ice.InitializationException("Admin is disabled");
                }

                if(createAdapter)
                {
                    if(_initData.properties.getProperty("Ice.Admin.Endpoints").Length > 0)
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

            if(createAdapter)
            {
                try
                {
                    adminAdapter.activate();
                }
                catch(Ice.LocalException)
                {
                    //
                    // We cleanup _adminAdapter, however this error is not recoverable
                    // (can't call again getAdmin() after fixing the problem)
                    // since all the facets (servants) in the adapter are lost
                    //
                    adminAdapter.destroy();
                    lock(this)
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

            lock(this)
            {
                if(_state == StateDestroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }

                if(_adminAdapter != null)
                {
                    return _adminAdapter.createProxy(_adminIdentity);
                }
                else if(_adminEnabled)
                {
                    if(_initData.properties.getProperty("Ice.Admin.Endpoints").Length > 0)
                    {
                        adminAdapter = _objectAdapterFactory.createObjectAdapter("Ice.Admin", null);
                    }
                    else
                    {
                        return null;
                    }
                    adminIdentity = new Ice.Identity("admin", _initData.properties.getProperty("Ice.Admin.InstanceName"));
                    if(adminIdentity.category.Length == 0)
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
            catch(Ice.LocalException)
            {
                //
                // We cleanup _adminAdapter, however this error is not recoverable
                // (can't call again getAdmin() after fixing the problem)
                // since all the facets (servants) in the adapter are lost
                //
                adminAdapter.destroy();
                lock(this)
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
            lock(this)
            {
                if(_state == StateDestroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }

                if(_adminAdapter == null || (_adminFacetFilter.Count > 0 && !_adminFacetFilter.Contains(facet)))
                {
                    if(_adminFacets.ContainsKey(facet))
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
            lock(this)
            {
                if(_state == StateDestroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }

                Ice.Object result = null;
                if(_adminAdapter == null || (_adminFacetFilter.Count > 0 && !_adminFacetFilter.Contains(facet)))
                {
                    try
                    {
                        result = _adminFacets[facet];
                    }
                    catch(KeyNotFoundException)
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
            lock(this)
            {
                if(_state == StateDestroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }

                Ice.Object result = null;
                if(_adminAdapter == null || (_adminFacetFilter.Count > 0 && !_adminFacetFilter.Contains(facet)))
                {
                    try
                    {
                        result = _adminFacets[facet];
                    }
                    catch(KeyNotFoundException)
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
            lock(this)
            {
                if(_state == StateDestroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }

                if(_adminAdapter == null)
                {
                    return new Dictionary<string, Ice.Object>(_adminFacets);
                }
                else
                {
                    Dictionary<string, Ice.Object> result = _adminAdapter.findAllFacets(_adminIdentity);
                    if(_adminFacets.Count > 0)
                    {
                        foreach(KeyValuePair<string, Ice.Object> p in _adminFacets)
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
            lock(this)
            {
                if(_state == StateDestroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }

                _referenceFactory = _referenceFactory.setDefaultLocator(locator);
            }
        }

        public void
        setDefaultRouter(Ice.RouterPrx router)
        {
            lock(this)
            {
                if(_state == StateDestroyed)
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
        setThreadHook(Ice.ThreadNotification threadHook)
        {
            //
            // No locking, as it can only be called during plug-in loading
            //
            _initData.threadHook = threadHook;
        }

        public Type resolveClass(string id)
        {
            Type c = AssemblyUtil.findType(this, typeToClass(id));

            //
            // Ensure the class is instantiable.
            //
            if(c != null && !c.IsAbstract && !c.IsInterface)
            {
                return c;
            }

            return null;
        }

        public string resolveCompactId(int compactId)
        {
            String className = "IceCompactId.TypeId_" + compactId;
            try
            {
                Type c = AssemblyUtil.findType(this, className);
                if(c != null)
                {
                    return (string)c.GetField("typeId").GetValue(null);
                }
            }
            catch(Exception)
            {
            }
            return "";
        }

        private static string typeToClass(string id)
        {
            if(!id.StartsWith("::", StringComparison.Ordinal))
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
                if(_initData.properties == null)
                {
                    _initData.properties = Ice.Util.createProperties();
                }

                lock(_staticLock)
                {
                    if(!_oneOffDone)
                    {
                        string stdOut = _initData.properties.getProperty("Ice.StdOut");
                        string stdErr = _initData.properties.getProperty("Ice.StdErr");

                        System.IO.StreamWriter outStream = null;

                        if(stdOut.Length > 0)
                        {
                            try
                            {
                                outStream = System.IO.File.AppendText(stdOut);
                            }
                            catch(System.IO.IOException ex)
                            {
                                Ice.FileException fe = new Ice.FileException(ex);
                                fe.path = stdOut;
                                throw fe;
                            }
                            outStream.AutoFlush = true;
                            System.Console.Out.Close();
                            System.Console.SetOut(outStream);
                        }
                        if(stdErr.Length > 0)
                        {
                            if(stdErr.Equals(stdOut))
                            {
                                System.Console.SetError(outStream);
                            }
                            else
                            {
                                System.IO.StreamWriter errStream = null;
                                try
                                {
                                    errStream = System.IO.File.AppendText(stdErr);
                                }
                                catch(System.IO.IOException ex)
                                {
                                    Ice.FileException fe = new Ice.FileException(ex);
                                    fe.path = stdErr;
                                    throw fe;
                                }
                                errStream.AutoFlush = true;
                                System.Console.Error.Close();
                                System.Console.SetError(errStream);
                            }
                        }

                        _oneOffDone = true;
                    }
                }

                if(_initData.logger == null)
                {
                    string logfile = _initData.properties.getProperty("Ice.LogFile");
                    if(_initData.properties.getPropertyAsInt("Ice.UseSyslog") > 0 &&
                       AssemblyUtil.platform_ != AssemblyUtil.Platform.Windows)
                    {
                        if(logfile.Length != 0)
                        {
                            throw new Ice.InitializationException("Ice.LogFile and Ice.UseSyslog cannot both be set.");
                        }
                        _initData.logger = new Ice.SysLoggerI(_initData.properties.getProperty("Ice.ProgramName"),
                            _initData.properties.getPropertyWithDefault("Ice.SyslogFacility", "LOG_USER"));
                    }
                    else if(logfile.Length != 0)
                    {

                        _initData.logger =
                            new Ice.FileLoggerI(_initData.properties.getProperty("Ice.ProgramName"), logfile);
                    }
                    else if(Ice.Util.getProcessLogger() is Ice.LoggerI)
                    {
                        //
                        // Ice.ConsoleListener is enabled by default.
                        //
                        bool console =
                            _initData.properties.getPropertyAsIntWithDefault("Ice.ConsoleListener", 1) > 0;
                        _initData.logger =
                            new Ice.TraceLoggerI(_initData.properties.getProperty("Ice.ProgramName"), console);
                    }

                    if(Ice.Util.getProcessLogger() is Ice.LoggerI)
                    {
                        _initData.logger =
                            new Ice.ConsoleLoggerI(_initData.properties.getProperty("Ice.ProgramName"));
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
                    if(num < 1 || num > 0x7fffffff / 1024)
                    {
                        _messageSizeMax = 0x7fffffff;
                    }
                    else
                    {
                        _messageSizeMax = num * 1024; // Property is in kilobytes, _messageSizeMax in bytes
                    }
                }

                if(_initData.properties.getProperty("Ice.BatchAutoFlushSize").Length == 0 &&
                   _initData.properties.getProperty("Ice.BatchAutoFlush").Length > 0)
                {
                    if(_initData.properties.getPropertyAsInt("Ice.BatchAutoFlush") > 0)
                    {
                        _batchAutoFlushSize = _messageSizeMax;
                    }
                }
                else
                {
                    int num = _initData.properties.getPropertyAsIntWithDefault("Ice.BatchAutoFlushSize", 1024); // 1MB
                    if(num < 1)
                    {
                        _batchAutoFlushSize = num;
                    }
                    else if(num > 0x7fffffff / 1024)
                    {
                        _batchAutoFlushSize = 0x7fffffff;
                    }
                    else
                    {
                        _batchAutoFlushSize = num * 1024; // Property is in kilobytes, _batchAutoFlushSize in bytes
                    }
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
                if(!ipv4 && !ipv6)
                {
                    throw new Ice.InitializationException("Both IPV4 and IPv6 support cannot be disabled.");
                }
                else if(ipv4 && ipv6)
                {
                    _protocolSupport = Network.EnableBoth;
                }
                else if(ipv4)
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
                _pluginManager = new Ice.PluginManagerI(communicator);

                if(_initData.valueFactoryManager == null)
                {
                    _initData.valueFactoryManager = new ValueFactoryManagerI();
                }

                _outgoingConnectionFactory = new OutgoingConnectionFactory(communicator, this);

                _objectAdapterFactory = new ObjectAdapterFactory(this, communicator);

                _retryQueue = new RetryQueue(this);
            }
            catch(Ice.LocalException)
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
            // Add WS and WSS endpoint factories if TCP/SSL factories are installed.
            //
            EndpointFactory tcpFactory = _endpointFactoryManager.get(Ice.TCPEndpointType.value);
            if(tcpFactory != null)
            {
                ProtocolInstance instance = new ProtocolInstance(this, Ice.WSEndpointType.value, "ws", false);
                _endpointFactoryManager.add(new WSEndpointFactory(instance, tcpFactory.clone(instance, null)));
            }
            EndpointFactory sslFactory = _endpointFactoryManager.get(Ice.SSLEndpointType.value);
            if(sslFactory != null)
            {
                ProtocolInstance instance = new ProtocolInstance(this, Ice.WSSEndpointType.value, "wss", true);
                _endpointFactoryManager.add(new WSEndpointFactory(instance, sslFactory.clone(instance, null)));
            }

            //
            // Create Admin facets, if enabled.
            //
            // Note that any logger-dependent admin facet must be created after we load all plugins,
            // since one of these plugins can be a Logger plugin that sets a new logger during loading
            //

            if(_initData.properties.getProperty("Ice.Admin.Enabled").Length == 0)
            {
                _adminEnabled = _initData.properties.getProperty("Ice.Admin.Endpoints").Length > 0;
            }
            else
            {
                _adminEnabled = _initData.properties.getPropertyAsInt("Ice.Admin.Enabled") > 0;
            }

            string[] facetFilter = _initData.properties.getPropertyAsList("Ice.Admin.Facets");
            if(facetFilter.Length > 0)
            {
                foreach(string s in facetFilter)
                {
                    _adminFacetFilter.Add(s);
                }
            }

            if(_adminEnabled)
            {
                //
                // Process facet
                //
                string processFacetName = "Process";
                if(_adminFacetFilter.Count == 0 || _adminFacetFilter.Contains(processFacetName))
                {
                    _adminFacets.Add(processFacetName, new ProcessI(communicator));
                }

                //
                // Logger facet
                //
                string loggerFacetName = "Logger";
                if(_adminFacetFilter.Count == 0 || _adminFacetFilter.Contains(loggerFacetName))
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
                if(_adminFacetFilter.Count == 0 || _adminFacetFilter.Contains(propertiesFacetName))
                {
                     propsAdmin= new PropertiesAdminI(this);
                    _adminFacets.Add(propertiesFacetName, propsAdmin);
                }

                //
                // Metrics facet
                //
                string metricsFacetName = "Metrics";
                if(_adminFacetFilter.Count == 0 || _adminFacetFilter.Contains(metricsFacetName))
                {
                    CommunicatorObserverI observer = new CommunicatorObserverI(_initData);
                    _initData.observer = observer;
                    _adminFacets.Add(metricsFacetName, observer.getFacet());

                    //
                    // Make sure the admin plugin receives property updates.
                    //
                    if(propsAdmin != null)
                    {
                        propsAdmin.addUpdateCallback(observer.getFacet());
                    }
                }
            }

            //
            // Set observer updater
            //
            if(_initData.observer != null)
            {
                _initData.observer.setObserverUpdater(new ObserverUpdaterI(this));
            }

            //
            // Create threads.
            //
            try
            {
                if(initializationData().properties.getProperty("Ice.ThreadPriority").Length > 0)
                {
                    ThreadPriority priority = IceInternal.Util.stringToThreadPriority(
                                                initializationData().properties.getProperty("Ice.ThreadPriority"));
                    _timer = new Timer(this, priority);
                }
                else
                {
                    _timer = new Timer(this);
                }
            }
            catch(System.Exception ex)
            {
                string s = "cannot create thread for timer:\n" + ex;
                _initData.logger.error(s);
                throw;
            }

            try
            {
                _endpointHostResolver = new EndpointHostResolver(this);
            }
            catch(System.Exception ex)
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
            if(_referenceFactory.getDefaultRouter() == null)
            {
                Ice.RouterPrx r = Ice.RouterPrxHelper.uncheckedCast(
                    _proxyFactory.propertyToProxy("Ice.Default.Router"));
                if(r != null)
                {
                    _referenceFactory = _referenceFactory.setDefaultRouter(r);
                }
            }

            if(_referenceFactory.getDefaultLocator() == null)
            {
                Ice.LocatorPrx l = Ice.LocatorPrxHelper.uncheckedCast(
                    _proxyFactory.propertyToProxy("Ice.Default.Locator"));
                if(l != null)
                {
                    _referenceFactory = _referenceFactory.setDefaultLocator(l);
                }
            }

            //
            // Show process id if requested (but only once).
            //
            lock(this)
            {
                if(!_printProcessIdDone && _initData.properties.getPropertyAsInt("Ice.PrintProcessId") > 0)
                {
                    using(Process p = Process.GetCurrentProcess())
                    {
                        System.Console.WriteLine(p.Id);
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
            if(_initData.properties.getPropertyAsIntWithDefault("Ice.InitPlugins", 1) > 0)
            {
                pluginManagerImpl.initializePlugins();
            }

            //
            // This must be done last as this call creates the Ice.Admin object adapter
            // and eventually registers a process proxy with the Ice locator (allowing
            // remote clients to invoke on Ice.Admin facets as soon as it's registered).
            //
            if(_initData.properties.getPropertyAsIntWithDefault("Ice.Admin.DelayCreation", 0) <= 0)
            {
                getAdmin();
            }
        }

        //
        // Only for use by Ice.CommunicatorI
        //
        public void destroy()
        {
            lock(this)
            {
                //
                // If destroy is in progress, wait for it to be done. This
                // is necessary in case destroy() is called concurrently
                // by multiple threads.
                //
                while(_state == StateDestroyInProgress)
                {
                    Monitor.Wait(this);
                }

                if(_state == StateDestroyed)
                {
                    return;
                }
                _state = StateDestroyInProgress;
            }

            //
            // Shutdown and destroy all the incoming and outgoing Ice
            // connections and wait for the connections to be finished.
            //
            if(_objectAdapterFactory != null)
            {
                _objectAdapterFactory.shutdown();
            }

            if(_outgoingConnectionFactory != null)
            {
                _outgoingConnectionFactory.destroy();
            }

            if(_objectAdapterFactory != null)
            {
                _objectAdapterFactory.destroy();
            }

            if(_outgoingConnectionFactory != null)
            {
                _outgoingConnectionFactory.waitUntilFinished();
            }

            if(_retryQueue != null)
            {
                _retryQueue.destroy(); // Must be called before destroying thread pools.
            }

            if(_initData.observer != null)
            {
                _initData.observer.setObserverUpdater(null);
            }

            LoggerAdminLogger logger = _initData.logger as LoggerAdminLogger;
            if(logger != null)
            {
                logger.destroy();
            }

            //
            // Now, destroy the thread pools. This must be done *only* after
            // all the connections are finished (the connections destruction
            // can require invoking callbacks with the thread pools).
            //
            if(_serverThreadPool != null)
            {
                _serverThreadPool.destroy();
            }
            if(_clientThreadPool != null)
            {
                _clientThreadPool.destroy();
            }
            if(_asyncIOThread != null)
            {
                _asyncIOThread.destroy();
            }
            if(_endpointHostResolver != null)
            {
                _endpointHostResolver.destroy();
            }

            //
            // Wait for all the threads to be finished.
            //
            if(_timer != null)
            {
                _timer.destroy();
            }
            if(_clientThreadPool != null)
            {
                _clientThreadPool.joinWithAllThreads();
            }
            if(_serverThreadPool != null)
            {
                _serverThreadPool.joinWithAllThreads();
            }
            if(_asyncIOThread != null)
            {
                _asyncIOThread.joinWithThread();
            }
            if(_endpointHostResolver != null)
            {
                _endpointHostResolver.joinWithThread();
            }

            foreach(Ice.ObjectFactory factory in _objectFactoryMap.Values)
            {
// Disable Obsolete warning/error
#pragma warning disable 612, 618
                factory.destroy();
#pragma warning restore 612, 618
            }
            _objectFactoryMap.Clear();

            if(_routerManager != null)
            {
                _routerManager.destroy();
            }

            if(_locatorManager != null)
            {
                _locatorManager.destroy();
            }

            if(_endpointFactoryManager != null)
            {
                _endpointFactoryManager.destroy();
            }

            if(_initData.properties.getPropertyAsInt("Ice.Warn.UnusedProperties") > 0)
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
            if(_pluginManager != null)
            {
                _pluginManager.destroy();
            }

            lock(this)
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
        }

        public BufSizeWarnInfo getBufSizeWarn(short type)
        {
            lock(_setBufSizeWarn)
            {
                BufSizeWarnInfo info;
                if(!_setBufSizeWarn.ContainsKey(type))
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
            lock(_setBufSizeWarn)
            {
                BufSizeWarnInfo info = getBufSizeWarn(type);
                info.sndWarn = true;
                info.sndSize = size;
                _setBufSizeWarn[type] = info;
            }
        }

        public void setRcvBufSizeWarn(short type, int size)
        {
            lock(_setBufSizeWarn)
            {
                BufSizeWarnInfo info = getBufSizeWarn(type);
                info.rcvWarn = true;
                info.rcvSize = size;
                _setBufSizeWarn[type] = info;
            }
        }

        public void addObjectFactory(Ice.ObjectFactory factory, string id)
        {
            lock(this)
            {
                //
                // Create a ValueFactory wrapper around the given ObjectFactory and register the wrapper
                // with the value factory manager. This may raise AlreadyRegisteredException.
                //
// Disable Obsolete warning/error
#pragma warning disable 612, 618
                _initData.valueFactoryManager.add((string type) => { return factory.create(type); }, id);
#pragma warning restore 612, 618
                _objectFactoryMap.Add(id, factory);
            }
        }

        public Ice.ObjectFactory findObjectFactory(string id)
        {
            lock(this)
            {
                Ice.ObjectFactory factory = null;
                _objectFactoryMap.TryGetValue(id, out factory);
                return factory;
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
            catch(Ice.CommunicatorDestroyedException)
            {
            }
        }

        internal void updateThreadObservers()
        {
            try
            {
                if(_clientThreadPool != null)
                {
                    _clientThreadPool.updateObservers();
                }
                if(_serverThreadPool != null)
                {
                    _serverThreadPool.updateObservers();
                }
                Debug.Assert(_objectAdapterFactory != null);
                _objectAdapterFactory.updateThreadObservers();
                if(_endpointHostResolver != null)
                {
                    _endpointHostResolver.updateObserver();
                }
                if(_asyncIOThread != null)
                {
                    _asyncIOThread.updateObserver();
                }
                if(_timer != null)
                {
                    _timer.updateObserver(_initData.observer);
                }
            }
            catch(Ice.CommunicatorDestroyedException)
            {
            }
        }

        internal void addAllAdminFacets()
        {
            lock(this)
            {
                Dictionary<string, Ice.Object> filteredFacets = new Dictionary<string, Ice.Object>();

                foreach(KeyValuePair<string, Ice.Object> entry in _adminFacets)
                {
                    if(_adminFacetFilter.Count == 0 || _adminFacetFilter.Contains(entry.Key))
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
            string serverId = _initData.properties.getProperty("Ice.Admin.ServerId");

            if(locator != null && serverId.Length > 0)
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
                catch(Ice.ServerNotFoundException)
                {
                    if(_traceLevels.location >= 1)
                    {
                        System.Text.StringBuilder s = new System.Text.StringBuilder();
                        s.Append("couldn't register server `" + serverId + "' with the locator registry:\n");
                        s.Append("the server is not known to the locator registry");
                        _initData.logger.trace(_traceLevels.locationCat, s.ToString());
                    }

                    throw new Ice.InitializationException("Locator knows nothing about server `" + serverId + "'");
                }
                catch(Ice.LocalException ex)
                {
                    if(_traceLevels.location >= 1)
                    {
                        System.Text.StringBuilder s = new System.Text.StringBuilder();
                        s.Append("couldn't register server `" + serverId + "' with the locator registry:\n" + ex);
                        _initData.logger.trace(_traceLevels.locationCat, s.ToString());
                    }
                    throw; // TODO: Shall we raise a special exception instead of a non obvious local exception?
                }

                if(_traceLevels.location >= 1)
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
            if(proxyHost.Length > 0)
            {
                if(protocolSupport == Network.EnableIPv6)
                {
                    throw new Ice.InitializationException("IPv6 only is not supported with SOCKS4 proxies");
                }
                int proxyPort = props.getPropertyAsIntWithDefault("Ice.SOCKSProxyPort", 1080);
                return new SOCKSNetworkProxy(proxyHost, proxyPort);
            }

            proxyHost = props.getProperty("Ice.HTTPProxyHost");
            if(proxyHost.Length > 0)
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
        private int _batchAutoFlushSize; // Immutable, not reset by destroy().
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
        private Dictionary<string, Ice.Object> _adminFacets = new Dictionary<string, Ice.Object>();
        private HashSet<string> _adminFacetFilter = new HashSet<string>();
        private Ice.Identity _adminIdentity;
        private Dictionary<short, BufSizeWarnInfo> _setBufSizeWarn = new Dictionary<short, BufSizeWarnInfo>();
        private static bool _printProcessIdDone = false;
        private static bool _oneOffDone = false;
        private Dictionary<string, Ice.ObjectFactory> _objectFactoryMap = new Dictionary<string, Ice.ObjectFactory>();
        private static System.Object _staticLock = new System.Object();
    }
}
