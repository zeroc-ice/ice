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

    public synchronized ProxyFactory
    proxyFactory()
    {
        return _proxyFactory;
    }

    public synchronized EmitterFactory
    emitterFactory()
    {
        return _emitterFactory;
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

    /* TODO: Server
    public synchronized ObjectAdapterFactory
    objectAdapterFactory()
    {
        return _objectAdapterFactory;
    }
    */

    public synchronized ThreadPool
    threadPool()
    {
        return _threadPool;
    }

    public synchronized BufferManager
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
            _proxyFactory = new ProxyFactory(this);
            _emitterFactory = new EmitterFactory(this);
            _servantFactoryManager = new ObjectFactoryManager();
            _userExceptionFactoryManager = new UserExceptionFactoryManager();
            /* TODO: Server
            _objectAdapterFactory = new ObjectAdapterFactory(this);
            */
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
        assert(_proxyFactory == null);
        assert(_emitterFactory == null);
        assert(_servantFactoryManager == null);
        assert(_userExceptionFactoryManager == null);
        /* TODO: Server
        assert(_objectAdapterFactory == null);
        */
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
            // this object, not the other way
            _communicator = null;
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

        if (_proxyFactory != null)
        {
            // No destroy function defined
            // _proxyFactory.destroy();
            _proxyFactory = null;
        }

        if (_emitterFactory != null)
        {
            _emitterFactory.destroy();
            _emitterFactory = null;
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

        /* TODO: Server
        if (_objectAdapterFactory != null)
        {
            _objectAdapterFactory.shutdown(); // ObjectAdapterFactory has
                                              // shutdown(), not destroy()
            _objectAdapterFactory = null;
        }
        */

        if (_threadPool != null)
        {   
            _threadPool.waitUntilFinished();
            _threadPool.destroy();
            _threadPool.joinWithAllThreads();
            _threadPool = null;
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
    private ProxyFactory _proxyFactory;
    private EmitterFactory _emitterFactory;
    private ObjectFactoryManager _servantFactoryManager;
    private UserExceptionFactoryManager _userExceptionFactoryManager;
    /* TODO: Server
    private ObjectAdapterFactory _objectAdapterFactory;
    */
    private ThreadPool _threadPool;
    private BufferManager _bufferManager;
}
