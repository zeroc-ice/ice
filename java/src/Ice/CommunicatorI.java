// **********************************************************************
//
// Copyright (c) 2003
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

package Ice;

final class CommunicatorI extends LocalObjectImpl implements Communicator
{
    public void
    destroy()
    {
	IceInternal.Instance instance = null;

	synchronized(this)
	{
	    if(!_destroyed) // Don't destroy twice.
	    {
		_destroyed = true;
		_serverThreadPool = null;
		instance = _instance;
	    }
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

	ObjectAdapter adapter = _instance.objectAdapterFactory().createObjectAdapter(name);

	if(_serverThreadPool == null) // Lazy initialization of _serverThreadPool.
	{
	    _serverThreadPool = _instance.serverThreadPool();
	}

        return adapter;
    }

    public synchronized ObjectAdapter
    createObjectAdapterWithEndpoints(String name, String endpoints)
    {
	getProperties().setProperty(name + ".Endpoints", endpoints);
	return createObjectAdapter(name);
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

    public synchronized void
    addUserExceptionFactory(IceInternal.UserExceptionFactory factory, String id)
    {
        if(_destroyed)
        {
            throw new CommunicatorDestroyedException();
        }
        _instance.userExceptionFactoryManager().add(factory, id);
    }

    public synchronized void
    removeUserExceptionFactory(String id)
    {
        if(_destroyed)
        {
            throw new CommunicatorDestroyedException();
        }
        _instance.userExceptionFactoryManager().remove(id);
    }

    public synchronized IceInternal.UserExceptionFactory
    findUserExceptionFactory(String id)
    {
        if(_destroyed)
        {
            throw new CommunicatorDestroyedException();
        }
        return _instance.userExceptionFactoryManager().find(id);
    }

    public synchronized Properties
    getProperties()
    {
	//
	// No check for destruction. It must be possible to access the
	// properties after destruction.
	//
        return _instance.properties();
    }

    public synchronized Logger
    getLogger()
    {
	//
	// No check for destruction. It must be possible to access the
	// logger after destruction.
	//
        return _instance.logger();
    }

    public synchronized void
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

    public synchronized void
    setDefaultRouter(RouterPrx router)
    {
        if(_destroyed)
        {
            throw new CommunicatorDestroyedException();
        }
        _instance.referenceFactory().setDefaultRouter(router);
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

    CommunicatorI(StringSeqHolder args, Properties properties)
    {
	_destroyed = false;
        _instance = new IceInternal.Instance(this, args, properties);
    }

    protected void
    finalize()
        throws Throwable
    {
        if(!_destroyed)
        {
            _instance.logger().warning("Ice::Communicator::destroy() has not been called");
        }

        super.finalize();
    }

    //
    // Certain initialization tasks need to be completed after the
    // constructor.
    //
    void
    finishSetup(StringSeqHolder args)
    {
        _instance.finishSetup(args);
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

    //
    // We need _serverThreadPool directly in CommunicatorI. That's
    // because the shutdown() operation is signal-safe, and thus must
    // not access any mutex locks or _instance. It may only access
    // _serverThreadPool.initiateShutdown(), which is signal-safe as
    // well.
    //
    private IceInternal.ThreadPool _serverThreadPool;
}
