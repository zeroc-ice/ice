// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package com.zeroc.Ice;

public final class CommunicatorI implements Communicator
{
    @Override
    public void
    close()
    {
        _instance.destroy(false); // Don't allow destroy to be interrupted if called from try with statement.
    }

    @Override
    public void
    destroy()
    {
        _instance.destroy(true); // Destroy is interruptible when call explicitly.
    }

    @Override
    public void
    shutdown()
    {
        try
        {
            _instance.objectAdapterFactory().shutdown();
        }
        catch(com.zeroc.Ice.CommunicatorDestroyedException ex)
        {
            // Ignore
        }
    }

    @Override
    public void
    waitForShutdown()
    {
        try
        {
            _instance.objectAdapterFactory().waitForShutdown();
        }
        catch(com.zeroc.Ice.CommunicatorDestroyedException ex)
        {
            // Ignore
        }
    }

    @Override
    public boolean
    isShutdown()
    {
        try
        {
            return _instance.objectAdapterFactory().isShutdown();
        }
        catch(com.zeroc.Ice.CommunicatorDestroyedException ex)
        {
            return true;
        }
    }

    @Override
    public ObjectPrx
    stringToProxy(String s)
    {
        return _instance.proxyFactory().stringToProxy(s);
    }

    @Override
    public String
    proxyToString(ObjectPrx proxy)
    {
        return _instance.proxyFactory().proxyToString(proxy);
    }

    @Override
    public ObjectPrx
    propertyToProxy(String s)
    {
        return _instance.proxyFactory().propertyToProxy(s);
    }

    @Override
    public java.util.Map<String, String>
    proxyToProperty(ObjectPrx proxy, String prefix)
    {
        return _instance.proxyFactory().proxyToProperty(proxy, prefix);
    }

    @Override @SuppressWarnings("deprecation")
    public Identity
    stringToIdentity(String s)
    {
        return Util.stringToIdentity(s);
    }

    @Override
    public String
    identityToString(Identity ident)
    {
        return Util.identityToString(ident, _instance.toStringMode());
    }

    @Override
    public ObjectAdapter
    createObjectAdapter(String name)
    {
        return _instance.objectAdapterFactory().createObjectAdapter(name, null);
    }

    @Override
    public ObjectAdapter
    createObjectAdapterWithEndpoints(String name, String endpoints)
    {
        if(name.length() == 0)
        {
            name = java.util.UUID.randomUUID().toString();
        }

        getProperties().setProperty(name + ".Endpoints", endpoints);
        return _instance.objectAdapterFactory().createObjectAdapter(name, null);
    }

    @Override
    public ObjectAdapter
    createObjectAdapterWithRouter(String name, RouterPrx router)
    {
        if(name.length() == 0)
        {
            name = java.util.UUID.randomUUID().toString();
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

    @Override @SuppressWarnings("deprecation")
    public void addObjectFactory(ObjectFactory factory, String id)
    {
        _instance.addObjectFactory(factory, id);
    }

    @Override @SuppressWarnings("deprecation")
    public ObjectFactory findObjectFactory(String id)
    {
        return _instance.findObjectFactory(id);
    }

    @Override
    public ValueFactoryManager getValueFactoryManager()
    {
        return _instance.initializationData().valueFactoryManager;
    }

    @Override
    public Properties
    getProperties()
    {
        return _instance.initializationData().properties;
    }

    @Override
    public Logger
    getLogger()
    {
        return _instance.initializationData().logger;
    }

    @Override
    public com.zeroc.Ice.Instrumentation.CommunicatorObserver
    getObserver()
    {
        return _instance.initializationData().observer;
    }

    @Override
    public RouterPrx
    getDefaultRouter()
    {
        return _instance.referenceFactory().getDefaultRouter();
    }

    @Override
    public void
    setDefaultRouter(RouterPrx router)
    {
        _instance.setDefaultRouter(router);
    }

    @Override
    public LocatorPrx
    getDefaultLocator()
    {
        return _instance.referenceFactory().getDefaultLocator();
    }

    @Override
    public void
    setDefaultLocator(LocatorPrx locator)
    {
        _instance.setDefaultLocator(locator);
    }

    @Override
    public ImplicitContext
    getImplicitContext()
    {
        return _instance.getImplicitContext();
    }

    @Override
    public PluginManager
    getPluginManager()
    {
        return _instance.pluginManager();
    }

    @Override
    public void flushBatchRequests(CompressBatch compressBatch)
    {
        _iceI_flushBatchRequestsAsync(compressBatch).waitForResponse();
    }

    @Override
    public java.util.concurrent.CompletableFuture<Void> flushBatchRequestsAsync(CompressBatch compressBatch)
    {
        return _iceI_flushBatchRequestsAsync(compressBatch);
    }

    private com.zeroc.IceInternal.CommunicatorFlushBatch _iceI_flushBatchRequestsAsync(CompressBatch compressBatch)
    {
        //
        // This callback object receives the results of all invocations
        // of Connection.begin_flushBatchRequests.
        //
        com.zeroc.IceInternal.CommunicatorFlushBatch f =
            new com.zeroc.IceInternal.CommunicatorFlushBatch(this, _instance);
        f.invoke(compressBatch);
        return f;
    }

    @Override
    public ObjectPrx
    createAdmin(ObjectAdapter adminAdapter, Identity adminId)
    {
        return _instance.createAdmin(adminAdapter, adminId);
    }

    @Override
    public ObjectPrx
    getAdmin()
    {
        return _instance.getAdmin();
    }

    @Override
    public void
    addAdminFacet(Object servant, String facet)
    {
        _instance.addAdminFacet(servant, facet);
    }

    @Override
    public Object
    removeAdminFacet(String facet)
    {
        return _instance.removeAdminFacet(facet);
    }

    @Override
    public Object
    findAdminFacet(String facet)
    {
        return _instance.findAdminFacet(facet);
    }

    @Override
    public java.util.Map<String, com.zeroc.Ice.Object>
    findAllAdminFacets()
    {
        return _instance.findAllAdminFacets();
    }

    CommunicatorI(InitializationData initData)
    {
        _instance = new com.zeroc.IceInternal.Instance(this, initData);
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
    void finishSetup(String[] args, java.util.List<String> rArgs)
    {
        try
        {
            args = _instance.finishSetup(args, this);
            if(rArgs != null)
            {
                rArgs.clear();
                if(args.length > 0)
                {
                    rArgs.addAll(java.util.Arrays.asList(args));
                }
            }
        }
        catch(RuntimeException ex)
        {
            _instance.destroy(false);
            throw ex;
        }
    }

    //
    // For use by com.zeroc.IceInternal.Util.getInstance()
    //
    public com.zeroc.IceInternal.Instance
    getInstance()
    {
        return _instance;
    }

    private com.zeroc.IceInternal.Instance _instance;
}
