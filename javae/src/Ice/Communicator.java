// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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

    public boolean
    isShutdown()
    {
	return _instance.objectAdapterFactory().isShutdown();
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

    public Ice.ObjectPrx
    propertyToProxy(String s)
    {
        return _instance.proxyFactory().propertyToProxy(s);
    }

    public Ice.Identity
    stringToIdentity(String s)
    {
        return _instance.stringToIdentity(s);
    }

    public String
    identityToString(Ice.Identity ident)
    {
        return _instance.identityToString(ident);
    }

    public ObjectAdapter
    createObjectAdapter(String name)
    {
	return createObjectAdapterWithEndpoints(name, getProperties().getProperty(name + ".Endpoints"));
    }

    public ObjectAdapter
    createObjectAdapterWithEndpoints(String name, String endpoints)
    {
	return _instance.objectAdapterFactory().createObjectAdapter(name, endpoints, null);
    }

    public ObjectAdapter
    createObjectAdapterWithRouter(String name, RouterPrx router)
    {
	return _instance.objectAdapterFactory().createObjectAdapter(name, "", router);
    }

    public Properties
    getProperties()
    {
        return _instance.initializationData().properties;
    }

    public Logger
    getLogger()
    {
        return _instance.initializationData().logger;
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

    public void
    flushBatchRequests()
    {
        _instance.flushBatchRequests();
    }

    Communicator(InitializationData initData)
    {
        _instance = new IceInternal.Instance(this, initData);
    }

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
