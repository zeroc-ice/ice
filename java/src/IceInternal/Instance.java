// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.TimeUnit;

public final class Instance
{
    static private class ThreadObserverHelper
    {
        ThreadObserverHelper(String threadName)
        {
            _threadName = threadName;
        }

        synchronized public void updateObserver(Ice.Instrumentation.CommunicatorObserver obsv)
        {
            assert(obsv != null);
            
            _observer = obsv.getThreadObserver("Communicator", 
                                               _threadName,
                                               Ice.Instrumentation.ThreadState.ThreadStateIdle,
                                               _observer);
            if(_observer != null)
            {
                _observer.attach();
            }
        }

        protected void beforeExecute()
        {
            _threadObserver = _observer;
            if(_threadObserver != null)
            {
                _threadObserver.stateChanged(Ice.Instrumentation.ThreadState.ThreadStateIdle,
                                             Ice.Instrumentation.ThreadState.ThreadStateInUseForOther);
            }
        }

        protected void afterExecute()
        {
            if(_threadObserver != null)
            {
                _threadObserver.stateChanged(Ice.Instrumentation.ThreadState.ThreadStateInUseForOther,
                                             Ice.Instrumentation.ThreadState.ThreadStateIdle);
                _threadObserver = null;
            }
        }

        final private String _threadName;
        //
        // We use a volatile to avoid synchronization when reading
        // _observer. Reference assignement is atomic in Java so it
        // also doesn't need to be synchronized.
        // 
        private volatile Ice.Instrumentation.ThreadObserver _observer;
        private Ice.Instrumentation.ThreadObserver _threadObserver;
    };

    static private class Timer extends java.util.concurrent.ScheduledThreadPoolExecutor
    {
        Timer(Ice.Properties props, String threadName)
        {
            super(1, Util.createThreadFactory(props, threadName)); // Single thread executor
            setRemoveOnCancelPolicy(true);
            setExecuteExistingDelayedTasksAfterShutdownPolicy(false);
            _observerHelper = new ThreadObserverHelper(threadName);
        }

        public void updateObserver(Ice.Instrumentation.CommunicatorObserver obsv)
        {
            _observerHelper.updateObserver(obsv);
        }

        @Override
        protected void beforeExecute(Thread t, Runnable r)
        {
            _observerHelper.beforeExecute();
        }

        @Override
        protected void afterExecute(Runnable t, Throwable e)
        {
            _observerHelper.afterExecute();
        }

        private final ThreadObserverHelper _observerHelper;
    };

    static private class QueueExecutor extends java.util.concurrent.ThreadPoolExecutor
    {
        QueueExecutor(Ice.Properties props, String threadName)
        {
            super(1, 1, 0, TimeUnit.MILLISECONDS, new java.util.concurrent.LinkedBlockingQueue<Runnable>(),
                  Util.createThreadFactory(props, threadName));
            _observerHelper = new ThreadObserverHelper(threadName);
        }

        public void updateObserver(Ice.Instrumentation.CommunicatorObserver obsv)
        {
            _observerHelper.updateObserver(obsv);
        }

        @Override
        protected void beforeExecute(Thread t, Runnable r)
        {
            _observerHelper.beforeExecute();
        }

        @Override
        protected void afterExecute(Runnable t, Throwable e)
        {
            _observerHelper.afterExecute();
        }

        private final ThreadObserverHelper _observerHelper;
    };

    private class ObserverUpdaterI implements Ice.Instrumentation.ObserverUpdater
    {
        @Override
        public void
        updateConnectionObservers()
        {
            Instance.this.updateConnectionObservers();
        }

        @Override
        public void
        updateThreadObservers()
        {
            Instance.this.updateThreadObservers();
        }
    }

    public Ice.InitializationData
    initializationData()
    {
        //
        // No check for destruction. It must be possible to access the
        // initialization data after destruction.
        //
        // No mutex lock, immutable.
        //
        return _initData;
    }

    public TraceLevels
    traceLevels()
    {
        // No mutex lock, immutable.
        assert(_traceLevels != null);
        return _traceLevels;
    }

    public DefaultsAndOverrides
    defaultsAndOverrides()
    {
        // No mutex lock, immutable.
        assert(_defaultsAndOverrides != null);
        return _defaultsAndOverrides;
    }

