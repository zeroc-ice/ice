// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

package IceInternal;

public class Instance
{
    public Ice.Properties
    properties()
    {
	// No mutex lock, immutable.
        return _properties;
    }

    public synchronized Ice.Logger
    logger()
    {
        return _logger;
    }

    public synchronized void
    logger(Ice.Logger logger)
    {
        _logger = logger;
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
        return _routerManager;
    }

    public synchronized LocatorManager
    locatorManager()
    {
        return _locatorManager;
    }

    public synchronized ReferenceFactory
    referenceFactory()
    {
        return _referenceFactory;
    }

    public synchronized ProxyFactory
    proxyFactory()
    {
        return _proxyFactory;
    }

    public synchronized OutgoingConnectionFactory
    outgoingConnectionFactory()
    {
        return _outgoingConnectionFactory;
    }

    public synchronized ObjectFactoryManager
    servantFactoryManager()
    {
        return _servantFactoryManager;
    }

    public synchronized UserExceptionFactoryManager
    userExceptionFactoryManager()
    {
        return _userExceptionFactoryManager;
    }

    public synchronized ObjectAdapterFactory
    objectAdapterFactory()
    {
        return _objectAdapterFactory;
    }

    public synchronized ThreadPool
    clientThreadPool()
    {
        if(!_destroyed)
        {
            if(_clientThreadPool == null) // Lazy initialization.
            {
                _clientThreadPool = new ThreadPool(this, false, "ThreadPool.Client");
            }
        }

        return _clientThreadPool;
    }

    public synchronized ThreadPool
    serverThreadPool()
    {
        if(!_destroyed)
        {
            if(_serverThreadPool == null) // Lazy initialization.
            {
                _serverThreadPool = new ThreadPool(this, true, "ThreadPool.Server");
            }
        }

        return _serverThreadPool;
    }

    public synchronized EndpointFactoryManager
    endpointFactoryManager()
    {
        return _endpointFactoryManager;
    }

    public synchronized Ice.PluginManager
    pluginManager()
    {
        return _pluginManager;
    }

    public BufferManager
    bufferManager()
    {
	// No mutex lock, immutable.
        return _bufferManager;
    }

    //
    // Only for use by Ice.CommunicatorI
    //
    public
    Instance(Ice.Communicator communicator, Ice.StringSeqHolder args, Ice.Properties properties)
    {
        _destroyed = false;
        _properties = properties;

        //
        // Convert command-line options beginning with --Ice. to properties.
        //
        args.value = properties.parseCommandLineOptions("Ice", args.value);

        try
        {
	    if(_properties.getPropertyAsInt("Ice.UseSyslog") > 0)
	    {
		_logger = new Ice.SysLoggerI(_properties.getProperty("Ice.ProgramName"));
	    }
	    else
	    {
		_logger = new Ice.LoggerI(_properties.getProperty("Ice.ProgramName"));
	    }

            _traceLevels = new TraceLevels(_properties);

            _defaultsAndOverrides = new DefaultsAndOverrides(_properties);

            _routerManager = new RouterManager();

            _locatorManager = new LocatorManager();

            _referenceFactory = new ReferenceFactory(this);

            _proxyFactory = new ProxyFactory(this);

            _endpointFactoryManager = new EndpointFactoryManager(this);
            EndpointFactory tcpEndpointFactory = new TcpEndpointFactory(this);
            _endpointFactoryManager.add(tcpEndpointFactory);
            EndpointFactory udpEndpointFactory = new UdpEndpointFactory(this);
            _endpointFactoryManager.add(udpEndpointFactory);

            _pluginManager = new Ice.PluginManagerI(communicator);

            _outgoingConnectionFactory = new OutgoingConnectionFactory(this);

            _servantFactoryManager = new ObjectFactoryManager();

            _userExceptionFactoryManager = new UserExceptionFactoryManager();

            _objectAdapterFactory = new ObjectAdapterFactory(this, communicator);

            _bufferManager = new BufferManager(); // Must be created before the ThreadPool
        }
        catch(Ice.LocalException ex)
        {
            destroy();
            throw ex;
        }
    }

