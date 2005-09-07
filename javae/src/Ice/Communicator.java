// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public final class Communicator
{
    public void
    destroy()
    {
	_instance.destroy();
    }

    public void
    shutdown()
    {
	_instance.objectAdapterFactory().shutdown();
    }

    public void
    waitForShutdown()
    {
	_instance.objectAdapterFactory().waitForShutdown();
    }

    public Ice.ObjectPrx
    stringToProxy(String s)
    {
        return _instance.proxyFactory().stringToProxy(s);
    }

    public String
    proxyToString(Ice.ObjectPrx proxy)
    {
        return _instance.proxyFactory().proxyToString(proxy);
    }

    public ObjectAdapter
    createObjectAdapter(String name)
    {
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

    public Properties
    getProperties()
    {
        return _instance.properties();
    }

    public Logger
    getLogger()
    {
        return _instance.logger();
    }

    public void
    setLogger(Logger logger)
    {
        _instance.logger(logger);
    }

    public RouterPrx
    getDefaultRouter()
    {
        return _instance.referenceFactory().getDefaultRouter();
    }

    public void
    setDefaultRouter(RouterPrx router)
    {
        _instance.referenceFactory().setDefaultRouter(router);
    }

    public LocatorPrx
    getDefaultLocator()
    {
        return _instance.referenceFactory().getDefaultLocator();
    }

    public void
    setDefaultLocator(LocatorPrx locator)
    {
        _instance.referenceFactory().setDefaultLocator(locator);
    }

    public java.util.Hashtable
    getDefaultContext()
    {
	return _instance.getDefaultContext();
    }

    public void
    setDefaultContext(java.util.Hashtable ctx)
    {
	_instance.setDefaultContext(ctx);
    }

    public void
    flushBatchRequests()
    {
        _instance.flushBatchRequests();
    }

    Communicator(Properties properties)
    {
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
        if(!_instance.destroyed())
        {
            _instance.logger().warning("Ice::Communicator::destroy() has not been called");
        }
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

    private IceInternal.Instance _instance;
}