    public synchronized RouterManager
    routerManager()
    {
        if(_state == StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        assert(_routerManager != null);
        return _routerManager;
    }

    public synchronized LocatorManager
    locatorManager()
    {
        if(_state == StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        assert(_locatorManager != null);
        return _locatorManager;
    }

    public synchronized ReferenceFactory
    referenceFactory()
    {
        if(_state == StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        assert(_referenceFactory != null);
        return _referenceFactory;
    }

    public synchronized ProxyFactory
    proxyFactory()
    {
        if(_state == StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        assert(_proxyFactory != null);
        return _proxyFactory;
    }

    public synchronized OutgoingConnectionFactory
    outgoingConnectionFactory()
    {
        if(_state == StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        assert(_outgoingConnectionFactory != null);
        return _outgoingConnectionFactory;
    }

    public synchronized ObjectFactoryManager
    servantFactoryManager()
    {
        if(_state == StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        assert(_servantFactoryManager != null);
        return _servantFactoryManager;
    }

    public synchronized ObjectAdapterFactory
    objectAdapterFactory()
    {
        if(_state == StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        assert(_objectAdapterFactory != null);
        return _objectAdapterFactory;
    }

    public int
    protocolSupport()
    {
        return _protocolSupport;
    }

    public boolean
    preferIPv6()
    {
        return _preferIPv6;
    }

    public NetworkProxy
    networkProxy()
    {
        return _networkProxy;
    }

    public synchronized ThreadPool
    clientThreadPool()
    {
        if(_state == StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        assert(_clientThreadPool != null);
        return _clientThreadPool;
    }

    public synchronized ThreadPool
    serverThreadPool(boolean create)
    {
        if(_state == StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        if(_serverThreadPool == null && create) // Lazy initialization.
        {
            int timeout = _initData.properties.getPropertyAsInt("Ice.ServerIdleTime");
            _serverThreadPool = new ThreadPool(this, "Ice.ThreadPool.Server", timeout);
        }

        return _serverThreadPool;
    }

    public synchronized EndpointHostResolver
    endpointHostResolver()
    {
        if(_state == StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        assert(_endpointHostResolver != null);
        return _endpointHostResolver;
    }

    synchronized public RetryQueue
    retryQueue()
    {
        if(_state == StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        assert(_retryQueue != null);
        return _retryQueue;
    }

    synchronized public java.util.concurrent.ScheduledExecutorService
    timer()
    {
        if(_state == StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        assert(_timer != null);
        return _timer;
    }

    public synchronized EndpointFactoryManager
    endpointFactoryManager()
    {
        if(_state == StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        assert(_endpointFactoryManager != null);
        return _endpointFactoryManager;
    }

    public synchronized Ice.PluginManager
    pluginManager()
    {
        if(_state == StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        assert(_pluginManager != null);
        return _pluginManager;
    }

    public int
    messageSizeMax()
    {
        // No mutex lock, immutable.
        return _messageSizeMax;
    }

    public int
    cacheMessageBuffers()
    {
        // No mutex lock, immutable.
        return _cacheMessageBuffers;
    }

    public ACMConfig
    clientACM()
    {
        // No mutex lock, immutable.
        return _clientACM;
    }

    public ACMConfig
    serverACM()
    {
        // No mutex lock, immutable.
        return _serverACM;
    }

    public Ice.ImplicitContextI
    getImplicitContext()
    {
        return _implicitContext;
    }

    public Ice.Identity
    stringToIdentity(String s)
    {
        return Ice.Util.stringToIdentity(s);
    }

    public String
    identityToString(Ice.Identity ident)
    {
        return Ice.Util.identityToString(ident);
    }

    public synchronized Ice.ObjectPrx
    createAdmin(Ice.ObjectAdapter adminAdapter, Ice.Identity adminIdentity)
    {
        if(Thread.interrupted())
        {
            throw new Ice.OperationInterruptedException();
        }

        boolean createAdapter = (adminAdapter == null);
    
        synchronized(this)
        {
            if(_state == StateDestroyed)
            {
                throw new Ice.CommunicatorDestroyedException();
            }
            
            if(adminIdentity == null || adminIdentity.name == null || adminIdentity.name.isEmpty())
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
                if(!_initData.properties.getProperty("Ice.Admin.Endpoints").isEmpty())
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
            catch(Ice.LocalException ex)
            {
                //
                // We cleanup _adminAdapter, however this error is not recoverable
                // (can't call again getAdmin() after fixing the problem)
                // since all the facets (servants) in the adapter are lost
                //
                adminAdapter.destroy();
                synchronized(this)
                {
                    _adminAdapter = null;
                }
                throw ex;
            }
        }
        setServerProcessProxy(adminAdapter, adminIdentity);
        return adminAdapter.createProxy(adminIdentity);
    }

    public Ice.ObjectPrx
    getAdmin()
    {
        if(Thread.interrupted())
        {
            throw new Ice.OperationInterruptedException();
        }

        Ice.ObjectAdapter adminAdapter;
        Ice.Identity adminIdentity;

        synchronized(this)
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
                if(!_initData.properties.getProperty("Ice.Admin.Endpoints").isEmpty())
                {
                    adminAdapter = _objectAdapterFactory.createObjectAdapter("Ice.Admin", null);
                }
                else
                {
                    return null;
                }
                adminIdentity = new Ice.Identity("admin", _initData.properties.getProperty("Ice.Admin.InstanceName"));
                if(adminIdentity.category.isEmpty())
                {
                    adminIdentity.category = java.util.UUID.randomUUID().toString();
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
        catch(Ice.LocalException ex)
        {
            //
            // We cleanup _adminAdapter, however this error is not recoverable
            // (can't call again getAdmin() after fixing the problem)
            // since all the facets (servants) in the adapter are lost
            //
            adminAdapter.destroy();
            synchronized(this)
            {
                _adminAdapter = null;
            }
            throw ex;
        }
        
        setServerProcessProxy(adminAdapter, adminIdentity);
        return adminAdapter.createProxy(adminIdentity);
    }

    public synchronized void
    addAdminFacet(Ice.Object servant, String facet)
    {
        if(_state == StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        if(_adminAdapter == null || (!_adminFacetFilter.isEmpty() && !_adminFacetFilter.contains(facet)))
        {
            if(_adminFacets.get(facet) != null)
            {
                throw new Ice.AlreadyRegisteredException("facet", facet);
            }
            _adminFacets.put(facet, servant);
        }
        else
        {
            _adminAdapter.addFacet(servant, _adminIdentity, facet);
        }
    }

    public synchronized Ice.Object
    removeAdminFacet(String facet)
    {
        if(_state == StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        Ice.Object result = null;

        if(_adminAdapter == null || (!_adminFacetFilter.isEmpty() && !_adminFacetFilter.contains(facet)))
        {
            result = _adminFacets.remove(facet);
            if(result == null)
            {
                throw new Ice.NotRegisteredException("facet", facet);
            }
        }
        else
        {
            result = _adminAdapter.removeFacet(_adminIdentity, facet);
        }

        return result;
    }

    public synchronized Ice.Object
    findAdminFacet(String facet)
    {
        if(_state == StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        Ice.Object result = null;

        if(_adminAdapter == null || (!_adminFacetFilter.isEmpty() && !_adminFacetFilter.contains(facet)))
        {
            result = _adminFacets.get(facet);
        }
        else
        {
            result = _adminAdapter.findFacet(_adminIdentity, facet);
        }

        return result;
    }

    public synchronized java.util.Map<String, Ice.Object>
    findAllAdminFacets()
    {
        if(_state == StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        if(_adminAdapter == null)
        {
            return new java.util.HashMap<String, Ice.Object>(_adminFacets);
        }
        else
        {
            java.util.Map<String, Ice.Object> result = _adminAdapter.findAllFacets(_adminIdentity);
            if(!_adminFacets.isEmpty())
            {
                // Also returns filtered facets
                result.putAll(_adminFacets);
            }
            return result;
        }
    }

    public synchronized void
    setDefaultLocator(Ice.LocatorPrx locator)
    {
        if(_state == StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        _referenceFactory = _referenceFactory.setDefaultLocator(locator);
    }

    public synchronized void
    setDefaultRouter(Ice.RouterPrx router)
    {
        if(_state == StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        _referenceFactory = _referenceFactory.setDefaultRouter(router);
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

    public Class<?>
    findClass(String className)
    {
        return Util.findClass(className, _initData.classLoader);
    }

    public synchronized String
    getClassForType(String type)
    {
        return _typeToClassMap.get(type);
    }

    public synchronized void
    addClassForType(String type, String className)
    {
        if(_typeToClassMap.containsKey(type))
        {
            assert(_typeToClassMap.get(type).equals(className));
        }
        else
        {
            _typeToClassMap.put(type, className);
        }
    }

    public String[]
    getPackages()
    {
        return _packages;
    }

    public boolean
    useApplicationClassLoader()
    {
        return _useApplicationClassLoader;
    }

    public boolean
    queueRequests()
    {
        return _queueExecutor != null;
    }

    synchronized public ExecutorService
    getQueueExecutor()
    {
        if(_state == StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }
        return _queueExecutor;
    }

    //
    // Only for use by Ice.CommunicatorI
    //
    public
    Instance(Ice.Communicator communicator, Ice.InitializationData initData)
    {
        _state = StateActive;
        _initData = initData;

        try
        {
            if(_initData.properties == null)
            {
                _initData.properties = Ice.Util.createProperties();
            }

            synchronized(Instance.class)
            {
                if(!_oneOfDone)
                {
                    String stdOut = _initData.properties.getProperty("Ice.StdOut");
                    String stdErr = _initData.properties.getProperty("Ice.StdErr");

                    java.io.PrintStream outStream = null;

                    if(stdOut.length() > 0)
                    {
                        //
                        // We need to close the existing stdout for JVM thread dump to go
                        // to the new file
                        //
                        System.out.close();

                        try
                        {
                            outStream = new java.io.PrintStream(new java.io.FileOutputStream(stdOut, true));
                        }
                        catch(java.io.FileNotFoundException ex)
                        {
                            throw new Ice.FileException(0, stdOut, ex);
                        }

                        System.setOut(outStream);
                    }
                    if(stdErr.length() > 0)
                    {
                        //
                        // close for consistency with stdout
                        //
                        System.err.close();

                        if(stdErr.equals(stdOut))
                        {
                            System.setErr(outStream);
                        }
                        else
                        {
                            try
                            {
                                System.setErr(new java.io.PrintStream(new java.io.FileOutputStream(stdErr, true)));
                            }
                            catch(java.io.FileNotFoundException ex)
                            {
                                throw new Ice.FileException(0, stdErr, ex);
                            }

                        }
                    }
                    _oneOfDone = true;
                }
            }

            if(_initData.logger == null)
            {
                String logfile = _initData.properties.getProperty("Ice.LogFile");
                if(_initData.properties.getPropertyAsInt("Ice.UseSyslog") > 0 &&
                   !System.getProperty("os.name").startsWith("Windows"))
                {
                    if(logfile.length() != 0)
                    {
                        throw new Ice.InitializationException("Both syslog and file logger cannot be enabled.");
                    }
                    _initData.logger = new Ice.SysLoggerI(_initData.properties.getProperty("Ice.ProgramName"),
                                _initData.properties.getPropertyWithDefault("Ice.SyslogFacility", "LOG_USER"));
                }
                else if(logfile.length() != 0)
                {
                    _initData.logger = new Ice.LoggerI(_initData.properties.getProperty("Ice.ProgramName"), logfile);
                }
                else
                {
                    _initData.logger = Ice.Util.getProcessLogger();
                }
            }

            _packages = validatePackages();

            _useApplicationClassLoader = _initData.properties.getPropertyAsInt("Ice.UseApplicationClassLoader") > 0;

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
                final int defaultMessageSizeMax = 1024;
                int num = _initData.properties.getPropertyAsIntWithDefault("Ice.MessageSizeMax", defaultMessageSizeMax);
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


            _implicitContext = Ice.ImplicitContextI.create(_initData.properties.getProperty("Ice.ImplicitContext"));

            _routerManager = new RouterManager();

            _locatorManager = new LocatorManager(_initData.properties);

            _referenceFactory = new ReferenceFactory(this, communicator);

            _proxyFactory = new ProxyFactory(this);

            boolean ipv4 = _initData.properties.getPropertyAsIntWithDefault("Ice.IPv4", 1) > 0;
            boolean ipv6 = _initData.properties.getPropertyAsIntWithDefault("Ice.IPv6", 1) > 0;
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
            ProtocolInstance tcpProtocolInstance = new ProtocolInstance(this, Ice.TCPEndpointType.value, "tcp");
            EndpointFactory tcpEndpointFactory = new TcpEndpointFactory(tcpProtocolInstance);
            _endpointFactoryManager.add(tcpEndpointFactory);

            ProtocolInstance udpProtocolInstance = new ProtocolInstance(this, Ice.UDPEndpointType.value, "udp");
            EndpointFactory udpEndpointFactory = new UdpEndpointFactory(udpProtocolInstance);
            _endpointFactoryManager.add(udpEndpointFactory);

            ProtocolInstance wsProtocolInstance = new ProtocolInstance(this, Ice.WSEndpointType.value, "ws");
            EndpointFactory wsEndpointFactory = new WSEndpointFactory(wsProtocolInstance,
                                                                      tcpEndpointFactory.clone(wsProtocolInstance));
            _endpointFactoryManager.add(wsEndpointFactory);

            _pluginManager = new Ice.PluginManagerI(communicator, this);

            _outgoingConnectionFactory = new OutgoingConnectionFactory(communicator, this);

            _servantFactoryManager = new ObjectFactoryManager();

            _objectAdapterFactory = new ObjectAdapterFactory(this, communicator);

            _retryQueue = new RetryQueue(this);

            if(_initData.properties.getPropertyAsInt("Ice.BackgroundIO") > 0)
            {
                _queueExecutor = new QueueExecutor(_initData.properties, 
                                                   Util.createThreadName(_initData.properties, "Ice.BackgroundIO"));
                // Caching message buffers is not supported with background IO.
                _cacheMessageBuffers = 0;
            }
            else
            {
                _cacheMessageBuffers = _initData.properties.getPropertyAsIntWithDefault("Ice.CacheMessageBuffers", 2);
            }
        }
        catch(Ice.LocalException ex)
        {
            destroy();
            throw ex;
        }
    }

    @Override
    protected synchronized void
    finalize()
        throws Throwable
    {
        try
        {
            IceUtilInternal.Assert.FinalizerAssert(_state == StateDestroyed);
            IceUtilInternal.Assert.FinalizerAssert(_referenceFactory == null);
            IceUtilInternal.Assert.FinalizerAssert(_proxyFactory == null);
            IceUtilInternal.Assert.FinalizerAssert(_outgoingConnectionFactory == null);
            IceUtilInternal.Assert.FinalizerAssert(_servantFactoryManager == null);
            IceUtilInternal.Assert.FinalizerAssert(_objectAdapterFactory == null);
            IceUtilInternal.Assert.FinalizerAssert(_clientThreadPool == null);
            IceUtilInternal.Assert.FinalizerAssert(_serverThreadPool == null);
            IceUtilInternal.Assert.FinalizerAssert(_endpointHostResolver == null);
            IceUtilInternal.Assert.FinalizerAssert(_timer == null);
            IceUtilInternal.Assert.FinalizerAssert(_routerManager == null);
            IceUtilInternal.Assert.FinalizerAssert(_locatorManager == null);
            IceUtilInternal.Assert.FinalizerAssert(_endpointFactoryManager == null);
            IceUtilInternal.Assert.FinalizerAssert(_pluginManager == null);
            IceUtilInternal.Assert.FinalizerAssert(_retryQueue == null);
        }
        catch(java.lang.Exception ex)
        {
        }
        finally
        {
            super.finalize();
        }
    }

    public void
    finishSetup(Ice.StringSeqHolder args, Ice.Communicator communicator)
    {
        //
        // Load plug-ins.
        //
        assert(_serverThreadPool == null);
        Ice.PluginManagerI pluginManagerImpl = (Ice.PluginManagerI)_pluginManager;
        pluginManagerImpl.loadPlugins(args);

        //
        // Create Admin facets, if enabled.
        //
        // Note that any logger-dependent admin facet must be created after we load all plugins,
        // since one of these plugins can be a Logger plugin that sets a new logger during loading
        //
            
        if(_initData.properties.getProperty("Ice.Admin.Enabled").isEmpty())
        {
            _adminEnabled = !_initData.properties.getProperty("Ice.Admin.Endpoints").isEmpty();
        }
        else
        {
            _adminEnabled = _initData.properties.getPropertyAsInt("Ice.Admin.Enabled") > 0;
        }

        String[] facetFilter = _initData.properties.getPropertyAsList("Ice.Admin.Facets");
        if(facetFilter.length > 0)
        {
            _adminFacetFilter.addAll(java.util.Arrays.asList(facetFilter));
        }
        
        if(_adminEnabled)
        {        
            //
            // Process facet
            // 
            String processFacetName = "Process";
            if(_adminFacetFilter.isEmpty() || _adminFacetFilter.contains(processFacetName))
            {
                _adminFacets.put(processFacetName, new ProcessI(communicator));
            }
            
            //
            // Logger facet
            //
            String loggerFacetName = "Logger";
            if(_adminFacetFilter.isEmpty() || _adminFacetFilter.contains(loggerFacetName))
            {
                LoggerAdminLogger logger = new LoggerAdminLoggerI(_initData.properties, _initData.logger);
                setLogger(logger);
                _adminFacets.put(loggerFacetName, logger.getFacet());
            }

            //
            // Properties facet
            //
            String propertiesFacetName = "Properties";
            PropertiesAdminI propsAdmin = null;
            if(_adminFacetFilter.isEmpty() || _adminFacetFilter.contains(propertiesFacetName))
            {
                propsAdmin = new PropertiesAdminI(_initData.properties, _initData.logger);
                _adminFacets.put(propertiesFacetName, propsAdmin);
            }

            //
            // Metrics facet
            //
            String metricsFacetName = "Metrics";
            if(_adminFacetFilter.isEmpty() || _adminFacetFilter.contains(metricsFacetName))
            {
                 CommunicatorObserverI observer = new CommunicatorObserverI(_initData);
                 _initData.observer = observer;
                 _adminFacets.put(metricsFacetName, observer.getFacet());
                 
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
            _initData.observer.setObserverUpdater(new ObserverUpdaterI());
        }

        //
        // Create threads.
        //
        try
        {
            _timer = new Timer(_initData.properties, Util.createThreadName(_initData.properties, "Ice.Timer"));
        }
        catch(RuntimeException ex)
        {
            String s = "cannot create thread for timer:\n" + Ex.toString(ex);
            _initData.logger.error(s);
            throw ex;
        }

        try
        {
            _endpointHostResolver = new EndpointHostResolver(this);
        }
        catch(RuntimeException ex)
        {
            String s = "cannot create thread for endpoint host resolver:\n" + Ex.toString(ex);
            _initData.logger.error(s);
            throw ex;
        }

        _clientThreadPool = new ThreadPool(this, "Ice.ThreadPool.Client", 0);

        //
        // Get default router and locator proxies. Don't move this
        // initialization before the plug-in initialization!!! The proxies
        // might depend on endpoint factories to be installed by plug-ins.
        //
        Ice.RouterPrx router = Ice.RouterPrxHelper.uncheckedCast(_proxyFactory.propertyToProxy("Ice.Default.Router"));
        if(router != null)
        {
            _referenceFactory = _referenceFactory.setDefaultRouter(router);
        }

        Ice.LocatorPrx loc = Ice.LocatorPrxHelper.uncheckedCast(_proxyFactory.propertyToProxy("Ice.Default.Locator"));
        if(loc != null)
        {
            _referenceFactory = _referenceFactory.setDefaultLocator(loc);
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
    public void
    destroy()
    {
        if(Thread.interrupted())
        {
            throw new Ice.OperationInterruptedException();
        }

        synchronized(this)
        {
            //
            // If the _state is not StateActive then the instance is
            // either being destroyed, or has already been destroyed.
            //
            if(_state != StateActive)
            {
                return;
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
            try
            {
                _outgoingConnectionFactory.waitUntilFinished();
            }
            catch (InterruptedException e)
            {
                throw new Ice.OperationInterruptedException();
            }
        }

        if(_retryQueue != null)
        {
            _retryQueue.destroy();
        }

        if(_initData.observer != null)
        {
            _initData.observer.setObserverUpdater(null);
        }

        if(_initData.logger instanceof LoggerAdminLogger)
        {
            //
            // This only disables the remote logging; we don't set or reset _initData.logger
            //
            ((LoggerAdminLogger)_initData.logger).destroy();
        }
        
        ThreadPool serverThreadPool = null;
        ThreadPool clientThreadPool = null;
        EndpointHostResolver endpointHostResolver = null;
        ExecutorService queueExecutor = null;
        synchronized(this)
        {
            _objectAdapterFactory = null;
            _outgoingConnectionFactory = null;
            _retryQueue = null;

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
                // Shutdown the executor. It isn't necessary to call
                // awaitTermination since the threads are not daemon and
                // therefore the VM will block until all threads have
                // terminated.
                _timer.shutdown();
                // Once we support interrupt we can use shutdownNow.
                //_timer.shutdownNow();

                _timer = null;
            }

            if(_servantFactoryManager != null)
            {
                _servantFactoryManager.destroy();
                _servantFactoryManager = null;
            }

            //_referenceFactory.destroy(); // No destroy function defined.
            _referenceFactory = null;

            // _proxyFactory.destroy(); // No destroy function defined.
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
            _adminFacets.clear();

            queueExecutor = _queueExecutor;
            _queueExecutor = null;

            _typeToClassMap.clear();

            _state = StateDestroyed;
        }

        try
        {
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
        }
        catch(InterruptedException ex)
        {
            throw new Ice.OperationInterruptedException();
        }

        if(queueExecutor != null)
        {
            queueExecutor.shutdown();
            try
            {
                queueExecutor.awaitTermination(Long.MAX_VALUE, TimeUnit.NANOSECONDS);
            }
            catch (InterruptedException e)
            {
                throw new Ice.OperationInterruptedException();
            }
            queueExecutor = null;
        }

        if(_initData.properties.getPropertyAsInt("Ice.Warn.UnusedProperties") > 0)
        {
            java.util.List<String> unusedProperties = ((Ice.PropertiesI)_initData.properties).getUnusedProperties();
            if(unusedProperties.size() != 0)
            {
                StringBuffer message = new StringBuffer("The following properties were set but never read:");
                for(String p : unusedProperties)
                {
                    message.append("\n    ");
                    message.append(p);
                }
                _initData.logger.warning(message.toString());
            }
        }
    }

    private void
    updateConnectionObservers()
    {
        try
        {
            assert(_outgoingConnectionFactory != null);
            _outgoingConnectionFactory.updateConnectionObservers();
            assert(_objectAdapterFactory != null);
            _objectAdapterFactory.updateConnectionObservers();
        }
        catch(Ice.CommunicatorDestroyedException ex)
        {
        }
    }

    private void
    updateThreadObservers()
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
            assert(_objectAdapterFactory != null);
            _objectAdapterFactory.updateThreadObservers();
            if(_endpointHostResolver != null)
            {
                _endpointHostResolver.updateObserver();
            }
            if(_timer != null)
            {
                _timer.updateObserver(_initData.observer);
            }
            if(_queueExecutor != null)
            {
                _queueExecutor.updateObserver(_initData.observer);
            }
        }
        catch(Ice.CommunicatorDestroyedException ex)
        {
        }
    }

    private String[]
    validatePackages()
    {
        final String prefix = "Ice.Package.";
        java.util.Map<String, String> map = _initData.properties.getPropertiesForPrefix(prefix);
        java.util.List<String> packages = new java.util.ArrayList<String>();
        for(java.util.Map.Entry<String, String> p : map.entrySet())
        {
            String key = p.getKey();
            String pkg = p.getValue();
            if(key.length() == prefix.length())
            {
                _initData.logger.warning("ignoring invalid property: " + key + "=" + pkg);
            }
            String module = key.substring(prefix.length());
            String className = pkg + "." + module + "._Marker";
            Class<?> cls = null;
            try
            {
                cls = findClass(className);
            }
            catch(java.lang.Exception ex)
            {
            }
            if(cls == null)
            {
                _initData.logger.warning("unable to validate package: " + key + "=" + pkg);
            }
            else
            {
                packages.add(pkg);
            }
        }

        String pkg = _initData.properties.getProperty("Ice.Default.Package");
        if(pkg.length() > 0)
        {
            packages.add(pkg);
        }
        return packages.toArray(new String[packages.size()]);
    }

    private synchronized void
    addAllAdminFacets()
    {
        java.util.Map<String, Ice.Object> filteredFacets = new java.util.HashMap<String, Ice.Object>();
        for(java.util.Map.Entry<String, Ice.Object> p : _adminFacets.entrySet())
        {
            if(_adminFacetFilter.isEmpty() || _adminFacetFilter.contains(p.getKey()))
            {
                _adminAdapter.addFacet(p.getValue(), _adminIdentity, p.getKey());
            }
            else
            {
                filteredFacets.put(p.getKey(), p.getValue());
            }
        }
        _adminFacets = filteredFacets;
    }

    private void
    setServerProcessProxy(Ice.ObjectAdapter adminAdapter, Ice.Identity adminIdentity)
    {
        Ice.ObjectPrx admin = adminAdapter.createProxy(adminIdentity);
        Ice.LocatorPrx locator = adminAdapter.getLocator();
        String serverId = _initData.properties.getProperty("Ice.Admin.ServerId");
    
        if(locator != null && !serverId.isEmpty())
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
            catch(Ice.ServerNotFoundException ex)
            {
                if(_traceLevels.location >= 1)
                {
                    StringBuilder s = new StringBuilder(128);
                    s.append("couldn't register server `");
                    s.append(serverId);
                    s.append("' with the locator registry:\n");
                    s.append("the server is not known to the locator registry");
                    _initData.logger.trace(_traceLevels.locationCat, s.toString());
                }
                
                throw new Ice.InitializationException("Locator knows nothing about server `" + serverId + "'");
            }
            catch(Ice.LocalException ex)
            {
                if(_traceLevels.location >= 1)
                {
                    StringBuilder s = new StringBuilder(128);
                    s.append("couldn't register server `");
                    s.append(serverId);
                    s.append("' with the locator registry:\n");
                    s.append(ex.toString());
                    _initData.logger.trace(_traceLevels.locationCat, s.toString());
                }
                throw ex;
            }
            
            if(_traceLevels.location >= 1)
            {
                StringBuilder s = new StringBuilder(128);
                s.append("registered server `");
                s.append(serverId);
                s.append("' with the locator registry");
                _initData.logger.trace(_traceLevels.locationCat, s.toString());
            }
        }
    }

    private NetworkProxy createNetworkProxy(Ice.Properties properties, int protocolSupport)
    {
        String proxyHost;
        
        proxyHost = properties.getProperty("Ice.SOCKSProxyHost");
        if(!proxyHost.isEmpty())
        {
            if(protocolSupport == Network.EnableIPv6)
            {
                throw new Ice.InitializationException("IPv6 only is not supported with SOCKS4 proxies");
            }
            int proxyPort = properties.getPropertyAsIntWithDefault("Ice.SOCKSProxyPort", 1080);
            return new SOCKSNetworkProxy(proxyHost, proxyPort);
        }
        
        proxyHost = properties.getProperty("Ice.HTTPProxyHost");
        if(!proxyHost.isEmpty())
        {
            return new HTTPNetworkProxy(proxyHost, properties.getPropertyAsIntWithDefault("Ice.HTTPProxyPort", 1080));
        }
    
        return null;
    }

    private static final int StateActive = 0;
    private static final int StateDestroyInProgress = 1;
    private static final int StateDestroyed = 2;
    private int _state;

    private final Ice.InitializationData _initData; // Immutable, not reset by destroy().
    private final TraceLevels _traceLevels; // Immutable, not reset by destroy().
    private final DefaultsAndOverrides _defaultsAndOverrides; // Immutable, not reset by destroy().
    private final int _messageSizeMax; // Immutable, not reset by destroy().
    private final int _cacheMessageBuffers; // Immutable, not reset by destroy().
    private final ACMConfig _clientACM; // Immutable, not reset by destroy().
    private final ACMConfig _serverACM; // Immutable, not reset by destroy().
    private final Ice.ImplicitContextI _implicitContext;
    private RouterManager _routerManager;
    private LocatorManager _locatorManager;
    private ReferenceFactory _referenceFactory;
    private ProxyFactory _proxyFactory;
    private OutgoingConnectionFactory _outgoingConnectionFactory;
    private ObjectFactoryManager _servantFactoryManager;
    private ObjectAdapterFactory _objectAdapterFactory;
    private int _protocolSupport;
    private boolean _preferIPv6;
    private NetworkProxy _networkProxy;
    private ThreadPool _clientThreadPool;
    private ThreadPool _serverThreadPool;
    private EndpointHostResolver _endpointHostResolver;
    private RetryQueue _retryQueue;
    private Timer _timer;
    private EndpointFactoryManager _endpointFactoryManager;
    private Ice.PluginManager _pluginManager;

    private boolean _adminEnabled = false;
    private Ice.ObjectAdapter _adminAdapter;
    private java.util.Map<String, Ice.Object> _adminFacets = new java.util.HashMap<String, Ice.Object>();
    private java.util.Set<String> _adminFacetFilter = new java.util.HashSet<String>();
    private Ice.Identity _adminIdentity;

    private java.util.Map<String, String> _typeToClassMap = new java.util.HashMap<String, String>();
    final private String[] _packages;
    final private boolean _useApplicationClassLoader;

    private static boolean _oneOfDone = false;
    private QueueExecutor _queueExecutor;
}
