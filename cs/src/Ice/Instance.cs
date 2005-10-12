// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{

    using System.Diagnostics;

    public sealed class Instance
    {
	public bool destroyed()
	{
	    return _state == StateDestroyed;
	}

	public Ice.Properties properties()
	{
	    //
	    // No check for destruction. It must be possible to access the
	    // properties after destruction.
	    //
	    // No mutex lock, immutable.
	    //
	    return _properties;
	}
	
	public Ice.Logger logger()
	{
	    lock(this)
	    {
		//
		// No check for destruction. It must be possible to access the
		// logger after destruction.
		//
		return _logger;
	    }
	}
	
	public void logger(Ice.Logger logger)
	{
	    lock(this)
	    {
		//
		// No check for destruction. It must be possible to set the
		// logger after destruction (needed by logger plugins for
		// example to unset the logger).
		//
		_logger = logger;
	    }
	}
	
	public Ice.Stats stats()
	{
	    lock(this)
	    {
		if(_state == StateDestroyed)
		{
		    throw new Ice.CommunicatorDestroyedException();
		}
		
		return _stats;
	    }
	}
	
	public void stats(Ice.Stats stats)
	{
	    lock(this)
	    {
		if(_state == StateDestroyed)
		{
			throw new Ice.CommunicatorDestroyedException();
		}
		
		_stats = stats;
	    }
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
		
		if(_serverThreadPool == null)
		// Lazy initialization.
		{
		    int timeout = _properties.getPropertyAsInt("Ice.ServerIdleTime");
		    _serverThreadPool = new ThreadPool(this, "Ice.ThreadPool.Server", timeout);
		}
		
		return _serverThreadPool;
	    }
	}

	public bool threadPerConnection()
	{
	    // No mutex lock, immutable.
	    return _threadPerConnection;
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
	
        public void setDefaultContext(Ice.Context ctx)
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
		    _defaultContext = (Ice.Context)ctx.Clone();
		}
	    }
	}

	public Ice.Context getDefaultContext()
	{
	    lock(this)
	    {
		if(_state == StateDestroyed)
		{
		    throw new Ice.CommunicatorDestroyedException();
		}
		
		return (Ice.Context)_defaultContext.Clone();
	    }
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
	
	//
	// Only for use by Ice.CommunicatorI
	//
	public Instance(Ice.Communicator communicator, Ice.Properties properties, Ice.Logger logger)
	{
	    _state = StateActive;
	    _properties = properties;
	    _logger = logger;
	        
	    try
	    {
		lock(_staticLock)
		{
		    if(!_oneOffDone)
		    {
			string stdOut = _properties.getProperty("Ice.StdOut");
			string stdErr = _properties.getProperty("Ice.StdErr");
			
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
		
    	    	if(_logger == null)
		{
		    if(_properties.getPropertyAsInt("Ice.UseSyslog") > 0)
		    {
			_logger = new Ice.SysLoggerI(_properties.getProperty("Ice.ProgramName"));
		    }
		    else
		    {
			_logger = new Ice.LoggerI(_properties.getProperty("Ice.ProgramName"),
						  _properties.getPropertyAsInt("Ice.Logger.Timestamp") > 0);
		    }
		}
		
		_stats = null; // There is no default statistics callback object.
		
		_traceLevels = new TraceLevels(_properties);
		
		_defaultsAndOverrides = new DefaultsAndOverrides(_properties);
		
		{
		    const int defaultMessageSizeMax = 1024;
		    int num = _properties.getPropertyAsIntWithDefault("Ice.MessageSizeMax", defaultMessageSizeMax);
		    if(num < 1)
		    {
			_messageSizeMax = defaultMessageSizeMax * 1024; // Ignore stupid values.
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
		
		{
		    int clientACMDefault = 60; // Client ACM enabled by default.
		    int serverACMDefault = 0; // Server ACM disabled by default.
		    
		    //
		    // Legacy: If Ice.ConnectionIdleTime is set, we use it as
		    // default value for both the client- and server-side ACM.
		    //
		    if(_properties.getProperty("Ice.ConnectionIdleTime").Length > 0)
		    {
			int num = _properties.getPropertyAsInt("Ice.ConnectionIdleTime");
			clientACMDefault = num;
			serverACMDefault = num;
		    }
		    
		    _clientACM = _properties.getPropertyAsIntWithDefault("Ice.ACM.Client", clientACMDefault);
		    _serverACM = _properties.getPropertyAsIntWithDefault("Ice.ACM.Server", serverACMDefault);
		}

		_threadPerConnection = _properties.getPropertyAsInt("Ice.ThreadPerConnection") > 0;

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
	
#if DEBUG
	~Instance()
	{
            lock(this)
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
	    }
	}
#endif
	
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
	    if(_defaultsAndOverrides.defaultRouter.Length > 0)
	    {
		_referenceFactory.setDefaultRouter(Ice.RouterPrxHelper.uncheckedCast(
			_proxyFactory.stringToProxy(_defaultsAndOverrides.defaultRouter)));
	    }
	    
	    if(_defaultsAndOverrides.defaultLocator.Length > 0)
	    {
		_referenceFactory.setDefaultLocator(Ice.LocatorPrxHelper.uncheckedCast(
			_proxyFactory.stringToProxy(_defaultsAndOverrides.defaultLocator)));
	    }
	    
	    //
	    // Show process id if requested (but only once).
	    //
	    lock(this)
	    {
		if(!_printProcessIdDone && _properties.getPropertyAsInt("Ice.PrintProcessId") > 0)
		{
		    using(Process p = Process.GetCurrentProcess())
		    {
			System.Console.WriteLine(p.Id);
		    }
		    _printProcessIdDone = true;
		}
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
	    interval = _properties.getPropertyAsIntWithDefault("Ice.MonitorConnections", interval);
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
		_objectAdapterFactory.waitForShutdown();
	    }
	    
	    if(_outgoingConnectionFactory != null)
	    {
		_outgoingConnectionFactory.waitUntilFinished();
	    }
	    
	    ThreadPool serverThreadPool = null;
	    ThreadPool clientThreadPool = null;
	    
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

	    return true;
	}
	
	private const int StateActive = 0;
	private const int StateDestroyInProgress = 1;
	private const int StateDestroyed = 2;
	private int _state;
	private Ice.Properties _properties; // Immutable, not reset by destroy().
	private Ice.Logger _logger; // Not reset by destroy().
	private Ice.Stats _stats; // Not reset by destroy().
	private TraceLevels _traceLevels; // Immutable, not reset by destroy().
	private DefaultsAndOverrides _defaultsAndOverrides; // Immutable, not reset by destroy().
	private int _messageSizeMax; // Immutable, not reset by destroy().
	private int _clientACM; // Immutable, not reset by destroy().
	private int _serverACM; // Immutable, not reset by destroy().
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
	private bool _threadPerConnection;
	private EndpointFactoryManager _endpointFactoryManager;
	private Ice.PluginManager _pluginManager;
	private Ice.Context _defaultContext;
	private static Ice.Context _emptyContext = new Ice.Context();
	private static bool _printProcessIdDone = false;

        private static bool _oneOffDone = false;
        private static System.Object _staticLock = new System.Object();

    }

}
