// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public final class Instance
{
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
        return _traceLevels;
    }

    public DefaultsAndOverrides
    defaultsAndOverrides()
    {
        // No mutex lock, immutable.
        return _defaultsAndOverrides;
    }

    public synchronized RouterManager
    routerManager()
    {
        if(_state == StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        return _routerManager;
    }

    public synchronized LocatorManager
    locatorManager()
    {
        if(_state == StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        return _locatorManager;
    }

    public synchronized ReferenceFactory
    referenceFactory()
    {
        if(_state == StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        return _referenceFactory;
    }

    public synchronized ProxyFactory
    proxyFactory()
    {
        if(_state == StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        return _proxyFactory;
    }

    public synchronized OutgoingConnectionFactory
    outgoingConnectionFactory()
    {
        if(_state == StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        return _outgoingConnectionFactory;
    }

    public synchronized ConnectionMonitor
    connectionMonitor()
    {
        if(_state == StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        return _connectionMonitor;
    }

    public synchronized ObjectFactoryManager
    servantFactoryManager()
    {
        if(_state == StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        return _servantFactoryManager;
    }

    public synchronized ObjectAdapterFactory
    objectAdapterFactory()
    {
        if(_state == StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        return _objectAdapterFactory;
    }

    public synchronized ThreadPool
    clientThreadPool()
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

    public synchronized ThreadPool
    serverThreadPool()
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

    public boolean
    threadPerConnection()
    {
        return _threadPerConnection;
    }

    public int
    threadPerConnectionStackSize()
    {
        return _threadPerConnectionStackSize;
    }

    public synchronized EndpointFactoryManager
    endpointFactoryManager()
    {
        if(_state == StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        return _endpointFactoryManager;
    }

    public synchronized Ice.PluginManager
    pluginManager()
    {
        if(_state == StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        return _pluginManager;
    }

    public int
    messageSizeMax()
    {
        // No mutex lock, immutable.
        return _messageSizeMax;
    }

    public int
    clientACM()
    {
        // No mutex lock, immutable.
        return _clientACM;
    }

    public int
    serverACM()
    {
        // No mutex lock, immutable.
        return _serverACM;
    }

    public synchronized void
    setDefaultContext(java.util.Map ctx)
    {
        if(_state == StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        if(ctx == null || ctx.isEmpty())
        {
            _defaultContext = _emptyContext;
        }
        else
        {
            _defaultContext = new java.util.HashMap(ctx);
        }
    }

    public synchronized java.util.Map
    getDefaultContext()
    {
        if(_state == StateDestroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        return new java.util.HashMap(_defaultContext);
    }

    public Ice.ImplicitContextI
    getImplicitContext()
    {
        return _implicitContext;
    }


    public void
    flushBatchRequests()
    {
        OutgoingConnectionFactory connectionFactory;
        ObjectAdapterFactory adapterFactory;

        synchronized(this)
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
                if(!_oneOffDone)
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
                            Ice.FileException fe = new Ice.FileException();
                            fe.path = stdOut;
                            fe.initCause(ex);
                            throw fe;
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
                                Ice.FileException fe = new Ice.FileException();
                                fe.path = stdErr;
                                fe.initCause(ex);
                                throw fe;
                            }

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

            validatePackages();

            _traceLevels = new TraceLevels(_initData.properties);

            _defaultsAndOverrides = new DefaultsAndOverrides(_initData.properties);

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

            //
            // Client ACM enabled by default. Server ACM disabled by default.
            //
            _clientACM = _initData.properties.getPropertyAsIntWithDefault("Ice.ACM.Client", 60);
            _serverACM = _initData.properties.getPropertyAsInt("Ice.ACM.Server");

            _implicitContext = Ice.ImplicitContextI.create(
                _initData.properties.getProperty("Ice.ImplicitContext"));
            

            _threadPerConnection = _initData.properties.getPropertyAsInt("Ice.ThreadPerConnection") > 0;

            {
                int stackSize = _initData.properties.getPropertyAsInt("Ice.ThreadPerConnection.StackSize");
                if(stackSize < 0)
                {
                    stackSize = 0;
                }
                _threadPerConnectionStackSize = stackSize;
            }

            _routerManager = new RouterManager();

            _locatorManager = new LocatorManager();

            _referenceFactory = new ReferenceFactory(this, communicator);

            _proxyFactory = new ProxyFactory(this);

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
        }
        catch(Ice.LocalException ex)
        {
            destroy();
            throw ex;
        }
    }

    protected synchronized void
    finalize()
        throws Throwable
    {
        IceUtil.Assert.FinalizerAssert(_state == StateDestroyed);
        IceUtil.Assert.FinalizerAssert(_referenceFactory == null);
        IceUtil.Assert.FinalizerAssert(_proxyFactory == null);
        IceUtil.Assert.FinalizerAssert(_outgoingConnectionFactory == null);
        IceUtil.Assert.FinalizerAssert(_connectionMonitor == null);
        IceUtil.Assert.FinalizerAssert(_servantFactoryManager == null);
        IceUtil.Assert.FinalizerAssert(_objectAdapterFactory == null);
        IceUtil.Assert.FinalizerAssert(_clientThreadPool == null);
        IceUtil.Assert.FinalizerAssert(_serverThreadPool == null);
        IceUtil.Assert.FinalizerAssert(_routerManager == null);
        IceUtil.Assert.FinalizerAssert(_locatorManager == null);
        IceUtil.Assert.FinalizerAssert(_endpointFactoryManager == null);
        IceUtil.Assert.FinalizerAssert(_pluginManager == null);

        super.finalize();
    }

    public void
    finishSetup(Ice.StringSeqHolder args)
    {
        //
        // Load plug-ins.
        //
        Ice.PluginManagerI pluginManagerImpl = (Ice.PluginManagerI)_pluginManager;
        pluginManagerImpl.loadPlugins(args);
        Ice.Logger logger = pluginManagerImpl.getLogger();
        if(logger != null)
        {
            _initData.logger = logger;
        }

        //
        // Get default router and locator proxies. Don't move this
        // initialization before the plug-in initialization!!! The proxies
        // might depend on endpoint factories to be installed by plug-ins.
        //
        _referenceFactory.setDefaultRouter(Ice.RouterPrxHelper.uncheckedCast(
            _proxyFactory.propertyToProxy("Ice.Default.Router")));

        _referenceFactory.setDefaultLocator(Ice.LocatorPrxHelper.uncheckedCast(
            _proxyFactory.propertyToProxy("Ice.Default.Locator")));
        
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
    public void
    destroy()
    {
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
            _outgoingConnectionFactory.waitUntilFinished();
        }
        
        ThreadPool serverThreadPool = null;
        ThreadPool clientThreadPool = null;

        synchronized(this)
        {
            _objectAdapterFactory = null;

            _outgoingConnectionFactory = null;

            if(_connectionMonitor != null)
            {
                _connectionMonitor._destroy();
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
            
            _state = StateDestroyed;
        }

        //
        // Join with the thread pool threads outside the
        // synchronization.
        //
        if(clientThreadPool != null)
        {
            clientThreadPool.joinWithAllThreads();
        }
        if(serverThreadPool != null)
        {
            serverThreadPool.joinWithAllThreads();
        }

        if(_initData.properties.getPropertyAsInt("Ice.Warn.UnusedProperties") > 0)
        {
            java.util.List unusedProperties = ((Ice.PropertiesI)_initData.properties).getUnusedProperties();
            if(unusedProperties.size() != 0)
            {
                String message = "The following properties were set but never read:";
                java.util.Iterator p = unusedProperties.iterator();
                while(p.hasNext())
                {
                    message += "\n    " + (String)p.next();
                }
                _initData.logger.warning(message);
            }
        }
    }

    private void
    validatePackages()
    {
        final String prefix = "Ice.Package.";
        java.util.Map map = _initData.properties.getPropertiesForPrefix(prefix);
        java.util.Iterator p = map.entrySet().iterator();
        while(p.hasNext())
        {
            java.util.Map.Entry e = (java.util.Map.Entry)p.next();
            String key = (String)e.getKey();
            String pkg = (String)e.getValue();
            if(key.length() == prefix.length())
            {
                _initData.logger.warning("ignoring invalid property: " + key + "=" + pkg);
            }
            String module = key.substring(prefix.length());
            String className = pkg + "." + module + "._Marker";
            try
            {
                Class.forName(className);
            }
            catch(java.lang.Exception ex)
            {
                _initData.logger.warning("unable to validate package: " + key + "=" + pkg);
            }
        }
    }

    private static final int StateActive = 0;
    private static final int StateDestroyInProgress = 1;
    private static final int StateDestroyed = 2;
    private int _state;

    private final Ice.InitializationData _initData; // Immutable, not reset by destroy().
    private final TraceLevels _traceLevels; // Immutable, not reset by destroy().
    private final DefaultsAndOverrides _defaultsAndOverrides; // Immutable, not reset by destroy().
    private final int _messageSizeMax; // Immutable, not reset by destroy().
    private final int _clientACM; // Immutable, not reset by destroy().
    private final int _serverACM; // Immutable, not reset by destroy().
    private final Ice.ImplicitContextI _implicitContext;
    private RouterManager _routerManager;
    private LocatorManager _locatorManager;
    private ReferenceFactory _referenceFactory;
    private ProxyFactory _proxyFactory;
    private OutgoingConnectionFactory _outgoingConnectionFactory;
    private ConnectionMonitor _connectionMonitor;
    private ObjectFactoryManager _servantFactoryManager;
    private ObjectAdapterFactory _objectAdapterFactory;
    private ThreadPool _clientThreadPool;
    private ThreadPool _serverThreadPool;
    private final boolean _threadPerConnection;
    private final int _threadPerConnectionStackSize;
    private EndpointFactoryManager _endpointFactoryManager;
    private Ice.PluginManager _pluginManager;
    private java.util.Map _defaultContext;
 
    private static java.util.Map _emptyContext = new java.util.HashMap();

    private static boolean _oneOffDone = false;
}
