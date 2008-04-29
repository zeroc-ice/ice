// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
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

    public sealed class Instance
    {
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
            return _traceLevels;
        }
        
        public DefaultsAndOverrides defaultsAndOverrides()
        {
            // No mutex lock, immutable.
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
                
                return _referenceFactory;
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
                
                return _outgoingConnectionFactory;
            }
        }
        
        public ConnectionMonitor connectionMonitor()
        {
            lock(this)
            {
                if(_state == StateDestroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }
                
                return _connectionMonitor;
            }
        }
        
        public ObjectFactoryManager servantFactoryManager()
        {
            lock(this)
            {
                if(_state == StateDestroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }
                
                return _servantFactoryManager;
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
                
                return _objectAdapterFactory;
            }
        }

        public int protocolSupport()
        {
            lock(this)
            {
                if(_state == StateDestroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }
                
                return _protocolSupport;
            }
        }
        
        public ThreadPool clientThreadPool()
        {
            lock(this)
            {
                if(_state == StateDestroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }
                
                if(_clientThreadPool == null) // Lazy initialization.
                {
                    _clientThreadPool = new ThreadPool(this, "Ice.ThreadPool.Client", 0);
                }
                
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
                    int timeout = _initData.properties.getPropertyAsInt("Ice.ServerIdleTime");
                    _serverThreadPool = new ThreadPool(this, "Ice.ThreadPool.Server", timeout);
                }
                
                return _serverThreadPool;
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

                if(_endpointHostResolver == null) // Lazy initialization.
                {
                    _endpointHostResolver = new EndpointHostResolver(this);
                }

                return _endpointHostResolver;
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
                
                if(_timer == null)
                {
                    _timer = new Timer(this);
                }
                
                return _timer;
            }
        }

        public bool background()
        {
            // No mutex lock, immutable.
            return _background;
        }

        public EndpointFactoryManager endpointFactoryManager()
        {
            lock(this)
            {
                if(_state == StateDestroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }
                
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
                
                return _pluginManager;
            }
        }
        
        public int messageSizeMax()
        {
            // No mutex lock, immutable.
            return _messageSizeMax;
        }
        
        public int clientACM()
        {
            // No mutex lock, immutable.
            return _clientACM;
        }
        
        public int serverACM()
        {
            // No mutex lock, immutable.
            return _serverACM;
        }
        
        public void setDefaultContext(Dictionary<string, string> ctx)
        {
            lock(this)
            {
                if(_state == StateDestroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }
        
                if(ctx == null || ctx.Count == 0)
                {
                    _defaultContext = _emptyContext;
                }
                else
                {
                    _defaultContext = new Dictionary<string, string>(ctx);
                }
            }
        }

        public Dictionary<string, string> getDefaultContext()
        {
            lock(this)
            {
                if(_state == StateDestroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }
                
                return new Dictionary<string, string>(_defaultContext);
            }
        }

        public Ice.ImplicitContextI getImplicitContext()
        {
            return _implicitContext;
        }

        public void flushBatchRequests()
        {
            OutgoingConnectionFactory connectionFactory;
            ObjectAdapterFactory adapterFactory;
            
            lock(this)
            {
                if(_state == StateDestroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }
                
                connectionFactory = _outgoingConnectionFactory;
                adapterFactory = _objectAdapterFactory;
            }
            
            connectionFactory.flushBatchRequests();
            adapterFactory.flushBatchRequests();
        }

        public Ice.Identity stringToIdentity(string s)
        {
            return Ice.Util.stringToIdentity(s);
        }

        public string identityToString(Ice.Identity ident)
        {
            return Ice.Util.identityToString(ident);
        }

        public Ice.ObjectPrx 
        getAdmin()
        {
            Ice.ObjectAdapter adapter = null;
            string serverId = null;
            Ice.LocatorPrx defaultLocator = null;

            lock(this)
            {
                if(_state == StateDestroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }
                
                string adminOA = "Ice.Admin";
                
                if(_adminAdapter != null)
                {
                    return _adminAdapter.createProxy(_adminIdentity);
                }
                else if(_initData.properties.getProperty(adminOA + ".Endpoints").Length == 0)
                {
                    return null;
                }
                else
                {
                    serverId = _initData.properties.getProperty("Ice.Admin.ServerId");
                    string instanceName = _initData.properties.getProperty("Ice.Admin.InstanceName");
                    
                    defaultLocator = _referenceFactory.getDefaultLocator();
                    
                    if((defaultLocator != null && serverId.Length > 0) || instanceName.Length > 0)
                    {
                        if(_adminIdentity == null)
                        {
                            if(instanceName.Length == 0)
                            {
                                instanceName = Ice.Util.generateUUID();
                            }
                            _adminIdentity = new Ice.Identity("admin", instanceName);
                            //
                            // Afterwards, _adminIdentity is read-only
                            //
                        }
                        
                        //
                        // Create OA
                        //
                        _adminAdapter = _objectAdapterFactory.createObjectAdapter(adminOA, "", null);
                
                        //
                        // Add all facets to OA
                        //
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

                        adapter = _adminAdapter;
                    }
                }
            }

            if(adapter == null)
            {
                return null;
            }
            else
            {
                try
                {
                    adapter.activate();
                }
                catch(Ice.LocalException)
                {
                    //
                    // We cleanup _adminAdapter, however this error is not recoverable
                    // (can't call again getAdmin() after fixing the problem)
                    // since all the facets (servants) in the adapter are lost
                    //
                    lock(this)
                    {
                        _adminAdapter = null;
                        adapter.destroy();
                    }
                    throw;
                }

                if(defaultLocator != null && serverId.Length > 0)
                {    
                    Ice.ProcessPrx process = Ice.ProcessPrxHelper.uncheckedCast(
                        adapter.createProxy(_adminIdentity).ice_facet("Process"));
                
                    try
                    {
                        defaultLocator.getRegistry().setServerProcessProxy(serverId, process);
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
                        
                        throw new Ice.InitializationException("Locator knows nothing about server '" + serverId + "'");
                    }
                    catch(Ice.LocalException ex)
                    {
                        if(_traceLevels.location >= 1)
                        {
                            System.Text.StringBuilder s = new System.Text.StringBuilder();
                            s.Append("couldn't register server `" + serverId + "' with the locator registry:\n" + ex);
                            _initData.logger.trace(_traceLevels.locationCat, s.ToString());
                        }
                        throw ex; // TODO: Shall we raise a special exception instead of a non obvious local exception?
                    }
            
                    if(_traceLevels.location >= 1)
                    {
                        System.Text.StringBuilder s = new System.Text.StringBuilder();
                        s.Append("registered server `" + serverId + "' with the locator registry");
                        _initData.logger.trace(_traceLevels.locationCat, s.ToString());
                    }
                }
                return adapter.createProxy(_adminIdentity);
            }    
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
            
                if(_adminAdapter == null || (_adminFacetFilter.Count == 0 && !_adminFacetFilter.Contains(facet)))
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
                if(_adminAdapter == null || (_adminFacetFilter.Count == 0 && !_adminFacetFilter.Contains(facet)))
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
            // No locking, as it can only be called during plugin loading
            //
            _initData.logger = logger;
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
                    if(_initData.properties.getPropertyAsInt("Ice.UseSyslog") > 0)
                    {
                        _initData.logger = new Ice.SysLoggerI(_initData.properties.getProperty("Ice.ProgramName"));
                    }
                    else
                    {
                        _initData.logger = Ice.Util.getProcessLogger();
                    }
                }
                
                _traceLevels = new TraceLevels(_initData.properties);
                
                _defaultsAndOverrides = new DefaultsAndOverrides(_initData.properties);
                
                {
                    const int defaultMessageSizeMax = 1024;
                    int num = 
                        _initData.properties.getPropertyAsIntWithDefault("Ice.MessageSizeMax", defaultMessageSizeMax);
                    if(num < 1)
                    {
                        _messageSizeMax = defaultMessageSizeMax * 1024; // Ignore non-sensical values.
                    }
                    else if(num > 0x7fffffff / 1024)
                    {
                        _messageSizeMax = 0x7fffffff;
                    }
                    else
                    {
                        _messageSizeMax = num * 1024; // Property is in kilobytes, _messageSizeMax in bytes
                    }
                }
                
                //
                // Client ACM enabled by default. Server ACM disabled by default.
                //
                _clientACM = _initData.properties.getPropertyAsIntWithDefault("Ice.ACM.Client", 60);
                _serverACM = _initData.properties.getPropertyAsInt("Ice.ACM.Server");

                _implicitContext = Ice.ImplicitContextI.create(_initData.properties.getProperty("Ice.ImplicitContext"));
                _background = _initData.properties.getPropertyAsInt("Ice.Background") > 0;

                _routerManager = new RouterManager();
                
                _locatorManager = new LocatorManager();
                
                _referenceFactory = new ReferenceFactory(this, communicator);
                
                _proxyFactory = new ProxyFactory(this);
                
                bool ipv4 = _initData.properties.getPropertyAsIntWithDefault("Ice.IPv4", 1) > 0;
                bool ipv6 = _initData.properties.getPropertyAsIntWithDefault("Ice.IPv6", 0) > 0;
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
                _endpointFactoryManager = new EndpointFactoryManager(this);
                EndpointFactory tcpEndpointFactory = new TcpEndpointFactory(this);
                _endpointFactoryManager.add(tcpEndpointFactory);
                EndpointFactory udpEndpointFactory = new UdpEndpointFactory(this);
                _endpointFactoryManager.add(udpEndpointFactory);
                
                _pluginManager = new Ice.PluginManagerI(communicator);

                _defaultContext = _emptyContext;
                
                _outgoingConnectionFactory = new OutgoingConnectionFactory(this);
                
                _servantFactoryManager = new ObjectFactoryManager();
                
                _objectAdapterFactory = new ObjectAdapterFactory(this, communicator);

                string[] facetFilter = _initData.properties.getPropertyAsList("Ice.Admin.Facets");
                if(facetFilter.Length > 0)
                {
                    foreach(string s in facetFilter)
                    {
                        _adminFacetFilter.Add(s);
                    }
                }
                _adminFacets.Add("Properties", new PropertiesAdminI(_initData.properties));
                _adminFacets.Add("Process", new ProcessI(communicator));
            }
            catch(Ice.LocalException)
            {
                destroy();
                throw;
            }
        }
        
        public void finishSetup(ref string[] args)
        {
            //
            // Load plug-ins.
            //
            Ice.PluginManagerI pluginManagerImpl = (Ice.PluginManagerI)_pluginManager;
            pluginManagerImpl.loadPlugins(ref args);
            
            //
            // Get default router and locator proxies. Don't move this
            // initialization before the plug-in initialization!!! The proxies
            // might depend on endpoint factories to be installed by plug-ins.
            //
            Ice.RouterPrx r = Ice.RouterPrxHelper.uncheckedCast(_proxyFactory.propertyToProxy("Ice.Default.Router"));
            if(r != null)
            {
                _referenceFactory = _referenceFactory.setDefaultRouter(r);
            }

            Ice.LocatorPrx l = Ice.LocatorPrxHelper.uncheckedCast(_proxyFactory.propertyToProxy("Ice.Default.Locator"));
            if(l != null)
            {
                _referenceFactory = _referenceFactory.setDefaultLocator(l);
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
             
            if(_initData.properties.getPropertyAsIntWithDefault("Ice.Admin.DelayCreation", 0) <= 0)
            {
                getAdmin();
            }

            //
            // Start connection monitor if necessary.
            //
            int interval = 0;
            if(_clientACM > 0 && _serverACM > 0)
            {
                if(_clientACM < _serverACM)
                {
                    interval = _clientACM;
                }
                else
                {
                    interval = _serverACM;
                }
            }
            else if(_clientACM > 0)
            {
                interval = _clientACM;
            }
            else if(_serverACM > 0)
            {
                interval = _serverACM;
            }
            interval = _initData.properties.getPropertyAsIntWithDefault("Ice.MonitorConnections", interval);
            if(interval > 0)
            {
                _connectionMonitor = new ConnectionMonitor(this, interval);
            }
            
            //
            // Thread pool initialization is now lazy initialization in
            // clientThreadPool() and serverThreadPool().
            //
        }
        
        //
        // Only for use by Ice.CommunicatorI
        //
        public bool destroy()
        {
            lock(this)
            {
                //
                // If the _state is not StateActive then the instance is
                // either being destroyed, or has already been destroyed.
                //
                if(_state != StateActive)
                {
                    return false;
                }
            
                //
                // We cannot set state to StateDestroyed otherwise instance
                // methods called during the destroy process (such as
                // outgoingConnectionFactory() from
                // ObjectAdapterI::deactivate() will cause an exception.
                //
                _state = StateDestroyInProgress;
            }
            
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
            
            ThreadPool serverThreadPool = null;
            ThreadPool clientThreadPool = null;
            EndpointHostResolver endpointHostResolver = null;

            lock(this)
            {
                _objectAdapterFactory = null;
                
                _outgoingConnectionFactory = null;
                
                if(_connectionMonitor != null)
                {
                    _connectionMonitor.destroy();
                    _connectionMonitor = null;
                }
                
                if(_serverThreadPool != null)
                {
                    _serverThreadPool.destroy();
                    serverThreadPool = _serverThreadPool;
                    _serverThreadPool = null;
                }

                if(_clientThreadPool != null)
                {
                    _clientThreadPool.destroy();
                    clientThreadPool = _clientThreadPool;
                    _clientThreadPool = null;
                }

                if(_endpointHostResolver != null)
                {
                    _endpointHostResolver.destroy();
                    endpointHostResolver = _endpointHostResolver;
                    _endpointHostResolver = null;
                }

                if(_timer != null)
                {
                    _timer.destroy();
                    _timer = null;
                }
                
                if(_servantFactoryManager != null)
                {
                    _servantFactoryManager.destroy();
                    _servantFactoryManager = null;
                }
                
                if(_referenceFactory != null)
                {
                    _referenceFactory.destroy();
                    _referenceFactory = null;
                }
                
                // No destroy function defined.
                // _proxyFactory.destroy();
                _proxyFactory = null;
                
                if(_routerManager != null)
                {
                    _routerManager.destroy();
                    _routerManager = null;
                }
                
                if(_locatorManager != null)
                {
                    _locatorManager.destroy();
                    _locatorManager = null;
                }
                
                if(_endpointFactoryManager != null)
                {
                    _endpointFactoryManager.destroy();
                    _endpointFactoryManager = null;
                }
                
                if(_pluginManager != null)
                {
                    _pluginManager.destroy();
                    _pluginManager = null;
                }

                _adminAdapter = null;
                _adminFacets.Clear();
                
                _state = StateDestroyed;
            }
            
            //
            // Join with threads outside the synchronization.
            //
            if(clientThreadPool != null)
            {
                clientThreadPool.joinWithAllThreads();
            }
            if(serverThreadPool != null)
            {
                serverThreadPool.joinWithAllThreads();
            }
            if(endpointHostResolver != null)
            {
                endpointHostResolver.joinWithThread();
            }

            if(_initData.properties.getPropertyAsInt("Ice.Warn.UnusedProperties") > 0)
            {
                ArrayList unusedProperties = ((Ice.PropertiesI)_initData.properties).getUnusedProperties();
                if(unusedProperties.Count != 0)
                {
                    string message = "The following properties were set but never read:";
                    foreach(string s in unusedProperties)
                    {
                        message += "\n    " + s;
                    }
                    _initData.logger.warning(message);
                }
            }

            return true;
        }
        
        private const int StateActive = 0;
        private const int StateDestroyInProgress = 1;
        private const int StateDestroyed = 2;
        private int _state;
        private Ice.InitializationData _initData; // Immutable, not reset by destroy().
        private TraceLevels _traceLevels; // Immutable, not reset by destroy().
        private DefaultsAndOverrides _defaultsAndOverrides; // Immutable, not reset by destroy().
        private int _messageSizeMax; // Immutable, not reset by destroy().
        private int _clientACM; // Immutable, not reset by destroy().
        private int _serverACM; // Immutable, not reset by destroy().
        private Ice.ImplicitContextI _implicitContext; // Immutable
        private RouterManager _routerManager;
        private LocatorManager _locatorManager;
        private ReferenceFactory _referenceFactory;
        private ProxyFactory _proxyFactory;
        private OutgoingConnectionFactory _outgoingConnectionFactory;
        private ConnectionMonitor _connectionMonitor;
        private ObjectFactoryManager _servantFactoryManager;
        private ObjectAdapterFactory _objectAdapterFactory;
        private int _protocolSupport;
        private ThreadPool _clientThreadPool;
        private ThreadPool _serverThreadPool;
        private EndpointHostResolver _endpointHostResolver;
        private Timer _timer;
        private bool _background;
        private EndpointFactoryManager _endpointFactoryManager;
        private Ice.PluginManager _pluginManager;
        private Dictionary<string, string> _defaultContext;
        private Ice.ObjectAdapter _adminAdapter;
        private Dictionary<string, Ice.Object> _adminFacets = new Dictionary<string, Ice.Object>();
        private IceUtilInternal.Set _adminFacetFilter = new IceUtilInternal.Set();
        private Ice.Identity _adminIdentity;

        private static Dictionary<string, string> _emptyContext = new Dictionary<string, string>();
        private static bool _printProcessIdDone = false;

        private static bool _oneOffDone = false;
        private static System.Object _staticLock = new System.Object();
    }

}
