// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package IceInternal;

public class Instance
{
    public synchronized Ice.Communicator
    communicator()
    {
        return _communicator;
    }

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

    public String
    defaultProtocol()
    {
	// No mutex lock, immutable.
        return _defaultProtocol;
    }

    public String
    defaultHost()
    {
	// No mutex lock, immutable.
        return _defaultHost;
    }

    public synchronized RouterManager
    routerManager()
    {
        return _routerManager;
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
        if (_communicator != null) // Not destroyed?
        {
            if (_clientThreadPool == null) // Lazy initialization.
            {
                _clientThreadPool = new ThreadPool(this, false);
            }
        }

        return _clientThreadPool;
    }

    public synchronized ThreadPool
    serverThreadPool()
    {
        if (_communicator != null) // Not destroyed?
        {
            if (_serverThreadPool == null) // Lazy initialization.
            {
                _serverThreadPool = new ThreadPool(this, false);
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
        _communicator = communicator;
        _properties = properties;

        //
        // Convert command-line options beginning with --Ice. to properties.
        //
        args.value = properties.parseCommandLineOptions("Ice", args.value);

        try
        {
            _logger = new Ice.LoggerI();
            _traceLevels = new TraceLevels(_properties);
            _defaultProtocol = _properties.getPropertyWithDefault("Ice.DefaultProtocol", "tcp");
            _defaultHost = _properties.getProperty("Ice.DefaultHost");
	    if (_defaultHost.length() == 0)
	    {
		_defaultHost = Network.getLocalHost(true);
	    }	    
            _routerManager = new RouterManager();
            _referenceFactory = new ReferenceFactory(this);
            _proxyFactory = new ProxyFactory(this);

            //
            // Install TCP and UDP endpoint factories.
            //
            _endpointFactoryManager = new EndpointFactoryManager(this);
            EndpointFactory tcpEndpointFactory = new TcpEndpointFactory(this);
            _endpointFactoryManager.add(tcpEndpointFactory);
            EndpointFactory udpEndpointFactory = new UdpEndpointFactory(this);
            _endpointFactoryManager.add(udpEndpointFactory);

            _pluginManager = new Ice.PluginManagerI(this);

            String router = _properties.getProperty("Ice.DefaultRouter");
            if (router.length() > 0)
            {
                _referenceFactory.setDefaultRouter(
		    Ice.RouterPrxHelper.uncheckedCast(_proxyFactory.stringToProxy(router)));
            }
            _outgoingConnectionFactory = new OutgoingConnectionFactory(this);
            _servantFactoryManager = new ObjectFactoryManager();
            _userExceptionFactoryManager = new UserExceptionFactoryManager();
            _objectAdapterFactory = new ObjectAdapterFactory(this);
            _bufferManager = new BufferManager(); // Must be created before the ThreadPool
        }
        catch (Ice.LocalException ex)
        {
            destroy();
            throw ex;
        }
    }

    protected void
    finalize()
        throws Throwable
    {
        assert(_communicator == null);
        assert(_referenceFactory == null);
        assert(_proxyFactory == null);
        assert(_outgoingConnectionFactory == null);
        assert(_servantFactoryManager == null);
        assert(_userExceptionFactoryManager == null);
        assert(_objectAdapterFactory == null);
        assert(_clientThreadPool == null);
        assert(_serverThreadPool == null);
        assert(_routerManager == null);
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
        ThreadPool clientThreadPool;
        ThreadPool serverThreadPool;
        Ice.PluginManager pluginManager;

	synchronized(this)
	{
	    //
	    // Destroy all contained objects. Then set all references to null,
	    // to avoid cyclic object dependencies.
	    //
	    
	    if (_communicator != null)
	    {
		// Don't destroy the communicator -- the communicator destroys
		// this object, not the other way.
		_communicator = null;
	    }

	    if (_objectAdapterFactory != null)
	    {
		// Don't shut down the object adapters -- the communicator
		// must do this before it destroys this object.
		_objectAdapterFactory = null;
	    }
	    
	    if (_servantFactoryManager != null)
	    {
		_servantFactoryManager.destroy();
		_servantFactoryManager = null;
	    }
	    
	    if (_userExceptionFactoryManager != null)
	    {
		_userExceptionFactoryManager.destroy();
		_userExceptionFactoryManager = null;
	    }
	    
	    if (_referenceFactory != null)
	    {
		_referenceFactory.destroy();
		_referenceFactory = null;
	    }
	    
	    if (_proxyFactory != null)
	    {
		// No destroy function defined
		// _proxyFactory.destroy();
		_proxyFactory = null;
	    }
	    
	    if (_outgoingConnectionFactory != null)
	    {
		_outgoingConnectionFactory.destroy();
		_outgoingConnectionFactory = null;
	    }

	    if (_routerManager != null)
	    {
		_routerManager.destroy();
		_routerManager = null;
	    }

            if (_endpointFactoryManager != null)
            {
                _endpointFactoryManager.destroy();
                _endpointFactoryManager = null;
            }

            //
            // We destroy the thread pool outside the thread
            // synchronization.
            //  
            clientThreadPool = _clientThreadPool;
            _clientThreadPool = null;
            serverThreadPool = _serverThreadPool;
            _serverThreadPool = null;
            
            //  
            // We destroy the plugin manager after the thread pools.
            //  
            pluginManager = _pluginManager;
            _pluginManager = null;
        }   

        if (clientThreadPool != null)
        {       
            clientThreadPool.waitUntilFinished();
            clientThreadPool.destroy();
            clientThreadPool.joinWithAllThreads();
        }   

        if (serverThreadPool != null)
        {   
            serverThreadPool.waitUntilFinished();
            serverThreadPool.destroy();
            serverThreadPool.joinWithAllThreads();
        }

        if (pluginManager != null)
        {
            pluginManager.destroy();
        }
    }

    private Ice.Communicator _communicator;
    private Ice.Properties _properties; // Immutable, not reset by destroy().
    private Ice.Logger _logger; // Not reset by destroy().
    private TraceLevels _traceLevels; // Immutable, not reset by destroy().
    private RouterManager _routerManager;
    private ReferenceFactory _referenceFactory;
    private ProxyFactory _proxyFactory;
    private OutgoingConnectionFactory _outgoingConnectionFactory;
    private ObjectFactoryManager _servantFactoryManager;
    private UserExceptionFactoryManager _userExceptionFactoryManager;
    private ObjectAdapterFactory _objectAdapterFactory;
    private ThreadPool _clientThreadPool;
    private ThreadPool _serverThreadPool;
    private String _defaultProtocol; // Immutable, not reset by destroy().
    private String _defaultHost; // Immutable, not reset by destroy().
    private EndpointFactoryManager _endpointFactoryManager;
    private Ice.PluginManager _pluginManager;
    private BufferManager _bufferManager; // Immutable, not reset by destroy().
}
