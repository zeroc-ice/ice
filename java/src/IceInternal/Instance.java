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
    threadPool()
    {
        return _threadPool;
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
    Instance(Ice.Communicator communicator, Ice.Properties properties)
    {
        _communicator = communicator;
        _properties = properties;

        try
        {
            _logger = new Ice.LoggerI();
            _traceLevels = new TraceLevels(_properties);
            _defaultProtocol = _properties.getProperty("Ice.DefaultProtocol");
            if (_defaultProtocol.length() == 0)
            {
                _defaultProtocol = "tcp";
            }
            _defaultHost = _properties.getProperty("Ice.DefaultHost");
            if (_defaultHost.length() == 0)
            {
                _defaultHost = Network.getLocalHost(true);
            }
            _routerManager = new RouterManager();
            _referenceFactory = new ReferenceFactory(this);
            _proxyFactory = new ProxyFactory(this);
            String value = _properties.getProperty("Ice.DefaultRouter");
            if (value.length() > 0)
            {
                _referenceFactory.setDefaultRouter(
                    Ice.RouterPrxHelper.uncheckedCast(_proxyFactory.stringToProxy(value)));
            }
            _outgoingConnectionFactory = new OutgoingConnectionFactory(this);
            _servantFactoryManager = new ObjectFactoryManager();
            _userExceptionFactoryManager = new UserExceptionFactoryManager();
            _objectAdapterFactory = new ObjectAdapterFactory(this);
            _bufferManager = new BufferManager(); // Must be created before the ThreadPool
            _threadPool = new ThreadPool(this);
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
        assert(_threadPool == null);
        assert(_routerManager == null);

        super.finalize();
    }

    //
    // Only for use by Ice.CommunicatorI
    //
    public void
    destroy()
    {
	ThreadPool threadPool;

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

	    //
	    // We destroy the thread pool outside the thread
	    // synchronization.
	    //
	    threadPool = _threadPool;
	    _threadPool = null;
	}
	
        if (threadPool != null)
        {   
            threadPool.waitUntilFinished();
            threadPool.destroy();
            threadPool.joinWithAllThreads();
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
    private ThreadPool _threadPool;
    private String _defaultProtocol; // Immutable, not reset by destroy().
    private String _defaultHost; // Immutable, not reset by destroy().
    private BufferManager _bufferManager; // Immutable, not reset by destroy().
}