    protected void
    finalize()
        throws Throwable
    {
        assert(_destroyed);
        assert(_referenceFactory == null);
        assert(_proxyFactory == null);
        assert(_outgoingConnectionFactory == null);
        assert(_servantFactoryManager == null);
        assert(_userExceptionFactoryManager == null);
        assert(_objectAdapterFactory == null);
        assert(_clientThreadPool == null);
        assert(_serverThreadPool == null);
        assert(_routerManager == null);
        assert(_locatorManager == null);
        assert(_endpointFactoryManager == null);
        assert(_pluginManager == null);

        super.finalize();
    }

    public void
    finishSetup(Ice.StringSeqHolder args)
    {
        //
        // Load plug-ins.
        //
        //pluginManagerImpl = (Ice.PluginManagerI)_pluginManager;
        //pluginManagerImpl.loadPlugins(args);

	//
	// Get default router and locator proxies. Don't move this
	// initialization before the plug-in initialization!!! The proxies
	// might depend on endpoint factories to be installed by plug-ins.
	//
	if(_defaultsAndOverrides.defaultRouter.length() > 0)
	{
	    _referenceFactory.setDefaultRouter(Ice.RouterPrxHelper.uncheckedCast(
		    _proxyFactory.stringToProxy(_defaultsAndOverrides.defaultRouter)));
	}

	if(_defaultsAndOverrides.defaultLocator.length() > 0)
	{
	    _referenceFactory.setDefaultLocator(Ice.LocatorPrxHelper.uncheckedCast(
		    _proxyFactory.stringToProxy(_defaultsAndOverrides.defaultLocator)));
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
	assert(!_destroyed);
	
	_objectAdapterFactory.shutdown();
	_objectAdapterFactory.waitForShutdown();
	
	_outgoingConnectionFactory.destroy();
	_outgoingConnectionFactory.waitUntilFinished();
	
	synchronized(this)
	{
	    _objectAdapterFactory = null;
	    _outgoingConnectionFactory = null;
	    
	    if(_serverThreadPool != null)
	    {
		_serverThreadPool.destroy();
		_serverThreadPool.joinWithAllThreads();
		_serverThreadPool = null;	
	    }
	    
	    if(_clientThreadPool != null)
	    {
		_clientThreadPool.destroy();
		_clientThreadPool.joinWithAllThreads();
		_clientThreadPool = null;
	    }
	    
	    _servantFactoryManager.destroy();
	    _servantFactoryManager = null;
	    
	    _userExceptionFactoryManager.destroy();
	    _userExceptionFactoryManager = null;
	    
	    _referenceFactory.destroy();
	    _referenceFactory = null;
	    
	    // No destroy function defined.
	    // _proxyFactory.destroy();
	    _proxyFactory = null;
	    
	    _routerManager.destroy();
	    _routerManager = null;
	    
	    _locatorManager.destroy();
	    _locatorManager = null;
	    
	    _endpointFactoryManager.destroy();
	    _endpointFactoryManager = null;
	    
	    _pluginManager.destroy();
	    _pluginManager = null;
	    
	    _destroyed = true;
	}
    }

    private boolean _destroyed;
    private Ice.Properties _properties; // Immutable, not reset by destroy().
    private Ice.Logger _logger; // Not reset by destroy().
    private TraceLevels _traceLevels; // Immutable, not reset by destroy().
    private DefaultsAndOverrides _defaultsAndOverrides; // Immutable, not reset by destroy().
    private RouterManager _routerManager;
    private LocatorManager _locatorManager;
    private ReferenceFactory _referenceFactory;
    private ProxyFactory _proxyFactory;
    private OutgoingConnectionFactory _outgoingConnectionFactory;
    private ObjectFactoryManager _servantFactoryManager;
    private UserExceptionFactoryManager _userExceptionFactoryManager;
    private ObjectAdapterFactory _objectAdapterFactory;
    private ThreadPool _clientThreadPool;
    private ThreadPool _serverThreadPool;
    private EndpointFactoryManager _endpointFactoryManager;
    private Ice.PluginManager _pluginManager;
    private BufferManager _bufferManager; // Immutable, not reset by destroy().
}
