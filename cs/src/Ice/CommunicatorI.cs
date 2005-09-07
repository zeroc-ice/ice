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
	    _instance.destroy();
	}

	public void shutdown()
	{
	    _instance.objectAdapterFactory().shutdown();
	}
	
	public void waitForShutdown()
	{
	    _instance.objectAdapterFactory().waitForShutdown();
	}
	
	public Ice.ObjectPrx stringToProxy(string s)
	{
	    return _instance.proxyFactory().stringToProxy(s);
	}
	
	public string proxyToString(Ice.ObjectPrx proxy)
	{
	    return _instance.proxyFactory().proxyToString(proxy);
	}

	public ObjectAdapter createObjectAdapter(string name)
	{
	    return _instance.objectAdapterFactory().createObjectAdapter(name);
	}
	
	public ObjectAdapter createObjectAdapterWithEndpoints(string name, string endpoints)
	{
	    string propertyKey = name + ".Endpoints";
	    string originalValue = getProperties().getProperty(propertyKey);
	    try
	    {
		getProperties().setProperty(propertyKey, endpoints);
		return createObjectAdapter(name);
	    }
	    catch(AlreadyRegisteredException ex)
	    {
		getProperties().setProperty(propertyKey, originalValue);
		throw ex;
	    }
	}
	
	public void addObjectFactory(ObjectFactory factory, string id)
	{
	    _instance.servantFactoryManager().add(factory, id);
	}
	
	public void removeObjectFactory(string id)
	{
	    _instance.servantFactoryManager().remove(id);
	}
	
	public ObjectFactory findObjectFactory(string id)
	{
	    return _instance.servantFactoryManager().find(id);
	}
	
	public Properties getProperties()
	{
	    return _instance.properties();
	}
	
	public Logger getLogger()
	{
	    return _instance.logger();
	}

	public void setLogger(Logger logger)
	{
	    _instance.logger(logger);
	}
	    
	public Stats getStats()
	{
	    return _instance.stats();
	}

	public void setStats(Stats stats)
	{
	    _instance.stats(stats);
	}

	public RouterPrx getDefaultRouter()
	{
	    return _instance.referenceFactory().getDefaultRouter();
	}

	public void setDefaultRouter(RouterPrx router)
	{
	    _instance.referenceFactory().setDefaultRouter(router);
	}

	public LocatorPrx getDefaultLocator()
	{
	    return _instance.referenceFactory().getDefaultLocator();
	}

	public void setDefaultLocator(LocatorPrx locator)
	{
	    _instance.referenceFactory().setDefaultLocator(locator);
	}
	
	public Ice.Context getDefaultContext()
	{
	    return _instance.getDefaultContext();
	}
	
	public void setDefaultContext(Ice.Context ctx)
	{
	    _instance.setDefaultContext(ctx);
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
	    _instance = new IceInternal.Instance(this, properties);
	}
	
#if DEBUG
	~CommunicatorI()
	{
	    lock(this)
	    {
		if(!_instance.destroyed())
		{
		    if(!System.Environment.HasShutdownStarted)
		    {
			_instance.logger().warning("Ice::Communicator::destroy() has not been called");
		    }
		    else
		    {
		        System.Console.Error.WriteLine("Ice::Communicator::destroy() has not been called");
		    }
		}
	    }
	}
#endif

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
	
	
	private IceInternal.Instance _instance;
    }

}
