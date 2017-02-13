// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

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
        _instance.objectAdapterFactory().shutdown();
    }

    @Override
    public void
    waitForShutdown()
    {
        _instance.objectAdapterFactory().waitForShutdown();
    }

    @Override
    public boolean
    isShutdown()
    {
        return _instance.objectAdapterFactory().isShutdown();
    }

    @Override
    public Ice.ObjectPrx
    stringToProxy(String s)
    {
        return _instance.proxyFactory().stringToProxy(s);
    }

    @Override
    public String
    proxyToString(Ice.ObjectPrx proxy)
    {
        return _instance.proxyFactory().proxyToString(proxy);
    }

    @Override
    public Ice.ObjectPrx
    propertyToProxy(String s)
    {
        return _instance.proxyFactory().propertyToProxy(s);
    }

    @Override
    public java.util.Map<String, String>
    proxyToProperty(Ice.ObjectPrx proxy, String prefix)
    {
        return _instance.proxyFactory().proxyToProperty(proxy, prefix);
    }

    @Override @SuppressWarnings("deprecation")
    public Ice.Identity
    stringToIdentity(String s)
    {
        return Ice.Util.stringToIdentity(s);
    }

    @Override @SuppressWarnings("deprecation")
    public String
    identityToString(Ice.Identity ident)
    {
        return Ice.Util.identityToString(ident, _instance.toStringMode());
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
    public Ice.Instrumentation.CommunicatorObserver
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
    public void
    flushBatchRequests(Ice.CompressBatch compressBatch)
    {
        end_flushBatchRequests(begin_flushBatchRequests(compressBatch));
    }

    @Override
    public AsyncResult
    begin_flushBatchRequests(Ice.CompressBatch compressBatch)
    {
        return begin_flushBatchRequestsInternal(compressBatch, null);
    }

    @Override
    public AsyncResult
    begin_flushBatchRequests(Ice.CompressBatch compressBatch, Callback cb)
    {
        return begin_flushBatchRequestsInternal(compressBatch, cb);
    }

    @Override
    public AsyncResult
    begin_flushBatchRequests(Ice.CompressBatch compressBatch, Callback_Communicator_flushBatchRequests cb)
    {
        return begin_flushBatchRequestsInternal(compressBatch, cb);
    }

    @Override
    public AsyncResult
    begin_flushBatchRequests(Ice.CompressBatch compressBatch,
                             IceInternal.Functional_VoidCallback responseCb,
                             IceInternal.Functional_GenericCallback1<Ice.Exception> exceptionCb,
                             IceInternal.Functional_BoolCallback sentCb)
    {
        return begin_flushBatchRequestsInternal(compressBatch,
            new IceInternal.Functional_CallbackBase(false, exceptionCb, sentCb)
                {
                    @Override
                    public final void _iceCompleted(AsyncResult result)
                    {
                        try
                        {
                            result.getCommunicator().end_flushBatchRequests(result);
                        }
                        catch(Exception ex)
                        {
                            _exceptionCb.apply(ex);
                        }
                    }
                });
    }

    private static final String _flushBatchRequests_name = "flushBatchRequests";

    private Ice.AsyncResult
    begin_flushBatchRequestsInternal(Ice.CompressBatch compressBatch, IceInternal.CallbackBase cb)
    {
        IceInternal.OutgoingConnectionFactory connectionFactory = _instance.outgoingConnectionFactory();
        IceInternal.ObjectAdapterFactory adapterFactory = _instance.objectAdapterFactory();

        //
        // This callback object receives the results of all invocations
        // of Connection.begin_flushBatchRequests.
        //
        IceInternal.CommunicatorFlushBatch result = new IceInternal.CommunicatorFlushBatch(this,
                                                                                           _instance,
                                                                                           _flushBatchRequests_name,
                                                                                           cb);

        connectionFactory.flushAsyncBatchRequests(compressBatch, result);
        adapterFactory.flushAsyncBatchRequests(compressBatch, result);

        //
        // Inform the callback that we have finished initiating all of the
        // flush requests.
        //
        result.ready();

        return result;
    }

    @Override
    public void
    end_flushBatchRequests(AsyncResult r)
    {
        IceInternal.CommunicatorFlushBatch ri =
            IceInternal.CommunicatorFlushBatch.check(r, this, _flushBatchRequests_name);
        ri.waitForResponseOrUserEx();
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
    public java.util.Map<String, Ice.Object>
    findAllAdminFacets()
    {
        return _instance.findAllAdminFacets();
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
            _instance.finishSetup(args, this);
        }
        catch(RuntimeException ex)
        {
            _instance.destroy(false);
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
