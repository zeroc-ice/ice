// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public final class CommunicatorI extends LocalObjectImpl implements Communicator
{
    public void
    destroy()
    {
	IceInternal.Instance instance = null;

	synchronized(this)
	{
	    if(_destroyed) // Don't destroy twice.
	    {
		return;
	    }
	    _destroyed = true;
	    instance = _instance;
	}

	if(instance != null)
	{
	    instance.destroy();
	}
    }

    public void
    shutdown()
    {
	IceInternal.ObjectAdapterFactory objectAdapterFactory;
	
	synchronized(this)
	{
	    if(_destroyed)
	    {
		throw new CommunicatorDestroyedException();
	    }
	    objectAdapterFactory = _instance.objectAdapterFactory();
	}
	
	//
	// We must call shutdown on the object adapter factory
	// outside the synchronization, otherwise the communicator is
	// blocked while we wait for shutdown.
	//
	objectAdapterFactory.shutdown();
    }

    public void
    waitForShutdown()
    {
	IceInternal.ObjectAdapterFactory objectAdapterFactory;
	
	synchronized(this)
	{
	    if(_destroyed)
	    {
		throw new CommunicatorDestroyedException();
	    }
	    objectAdapterFactory = _instance.objectAdapterFactory();
	}
	
	//
	// We must call waitForShutdown on the object adapter factory
	// outside the synchronization, otherwise the communicator is
	// blocked while we wait for shutdown.
	//
	objectAdapterFactory.waitForShutdown();
    }

    public synchronized Ice.ObjectPrx
    stringToProxy(String s)
    {
        if(_destroyed)
        {
            throw new CommunicatorDestroyedException();
        }
        return _instance.proxyFactory().stringToProxy(s);
    }

    public synchronized String
    proxyToString(Ice.ObjectPrx proxy)
    {
        if(_destroyed)
        {
            throw new CommunicatorDestroyedException();
        }
        return _instance.proxyFactory().proxyToString(proxy);
    }

    public synchronized ObjectAdapter
    createObjectAdapter(String name)
    {
        if(_destroyed)
        {
            throw new CommunicatorDestroyedException();
        }

	return _instance.objectAdapterFactory().createObjectAdapter(name);
    }

    public ObjectAdapter
    createObjectAdapterWithEndpoints(String name, String endpoints)
    {
	final String propertyKey = new String(name + ".Endpoints");
	final String originalValue = getProperties().getProperty(propertyKey);
	try
	{
	    getProperties().setProperty(propertyKey, endpoints);
	    return createObjectAdapter(name);
	}
	catch(AlreadyRegisteredException ex)
	{
	    if(originalValue.length() != 0)
	    {
		getProperties().setProperty(propertyKey, originalValue);
	    }
	    throw ex;
	}
    }

    public synchronized void
    addObjectFactory(ObjectFactory factory, String id)
    {
        if(_destroyed)
        {
            throw new CommunicatorDestroyedException();
        }
        _instance.servantFactoryManager().add(factory, id);
    }

    public synchronized void
    removeObjectFactory(String id)
    {
        if(_destroyed)
        {
            throw new CommunicatorDestroyedException();
        }
        _instance.servantFactoryManager().remove(id);
    }

    public synchronized ObjectFactory
    findObjectFactory(String id)
    {
        if(_destroyed)
        {
            throw new CommunicatorDestroyedException();
        }
        return _instance.servantFactoryManager().find(id);
    }

    public Properties
    getProperties()
    {
	//
	// No check for destruction. It must be possible to access the
	// properties after destruction.
	//
        return _instance.properties();
    }

    public Logger
    getLogger()
    {
	//
	// No check for destruction. It must be possible to access the
	// logger after destruction.
	//
        return _instance.logger();
    }

    public void
    setLogger(Logger logger)
    {
	//
	// No check for destruction. It must be possible to set the
	// logger after destruction (needed by logger plugins for
	// example to unset the logger).
	//
        _instance.logger(logger);
    }

    public synchronized Stats
    getStats()
    {
        if(_destroyed)
        {
            throw new CommunicatorDestroyedException();
        }
        return _instance.stats();
    }

    public synchronized void
    setStats(Stats stats)
    {
        if(_destroyed)
        {
            throw new CommunicatorDestroyedException();
        }
        _instance.stats(stats);
    }

    public synchronized RouterPrx
    getDefaultRouter()
    {
        if(_destroyed)
        {
            throw new CommunicatorDestroyedException();
        }
        return _instance.referenceFactory().getDefaultRouter();
    }

    public synchronized void
    setDefaultRouter(RouterPrx router)
    {
        if(_destroyed)
        {
            throw new CommunicatorDestroyedException();
        }
        _instance.referenceFactory().setDefaultRouter(router);
    }

    public synchronized LocatorPrx
    getDefaultLocator()
    {
        if(_destroyed)
        {
            throw new CommunicatorDestroyedException();
        }
        return _instance.referenceFactory().getDefaultLocator();
    }

    public synchronized void
    setDefaultLocator(LocatorPrx locator)
    {
        if(_destroyed)
        {
            throw new CommunicatorDestroyedException();
        }
        _instance.referenceFactory().setDefaultLocator(locator);
    }

    public synchronized java.util.Map
    getDefaultContext()
    {
        if(_destroyed)
        {
            throw new CommunicatorDestroyedException();
        }
	return _instance.getDefaultContext();
    }

    public synchronized void
    setDefaultContext(java.util.Map ctx)
    {
        if(_destroyed)
        {
            throw new CommunicatorDestroyedException();
        }
	_instance.setDefaultContext(ctx);
    }

    public PluginManager
    getPluginManager()
    {
        return null;
    }

    public void
    flushBatchRequests()
    {
        _instance.flushBatchRequests();
    }

    CommunicatorI(Properties properties)
    {
	_destroyed = false;
        _instance = new IceInternal.Instance(this, properties);
    }

    /**
      * For compatibility with C#, we do not invoke methods on other objects
      * from within a finalizer.
      *
    protected synchronized void
    finalize()
        throws Throwable
    {
	if(!_destroyed)
	{
	    _instance.logger().warning("Ice::Communicator::destroy() has not been called");
	}

        super.finalize();
    }
      */

    //
    // Certain initialization tasks need to be completed after the
    // constructor.
    //
    void
    finishSetup(StringSeqHolder args)
    {
	try
	{
	    _instance.finishSetup(args);
	}
	catch(RuntimeException ex)
	{
	    _instance.destroy();
	    _instance = null;
	    synchronized(this)
	    {
		_destroyed = true;
	    }
	    throw ex;
	}
    }

    //
    // For use by Util.getInstance()
    //
    IceInternal.Instance
    getInstance()
    {
        return _instance;
    }

    private boolean _destroyed;
    private IceInternal.Instance _instance;
}
