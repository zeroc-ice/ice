// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Ice
{
	
    sealed class CommunicatorI : LocalObjectImpl, Communicator
    {
	public void destroy()
	{
	    IceInternal.Instance instance = null;
	    
	    lock(this)
	    {
		if(!_destroyed) // Don't destroy twice.
		{
		    _destroyed = true;
		    instance = _instance;
		}
	    }
	    
	    if(instance != null)
	    {
		instance.destroy();
	    }
	}

	public void shutdown()
	{
	    IceInternal.ObjectAdapterFactory objectAdapterFactory;
	    
	    lock(this)
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
	
	public void waitForShutdown()
	{
	    IceInternal.ObjectAdapterFactory objectAdapterFactory;
	    
	    lock(this)
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
	
	public Ice.ObjectPrx stringToProxy(string s)
	{
	    lock(this)
	    {
		if(_destroyed)
		{
		    throw new CommunicatorDestroyedException();
		}
		return _instance.proxyFactory().stringToProxy(s);
	    }
	}
	
	public string proxyToString(Ice.ObjectPrx proxy)
	{
	    lock(this)
	    {
		if(_destroyed)
		{
		    throw new CommunicatorDestroyedException();
		}
		return _instance.proxyFactory().proxyToString(proxy);
	    }
	}

	public ObjectAdapter createObjectAdapter(string name)
	{
	    lock(this)
	    {
		if(_destroyed)
		{
		    throw new CommunicatorDestroyedException();
		}
		
		return _instance.objectAdapterFactory().createObjectAdapter(name);
	    }
	}
	
	public ObjectAdapter createObjectAdapterWithEndpoints(string name, string endpoints)
	{
	    getProperties().setProperty(name + ".Endpoints", endpoints);
	    return createObjectAdapter(name);
	}
	
	public void addObjectFactory(ObjectFactory factory, string id)
	{
	    lock(this)
	    {
		if(_destroyed)
		{
		    throw new CommunicatorDestroyedException();
		}
		_instance.servantFactoryManager().add(factory, id);
	    }
	}
	
	public void removeObjectFactory(string id)
	{
	    lock(this)
	    {
		if(_destroyed)
		{
		    throw new CommunicatorDestroyedException();
		}
		_instance.servantFactoryManager().remove(id);
	    }
	}
	
	public ObjectFactory findObjectFactory(string id)
	{
	    lock(this)
	    {
		if(_destroyed)
		{
		    throw new CommunicatorDestroyedException();
		}
		return _instance.servantFactoryManager().find(id);
	    }
	}
	
	public Properties getProperties()
	{
	    //
	    // No check for destruction. It must be possible to access the
	    // properties after destruction.
	    //
	    return _instance.properties();
	}

	
	public Logger getLogger()
	{
	    //
	    // No check for destruction. It must be possible to access the
	    // logger after destruction.
	    //
	    return _instance.logger();
	}

	public void setLogger(Logger logger)
	{
	    //
	    // No check for destruction. It must be possible to set the
	    // logger after destruction (needed by logger plugins for
	    // example to unset the logger).
	    //
	    _instance.logger(logger);
	}
	    
	public Stats getStats()
	{
	    lock(this)
	    {
		if(_destroyed)
		{
			throw new CommunicatorDestroyedException();
		}
		return _instance.stats();
	    }
	}

	public void setStats(Stats stats)
	{
	    lock(this)
	    {
		if(_destroyed)
		{
		    throw new CommunicatorDestroyedException();
		}
		_instance.stats(stats);
	    }
	}

	public RouterPrx getDefaultRouter()
	{
	    lock(this)
	    {
		if(_destroyed)
		{
		    throw new CommunicatorDestroyedException();
		}
		return _instance.referenceFactory().getDefaultRouter();
	    }
	}

	public void setDefaultRouter(RouterPrx router)
	{
	    lock(this)
	    {
		if(_destroyed)
		{
		    throw new CommunicatorDestroyedException();
		}
		_instance.referenceFactory().setDefaultRouter(router);
	    }
	}

	public LocatorPrx getDefaultLocator()
	{
	    lock(this)
	    {
		if(_destroyed)
		{
		    throw new CommunicatorDestroyedException();
		}
		return _instance.referenceFactory().getDefaultLocator();
	    }
	}

	public void setDefaultLocator(LocatorPrx locator)
	{
	    lock(this)
	    {
		if(_destroyed)
		{
		    throw new CommunicatorDestroyedException();
		}
		_instance.referenceFactory().setDefaultLocator(locator);
	    }
	}
	
	public Ice.Context getDefaultContext()
	{
	    lock(this)
	    {
		if(_destroyed)
		{
		    throw new CommunicatorDestroyedException();
		}
	        return _instance.getDefaultContext();
	    }
	}
	
	public void setDefaultContext(Ice.Context ctx)
	{
	    lock(this)
	    {
		if(_destroyed)
		{
		    throw new CommunicatorDestroyedException();
		}
	        _instance.setDefaultContext(ctx);
	    }
	}

	public PluginManager getPluginManager()
	{
	    return null;
	}

	public void flushBatchRequests()
	{
	    _instance.flushBatchRequests();
	}
	
	internal CommunicatorI(Properties properties)
	{
	    _destroyed = false;
	    _instance = new IceInternal.Instance(this, properties);
	}
	
	~CommunicatorI()
	{
	    if(!_destroyed)
	    {
		_instance.logger().warning("Ice::Communicator::destroy() has not been called");
	    }
	}

	//
	// Certain initialization tasks need to be completed after the
	// constructor.
	//
	internal void finishSetup(ref string[] args)
	{
            try
            {
                _instance.finishSetup(ref args);
            }
            catch(System.Exception)
            {
                _instance.destroy();
                _instance = null;
                _destroyed = true;
                throw;
            }
	}
	
	//
	// For use by Util.getInstance()
	//
	internal IceInternal.Instance getInstance()
	{
	    return _instance;
	}
	
	
	private bool _destroyed;
	private IceInternal.Instance _instance;
    }

}
