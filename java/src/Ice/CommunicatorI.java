// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public final class CommunicatorI implements Communicator
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

    public java.util.Map<String, String>
    proxyToProperty(Ice.ObjectPrx proxy, String prefix)
    {
        return _instance.proxyFactory().proxyToProperty(proxy, prefix);
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
        return _instance.objectAdapterFactory().createObjectAdapter(name, null);
    }

    public ObjectAdapter
    createObjectAdapterWithEndpoints(String name, String endpoints)
    {
        if(name.length() == 0)
        {
            Ice.InitializationException ex = new Ice.InitializationException();
            ex.reason = "Cannot configure endpoints with nameless object adapter";
            throw ex;
        }
        
        getProperties().setProperty(name + ".Endpoints", endpoints);
        return _instance.objectAdapterFactory().createObjectAdapter(name, null);
    }

    public ObjectAdapter
    createObjectAdapterWithRouter(String name, RouterPrx router)
    {
        if(name.length() == 0)
        {
            Ice.InitializationException ex = new Ice.InitializationException();
            ex.reason = "Cannot configure router with nameless object adapter";
            throw ex;
        }

        //
        // We set the proxy properties here, although we still use the proxy supplied.
        //
        java.util.Map<String, String> properties = proxyToProperty(router, name + ".Router");
        for(java.util.Map.Entry<String, String> p : properties.entrySet())
        {
            getProperties().setProperty(p.getKey(), p.getValue());
        }

        return _instance.objectAdapterFactory().createObjectAdapter(name, router);
    }

    public void
    addObjectFactory(ObjectFactory factory, String id)
    {
        _instance.servantFactoryManager().add(factory, id);
    }

    public ObjectFactory
    findObjectFactory(String id)
    {
        return _instance.servantFactoryManager().find(id);
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

    public Stats
    getStats()
    {
        return _instance.initializationData().stats;
    }

    public RouterPrx
    getDefaultRouter()
    {
        return _instance.referenceFactory().getDefaultRouter();
    }

    public void
    setDefaultRouter(RouterPrx router)
    {
        _instance.setDefaultRouter(router);
    }

    public LocatorPrx
    getDefaultLocator()
    {
        return _instance.referenceFactory().getDefaultLocator();
    }

    public void
    setDefaultLocator(LocatorPrx locator)
    {
        _instance.setDefaultLocator(locator);
    }

    public ImplicitContext
    getImplicitContext()
    {
        return _instance.getImplicitContext();
    }

    public PluginManager
    getPluginManager()
    {
        return _instance.pluginManager();
    }

    public void
    flushBatchRequests()
    {
        _instance.flushBatchRequests();
    }

    public ObjectPrx 
    getAdmin()
    {
        return _instance.getAdmin();
    }

    public void 
    addAdminFacet(Object servant, String facet)
    {
        _instance.addAdminFacet(servant, facet);
    }

    public Object 
    removeAdminFacet(String facet)
    {
        return _instance.removeAdminFacet(facet);
    }

    CommunicatorI(InitializationData initData)
    {
        _instance = new IceInternal.Instance(this, initData);
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
            throw ex;
        }
    }

    //
    // For use by IceInternal.Util.getInstance()
    //
    public IceInternal.Instance
    getInstance()
    {
        return _instance;
    }

    private IceInternal.Instance _instance;
}
