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

    public synchronized Ice.Properties
    properties()
    {
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

    public synchronized TraceLevels
    traceLevels()
    {
        return _traceLevels;
    }

    public String
    defaultProtocol()
    {
        // No synchronization necessary
        return _defaultProtocol;
    }

    public String
    defaultHost()
    {
        // No synchronization necessary
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

    //
    // TODO: This should be synchronized, but it causes a deadlock
    // on shutdown if a BasicStream is created while the Instance
    // is already locked (e.g., in ThreadPool)
    //
    public /*synchronized*/ BufferManager
    bufferManager()
    {
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
            if (_defaultProtocol == null)
            {
                _defaultProtocol = "tcp";
            }
            _defaultHost = _properties.getProperty("Ice.DefaultHost");
            if (_defaultHost == null)
            {
                _defaultHost = Network.getLocalHost(true);
            }
            _routerManager = new RouterManager();
            _referenceFactory = new ReferenceFactory(this);
            _proxyFactory = new ProxyFactory(this);
            String value = _properties.getProperty("Ice.DefaultRouter");
            if (value != null)
            {
                _referenceFactory.setDefaultRouter(
                    Ice.RouterPrxHelper.uncheckedCast(
                        _proxyFactory.stringToProxy(value)));
            }
            _outgoingConnectionFactory = new OutgoingConnectionFactory(this);
            _servantFactoryManager = new ObjectFactoryManager();
            _userExceptionFactoryManager = new UserExceptionFactoryManager();
            _objectAdapterFactory = new ObjectAdapterFactory(this);
            _threadPool = new ThreadPool(this);
            _bufferManager = new BufferManager();
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
        assert(_properties == null);
        assert(_logger == null);
        assert(_traceLevels == null);
        assert(_routerManager == null);
        assert(_referenceFactory == null);
        assert(_proxyFactory == null);
        assert(_outgoingConnectionFactory == null);
        assert(_servantFactoryManager == null);
        assert(_userExceptionFactoryManager == null);
        assert(_objectAdapterFactory == null);
        assert(_threadPool == null);

        super.finalize();
    }

    //
    // Only for use by Ice.CommunicatorI
    //
    public synchronized void
    destroy()
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
            _objectAdapterFactory.shutdown(); // ObjectAdapterFactory has
                                              // shutdown(), not destroy()
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

        if (_threadPool != null)
        {   
            _threadPool.waitUntilFinished();
            _threadPool.destroy();
            _threadPool.joinWithAllThreads();
            _threadPool = null;
        }

        if (_properties != null)
        {
            // No destroy function defined
            // _properties.destroy();
            _properties = null;
        }

        if (_logger != null)
        {
            _logger.destroy();
            _logger = null;
        }

        if (_traceLevels != null)
        {
            // No destroy function defined
            // _traceLevels.destroy();
            _traceLevels = null;
        }

        if (_bufferManager != null)
        {
            _bufferManager.destroy();
            _bufferManager = null;
        }
    }

    private Ice.Communicator _communicator;
    private Ice.Properties _properties;
    private Ice.Logger _logger;
    private TraceLevels _traceLevels;
    private RouterManager _routerManager;
    private ReferenceFactory _referenceFactory;
    private ProxyFactory _proxyFactory;
    private OutgoingConnectionFactory _outgoingConnectionFactory;
    private ObjectFactoryManager _servantFactoryManager;
    private UserExceptionFactoryManager _userExceptionFactoryManager;
    private ObjectAdapterFactory _objectAdapterFactory;
    private ThreadPool _threadPool;
    private String _defaultProtocol;
    private String _defaultHost;
    private BufferManager _bufferManager;
}
