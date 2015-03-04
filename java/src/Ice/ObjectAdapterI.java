// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public final class ObjectAdapterI implements ObjectAdapter
{
    public String
    getName()
    {
        //
        // No mutex lock necessary, _name is immutable.
        //
        return _noConfig ? "" : _name;
    }

    public synchronized Communicator
    getCommunicator()
    {
        return _communicator;
    }

    public void
    activate()
    {
        IceInternal.LocatorInfo locatorInfo = null;
        boolean registerProcess = false;
        boolean printAdapterReady = false;

        synchronized(this)
        {
            checkForDeactivation();

            //
            // If some threads are waiting on waitForHold(), we set this
            // flag to ensure the threads will start again the wait for
            // all the incoming connection factories.
            //
            _waitForHoldRetry = _waitForHold > 0;

            //
            // If the one off initializations of the adapter are already
            // done, we just need to activate the incoming connection
            // factories and we're done.
            //
            if(_activateOneOffDone)
            {
                for(IceInternal.IncomingConnectionFactory factory : _incomingConnectionFactories)
                {
                    factory.activate();
                }
                return;
            }

            //
            // One off initializations of the adapter: update the
            // locator registry and print the "adapter ready"
            // message. We set the _waitForActivate flag to prevent
            // deactivation from other threads while these one off
            // initializations are done.
            //
            _waitForActivate = true;

            locatorInfo = _locatorInfo;
            if(!_noConfig)
            {
                final Properties properties = _instance.initializationData().properties;
                registerProcess = properties.getPropertyAsInt(_name +".RegisterProcess") > 0;
                printAdapterReady = properties.getPropertyAsInt("Ice.PrintAdapterReady") > 0;
            }
        }

        try
        {
            Ice.Identity dummy = new Ice.Identity();
            dummy.name = "dummy";
            updateLocatorRegistry(locatorInfo, createDirectProxy(dummy), registerProcess);
        }
        catch(Ice.LocalException ex)
        {
            //
            // If we couldn't update the locator registry, we let the
            // exception go through and don't activate the adapter to
            // allow to user code to retry activating the adapter
            // later.
            //
            synchronized(this)
            {
                _waitForActivate = false;
                notifyAll();
            }
            throw ex;
        }

        if(printAdapterReady)
        {
            System.out.println(_name + " ready");
        }

        synchronized(this)
        {
            assert(!_deactivated); // Not possible if _waitForActivate = true;

            //
            // Signal threads waiting for the activation.
            //
            _waitForActivate = false;
            notifyAll();

            _activateOneOffDone = true;

            for(IceInternal.IncomingConnectionFactory factory : _incomingConnectionFactories)
            {
                factory.activate();
            }
        }
    }

    public synchronized void
    hold()
    {
        checkForDeactivation();

        for(IceInternal.IncomingConnectionFactory factory : _incomingConnectionFactories)
        {
            factory.hold();
        }
    }

    public void
    waitForHold()
    {
        while(true)
        {
            java.util.List<IceInternal.IncomingConnectionFactory> incomingConnectionFactories;
            synchronized(this)
            {
                checkForDeactivation();
            
                incomingConnectionFactories =
                    new java.util.ArrayList<IceInternal.IncomingConnectionFactory>(_incomingConnectionFactories);

                ++_waitForHold;
            }

            for(IceInternal.IncomingConnectionFactory factory : incomingConnectionFactories)
            {
                factory.waitUntilHolding();
            }

            synchronized(this)
            {
                if(--_waitForHold == 0)
                {
                    notifyAll();
                }
                
                //
                // If we don't need to retry, we're done. Otherwise, we wait until 
                // all the waiters finish waiting on the connections and we try 
                // again waiting on all the conncetions. This is necessary in the 
                // case activate() is called by another thread while waitForHold()
                // waits on the some connection, if we didn't retry, waitForHold() 
                // could return only after waiting on a subset of the connections.
                //
                if(!_waitForHoldRetry)
                {
                    return;
                }
                else
                {
                    while(_waitForHold > 0)
                    {
                        checkForDeactivation();
                        try
                        {
                            wait();
                        }
                        catch(java.lang.InterruptedException ex)
                        {
                        }
                    }
                    _waitForHoldRetry = false;
                }
            }
        }
    }

    public void
    deactivate()
    {
        IceInternal.OutgoingConnectionFactory outgoingConnectionFactory;
        java.util.List<IceInternal.IncomingConnectionFactory> incomingConnectionFactories;
        IceInternal.LocatorInfo locatorInfo;
        synchronized(this)
        {
            //
            // Ignore deactivation requests if the object adapter has
            // already been deactivated.
            //
            if(_deactivated)
            {
                return;
            }

            //
            //
            // Wait for activation to complete. This is necessary to not
            // get out of order locator updates.
            //
            while(_waitForActivate)
            {
                try
                {
                    wait();
                }
                catch(InterruptedException ex)
                {
                }
            }

            if(_routerInfo != null)
            {
                //
                // Remove entry from the router manager.
                //
                _instance.routerManager().erase(_routerInfo.getRouter());

                //
                //  Clear this object adapter with the router.
                //
                _routerInfo.setAdapter(null);
            }

            incomingConnectionFactories =
                new java.util.ArrayList<IceInternal.IncomingConnectionFactory>(_incomingConnectionFactories);
            outgoingConnectionFactory = _instance.outgoingConnectionFactory();
            locatorInfo = _locatorInfo;

            _deactivated = true;

            notifyAll();
        }

        try
        {
            updateLocatorRegistry(locatorInfo, null, false);
        }
        catch(Ice.LocalException ex)
        {
            //
            // We can't throw exceptions in deactivate so we ignore
            // failures to update the locator registry.
            //
        }

        //
        // Must be called outside the thread synchronization, because
        // Connection::destroy() might block when sending a CloseConnection
        // message.
        //
        for(IceInternal.IncomingConnectionFactory factory : incomingConnectionFactories)
        {
            factory.destroy();
        }

        //
        // Must be called outside the thread synchronization, because
        // changing the object adapter might block if there are still
        // requests being dispatched.
        //
        outgoingConnectionFactory.removeAdapter(this);
    }

    public void
    waitForDeactivate()
    {
        IceInternal.IncomingConnectionFactory[] incomingConnectionFactories;

        synchronized(this)
        {
            if(_destroyed)
            {
                return;
            }

            //
            // Wait for deactivation of the adapter itself, and
            // for the return of all direct method calls using this
            // adapter.
            //
            while(!_deactivated || _directCount > 0)
            {
                try
                {
                    wait();
                }
                catch(InterruptedException ex)
                {
                }
            }

            incomingConnectionFactories =
                _incomingConnectionFactories.toArray(new IceInternal.IncomingConnectionFactory[0]);
        }

        //
        // Now we wait for until all incoming connection factories are
        // finished.
        //
        for(IceInternal.IncomingConnectionFactory f : incomingConnectionFactories)
        {
            f.waitUntilFinished();
        }
    }

    public synchronized boolean
    isDeactivated()
    {
        return _deactivated;
    }

    public void
    destroy()
    {
        synchronized(this)
        {
            //
            // Another thread is in the process of destroying the object
            // adapter. Wait for it to finish.
            //
            while(_destroying)
            {
                try
                {
                    wait();
                }
                catch(InterruptedException ex)
                {
                }
            }

            //
            // Object adpater is already destroyed.
            //
            if(_destroyed)
            {
                return;
            }

            _destroying = true;
        }

        //
        // Deactivate and wait for completion.
        //
        deactivate();
        waitForDeactivate();

        //
        // Now it's also time to clean up our servants and servant
        // locators.
        //
        _servantManager.destroy();

        //
        // Destroy the thread pool.
        //
        if(_threadPool != null)
        {
            _threadPool.destroy();
            _threadPool.joinWithAllThreads();
        }

        IceInternal.ObjectAdapterFactory objectAdapterFactory;

        synchronized(this)
        {
            //
            // Signal that destroying is complete.
            //
            _destroying = false;
            _destroyed = true;
            notifyAll();

            _incomingConnectionFactories.clear();

            //
            // Remove object references (some of them cyclic).
            //
            _instance = null;
            _threadPool = null;
            _routerEndpoints = null;
            _routerInfo = null;
            _publishedEndpoints = null;
            _locatorInfo = null;
            _reference = null;

            objectAdapterFactory = _objectAdapterFactory;
            _objectAdapterFactory = null;
        }

        if(objectAdapterFactory != null)
        {
            objectAdapterFactory.removeObjectAdapter(this);
        }
    }

    public ObjectPrx
    add(Ice.Object object, Identity ident)
    {
        return addFacet(object, ident, "");
    }

    public synchronized ObjectPrx
    addFacet(Ice.Object object, Identity ident, String facet)
    {
        checkForDeactivation();
        checkIdentity(ident);

        //
        // Create a copy of the Identity argument, in case the caller
        // reuses it.
        //
        Identity id = new Identity();
        id.category = ident.category;
        id.name = ident.name;

        _servantManager.addServant(object, id, facet);

        return newProxy(id, facet);
    }

    public ObjectPrx
    addWithUUID(Ice.Object object)
    {
        return addFacetWithUUID(object, "");
    }

    public ObjectPrx
    addFacetWithUUID(Ice.Object object, String facet)
    {
        Identity ident = new Identity();
        ident.category = "";
        ident.name = java.util.UUID.randomUUID().toString();

        return addFacet(object, ident, facet);
    }

    public synchronized void
    addDefaultServant(Ice.Object servant, String category)
    {
        checkForDeactivation();

        _servantManager.addDefaultServant(servant, category);
    }

    public Ice.Object
    remove(Identity ident)
    {
        return removeFacet(ident, "");
    }

    public synchronized Ice.Object
    removeFacet(Identity ident, String facet)
    {
        checkForDeactivation();
        checkIdentity(ident);

        return _servantManager.removeServant(ident, facet);
    }

    public synchronized java.util.Map<String, Ice.Object>
    removeAllFacets(Identity ident)
    {
        checkForDeactivation();
        checkIdentity(ident);

        return _servantManager.removeAllFacets(ident);
    }

    public synchronized Ice.Object
    removeDefaultServant(String category)
    {
        checkForDeactivation();

        return _servantManager.removeDefaultServant(category);
    }

    public Ice.Object
    find(Identity ident)
    {
        return findFacet(ident, "");
    }

    public synchronized Ice.Object
    findFacet(Identity ident, String facet)
    {
        checkForDeactivation();
        checkIdentity(ident);

        return _servantManager.findServant(ident, facet);
    }

    public synchronized java.util.Map<String, Ice.Object>
    findAllFacets(Identity ident)
    {
        checkForDeactivation();
        checkIdentity(ident);

        return _servantManager.findAllFacets(ident);
    }

    public synchronized Ice.Object
    findByProxy(ObjectPrx proxy)
    {
        checkForDeactivation();

        IceInternal.Reference ref = ((ObjectPrxHelperBase)proxy).__reference();
        return findFacet(ref.getIdentity(), ref.getFacet());
    }

    public synchronized Ice.Object
    findDefaultServant(String category)
    {
        checkForDeactivation();

        return _servantManager.findDefaultServant(category);
    }

    public synchronized void
    addServantLocator(ServantLocator locator, String prefix)
    {
        checkForDeactivation();

        _servantManager.addServantLocator(locator, prefix);
    }

    public synchronized ServantLocator
    removeServantLocator(String prefix)
    {
        checkForDeactivation();

        return _servantManager.removeServantLocator(prefix);
    }

    public synchronized ServantLocator
    findServantLocator(String prefix)
    {
        checkForDeactivation();

        return _servantManager.findServantLocator(prefix);
    }

    public synchronized ObjectPrx
    createProxy(Identity ident)
    {
        checkForDeactivation();
        checkIdentity(ident);

        return newProxy(ident, "");
    }

    public synchronized ObjectPrx
    createDirectProxy(Identity ident)
    {
        checkForDeactivation();
        checkIdentity(ident);

        return newDirectProxy(ident, "");
    }

    public synchronized ObjectPrx
    createIndirectProxy(Identity ident)
    {
        checkForDeactivation();
        checkIdentity(ident);

        return newIndirectProxy(ident, "", _id);
    }

    public synchronized void
    setLocator(LocatorPrx locator)
    {
        checkForDeactivation();

        _locatorInfo = _instance.locatorManager().get(locator);
    }

    public void
    refreshPublishedEndpoints()
    {
        IceInternal.LocatorInfo locatorInfo = null;
        boolean registerProcess = false;
        java.util.List<IceInternal.EndpointI> oldPublishedEndpoints;

        synchronized(this)
        {
            checkForDeactivation();

            oldPublishedEndpoints = _publishedEndpoints;
            _publishedEndpoints = parsePublishedEndpoints();

            locatorInfo = _locatorInfo;
            if(!_noConfig)
            {
                registerProcess =
                    _instance.initializationData().properties.getPropertyAsInt(_name + ".RegisterProcess") > 0;
            }
        }

        try
        {
            Ice.Identity dummy = new Ice.Identity();
            dummy.name = "dummy";
            updateLocatorRegistry(locatorInfo, createDirectProxy(dummy), registerProcess);
        }
        catch(Ice.LocalException ex)
        {
            synchronized(this)
            {
                //
                // Restore the old published endpoints.
                //
                _publishedEndpoints = oldPublishedEndpoints;
                throw ex;
            }
        }
    }

    public synchronized Endpoint[]
    getEndpoints()
    {
        java.util.List<Endpoint> endpoints = new java.util.ArrayList<Endpoint>();
        for(IceInternal.IncomingConnectionFactory factory : _incomingConnectionFactories)
        {
            endpoints.add(factory.endpoint());
        }
        return endpoints.toArray(new Endpoint[0]);
    }

    public synchronized Endpoint[]
    getPublishedEndpoints()
    {
        return _publishedEndpoints.toArray(new Endpoint[0]);
    }

    public boolean
    isLocal(ObjectPrx proxy)
    {
        //
        // NOTE: it's important that isLocal() doesn't perform any blocking operations as
        // it can be called for AMI invocations if the proxy has no delegate set yet.
        //

        IceInternal.Reference ref = ((ObjectPrxHelperBase)proxy).__reference();
        if(ref.isWellKnown())
        {
            //
            // Check the active servant map to see if the well-known
            // proxy is for a local object.
            //
            return _servantManager.hasServant(ref.getIdentity());
        }
        else if(ref.isIndirect())
        {
            //
            // Proxy is local if the reference adapter id matches this
            // adapter id or replica group id.
            //
            return ref.getAdapterId().equals(_id) || ref.getAdapterId().equals(_replicaGroupId);
        }
        else
        {
            IceInternal.EndpointI[] endpoints = ref.getEndpoints();

            synchronized(this)
            {
                checkForDeactivation();

                //
                // Proxies which have at least one endpoint in common with the
                // endpoints used by this object adapter's incoming connection
                // factories are considered local.
                //
                for(IceInternal.EndpointI endpoint : endpoints)
                {
                    for(IceInternal.EndpointI p : _publishedEndpoints)
                    {
                        if(endpoint.equivalent(p))
                        {
                            return true;
                        }
                    }
                    for(IceInternal.IncomingConnectionFactory p : _incomingConnectionFactories)
                    {
                        if(endpoint.equivalent(p.endpoint()))
                        {
                            return true;
                        }
                    }
                }

                //
                // Proxies which have at least one endpoint in common with the
                // router's server proxy endpoints (if any), are also considered
                // local.
                //
                if(_routerInfo != null && _routerInfo.getRouter().equals(proxy.ice_getRouter()))
                {
                    for(IceInternal.EndpointI endpoint : endpoints)
                    {
                        for(IceInternal.EndpointI p : _routerEndpoints)
                        {
                            if(endpoint.equivalent(p))
                            {
                                return true;
                            }
                        }
                    }
                }
            }
        }

        return false;
    }

    public void
    flushAsyncBatchRequests(IceInternal.CommunicatorBatchOutgoingAsync outAsync)
    {
        java.util.List<IceInternal.IncomingConnectionFactory> f;
        synchronized(this)
        {
            f = new java.util.ArrayList<IceInternal.IncomingConnectionFactory>(_incomingConnectionFactories);
        }
        for(IceInternal.IncomingConnectionFactory p : f)
        {
            p.flushAsyncBatchRequests(outAsync);
        }
    }

    public void
    updateConnectionObservers()
    {
        java.util.List<IceInternal.IncomingConnectionFactory> f;
        synchronized(this)
        {
            f = new java.util.ArrayList<IceInternal.IncomingConnectionFactory>(_incomingConnectionFactories);
        }
        for(IceInternal.IncomingConnectionFactory p : f)
        {
            p.updateConnectionObservers();
        }
    }
    
    public void 
    updateThreadObservers()
    {
        IceInternal.ThreadPool threadPool = null;
        synchronized(this)
        {
            threadPool = _threadPool;
        }
        if(threadPool != null)
        {
            threadPool.updateObservers();
        }
    }

    public synchronized void
    incDirectCount()
    {
        checkForDeactivation();

        assert(_directCount >= 0);
        ++_directCount;
    }

    public synchronized void
    decDirectCount()
    {
        // Not check for deactivation here!

        assert(_instance != null); // Must not be called after destroy().

        assert(_directCount > 0);
        if(--_directCount == 0)
        {
            notifyAll();
        }
    }

    public IceInternal.ThreadPool
    getThreadPool()
    {
        // No mutex lock necessary, _threadPool and _instance are
        // immutable after creation until they are removed in
        // destroy().

        // Not check for deactivation here!

        assert(_instance != null); // Must not be called after destroy().

        if(_threadPool != null)
        {
            return _threadPool;
        }
        else
        {
            return _instance.serverThreadPool(true);
        }
    }

    public IceInternal.ServantManager
    getServantManager()
    {
        //
        // No mutex lock necessary, _servantManager is immutable.
        //
        return _servantManager;
    }

    public int
    getACM()
    {
        // Not check for deactivation here!

        assert(_instance != null); // Must not be called after destroy().

        if(_hasAcmTimeout)
        {
            return _acmTimeout;
        }
        else
        {
            return _instance.serverACM();
        }
    }

    //
    // Only for use by IceInternal.ObjectAdapterFactory
    //
    public
    ObjectAdapterI(IceInternal.Instance instance, Communicator communicator,
                   IceInternal.ObjectAdapterFactory objectAdapterFactory, String name,
                   RouterPrx router, boolean noConfig)
    {
        _deactivated = false;
        _instance = instance;
        _communicator = communicator;
        _objectAdapterFactory = objectAdapterFactory;
        _hasAcmTimeout = false;
        _acmTimeout = 0;
        _servantManager = new IceInternal.ServantManager(instance, name);
        _activateOneOffDone = false;
        _name = name;
        _directCount = 0;
        _waitForActivate = false;
        _waitForHold = 0;
        _waitForHoldRetry = false;
        _destroying = false;
        _destroyed = false;
        _noConfig = noConfig;

        if(_noConfig)
        {
            _id = "";
            _replicaGroupId = "";
            _reference = _instance.referenceFactory().create("dummy -t", "");
            return;
        }

        final Properties properties = _instance.initializationData().properties;
        java.util.List<String> unknownProps = new java.util.ArrayList<String>();
        boolean noProps = filterProperties(unknownProps);

        //
        // Warn about unknown object adapter properties.
        //
        if(unknownProps.size() != 0 && properties.getPropertyAsIntWithDefault("Ice.Warn.UnknownProperties", 1) > 0)
        {
            StringBuffer message = new StringBuffer("found unknown properties for object adapter `");
            message.append(_name);
            message.append("':");
            for(String p : unknownProps)
            {
                message.append("\n    ");
                message.append(p);
            }
            _instance.initializationData().logger.warning(message.toString());
        }

        //
        // Make sure named adapter has some configuration.
        //
        if(router == null && noProps)
        {
            //
            // These need to be set to prevent finalizer from complaining.
            //
            _deactivated = true;
            _destroyed = true;
            _instance = null;
            _incomingConnectionFactories = null;

            InitializationException ex = new InitializationException();
            ex.reason = "object adapter `" + _name + "' requires configuration";
            throw ex;
        }

        _id = properties.getProperty(_name + ".AdapterId");
        _replicaGroupId = properties.getProperty(_name + ".ReplicaGroupId");

        //
        // Setup a reference to be used to get the default proxy options
        // when creating new proxies. By default, create twoway proxies.
        //
        String proxyOptions = properties.getPropertyWithDefault(_name + ".ProxyOptions", "-t");
        try
        {
            _reference = _instance.referenceFactory().create("dummy " + proxyOptions, "");
        }
        catch(ProxyParseException e)
        {
            InitializationException ex = new InitializationException();
            ex.reason = "invalid proxy options `" + proxyOptions + "' for object adapter `" + _name + "'";
            throw ex;
        }

        try
        {
            int threadPoolSize = properties.getPropertyAsInt(_name + ".ThreadPool.Size");
            int threadPoolSizeMax = properties.getPropertyAsInt(_name + ".ThreadPool.SizeMax");

            //
            // Create the per-adapter thread pool, if necessary.
            //
            if(threadPoolSize > 0 || threadPoolSizeMax > 0)
            {
                _threadPool = new IceInternal.ThreadPool(_instance, _name + ".ThreadPool", 0);
            }

            _hasAcmTimeout = properties.getProperty(_name + ".ACM").length() > 0;
            if(_hasAcmTimeout)
            {
                _acmTimeout = properties.getPropertyAsInt(_name + ".ACM");
                _instance.connectionMonitor().checkIntervalForACM(_acmTimeout);
            }

            if(router == null)
            {
                router = RouterPrxHelper.uncheckedCast(_instance.proxyFactory().propertyToProxy(name + ".Router"));
            }
            if(router != null)
            {
                _routerInfo = _instance.routerManager().get(router);
                if(_routerInfo != null)
                {
                    //
                    // Make sure this router is not already registered with another adapter.
                    //
                    if(_routerInfo.getAdapter() != null)
                    {
                        throw new AlreadyRegisteredException("object adapter with router",
                                                             _instance.identityToString(router.ice_getIdentity()));
                    }

                    //
                    // Add the router's server proxy endpoints to this object
                    // adapter.
                    //
                    IceInternal.EndpointI[] endpoints = _routerInfo.getServerEndpoints();
                    for(IceInternal.EndpointI endpoint : endpoints)
                    {
                        _routerEndpoints.add(endpoint);
                    }
                    java.util.Collections.sort(_routerEndpoints); // Must be sorted.

                    //
                    // Remove duplicate endpoints, so we have a list of unique
                    // endpoints.
                    //
                    for(int i = 0; i < _routerEndpoints.size() - 1;)
                    {
                        IceInternal.EndpointI e1 = _routerEndpoints.get(i);
                        IceInternal.EndpointI e2 = _routerEndpoints.get(i + 1);
                        if(e1.equals(e2))
                        {
                            _routerEndpoints.remove(i);
                        }
                        else
                        {
                            ++i;
                        }
                    }

                    //
                    // Associate this object adapter with the router. This way,
                    // new outgoing connections to the router's client proxy will
                    // use this object adapter for callbacks.
                    //
                    _routerInfo.setAdapter(this);

                    //
                    // Also modify all existing outgoing connections to the
                    // router's client proxy to use this object adapter for
                    // callbacks.
                    //
                    _instance.outgoingConnectionFactory().setRouterInfo(_routerInfo);
                }
            }
            else
            {
                //
                // Parse the endpoints, but don't store them in the adapter. The connection
                // factory might change it, for example, to fill in the real port number.
                //
                java.util.List<IceInternal.EndpointI> endpoints =
                    parseEndpoints(properties.getProperty(_name + ".Endpoints"), true);
                for(IceInternal.EndpointI endp : endpoints)
                {
                    IceInternal.IncomingConnectionFactory factory =
                        new IceInternal.IncomingConnectionFactory(instance, endp, this, _name);
                    _incomingConnectionFactories.add(factory);
                }
                if(endpoints.size() == 0)
                {
                    IceInternal.TraceLevels tl = _instance.traceLevels();
                    if(tl.network >= 2)
                    {
                        _instance.initializationData().logger.trace(tl.networkCat,
                                                                    "created adapter `" + name + "' without endpoints");
                    }
                }

                //
                // Parse the publsihed endpoints.
                //
                _publishedEndpoints = parsePublishedEndpoints();
            }

            if(properties.getProperty(_name + ".Locator").length() > 0)
            {
                setLocator(LocatorPrxHelper.uncheckedCast(
                    _instance.proxyFactory().propertyToProxy(_name + ".Locator")));
            }
            else
            {
                setLocator(_instance.referenceFactory().getDefaultLocator());
            }
        }
        catch(LocalException ex)
        {
            destroy();
            throw ex;
        }
    }

    protected synchronized void
    finalize()
        throws Throwable
    {
        try
        {
            if(!_deactivated)
            {
                _instance.initializationData().logger.warning("object adapter `" + getName() +
                                                            "' has not been deactivated");
            }
            else if(!_destroyed)
            {
                _instance.initializationData().logger.warning("object adapter `" + getName() + "' has not been destroyed");
            }
            else
            {
                IceUtilInternal.Assert.FinalizerAssert(_threadPool == null);
                //IceUtilInternal.Assert.FinalizerAssert(_servantManager == null); // Not cleared, it needs to be immutable.
                //IceUtilInternal.Assert.FinalizerAssert(_incomingConnectionFactories.isEmpty());
                IceUtilInternal.Assert.FinalizerAssert(_directCount == 0);
                IceUtilInternal.Assert.FinalizerAssert(!_waitForActivate);
            }
        }
        catch(java.lang.Exception ex)
        {
        }
        finally
        {
            super.finalize();
        }
    }

    private ObjectPrx
    newProxy(Identity ident, String facet)
    {
        if(_id.length() == 0)
        {
            return newDirectProxy(ident, facet);
        }
        else if(_replicaGroupId.length() == 0)
        {
            return newIndirectProxy(ident, facet, _id);
        }
        else
        {
            return newIndirectProxy(ident, facet, _replicaGroupId);
        }
    }

    private ObjectPrx
    newDirectProxy(Identity ident, String facet)
    {
        IceInternal.EndpointI[] endpoints;

        int sz = _publishedEndpoints.size();
        endpoints = new IceInternal.EndpointI[sz + _routerEndpoints.size()];
        _publishedEndpoints.toArray(endpoints);

        //
        // Now we also add the endpoints of the router's server proxy, if
        // any. This way, object references created by this object adapter
        // will also point to the router's server proxy endpoints.
        //
        for(int i = 0; i < _routerEndpoints.size(); ++i)
        {
            endpoints[sz + i] = (IceInternal.EndpointI)_routerEndpoints.get(i);
        }

        //
        // Create a reference and return a proxy for this reference.
        //
        IceInternal.Reference ref = _instance.referenceFactory().create(ident, facet, _reference, endpoints);
        return _instance.proxyFactory().referenceToProxy(ref);
    }

    private ObjectPrx
    newIndirectProxy(Identity ident, String facet, String id)
    {
        //
        // Create a reference with the adapter id and return a proxy
        // for the reference.
        //
        IceInternal.Reference ref = _instance.referenceFactory().create(ident, facet, _reference, id);
        return _instance.proxyFactory().referenceToProxy(ref);
    }

    private void
    checkForDeactivation()
    {
        if(_deactivated)
        {
            ObjectAdapterDeactivatedException ex = new ObjectAdapterDeactivatedException();
            ex.name = getName();
            throw ex;
        }
    }

    private static void
    checkIdentity(Identity ident)
    {
        if(ident.name == null || ident.name.length() == 0)
        {
            IllegalIdentityException e = new IllegalIdentityException();
            e.id = (Identity)ident.clone();
            throw e;
        }

        if(ident.category == null)
        {
            ident.category = "";
        }
    }

    private java.util.List<IceInternal.EndpointI>
    parseEndpoints(String endpts, boolean oaEndpoints)
    {
        int beg;
        int end = 0;

        final String delim = " \t\n\r";

        java.util.List<IceInternal.EndpointI> endpoints = new java.util.ArrayList<IceInternal.EndpointI>();
        while(end < endpts.length())
        {
            beg = IceUtilInternal.StringUtil.findFirstNotOf(endpts, delim, end);
            if(beg == -1)
            {
                break;
            }

            end = beg;
            while(true)
            {
                end = endpts.indexOf(':', end);
                if(end == -1)
                {
                    end = endpts.length();
                    break;
                }
                else
                {
                    boolean quoted = false;
                    int quote = beg;
                    while(true)
                    {
                        quote = endpts.indexOf('\"', quote);
                        if(quote == -1 || end < quote)
                        {
                            break;
                        }
                        else
                        {
                            quote = endpts.indexOf('\"', ++quote);
                            if(quote == -1)
                            {
                                break;
                            }
                            else if(end < quote)
                            {
                                quoted = true;
                                break;
                            }
                            ++quote;
                        }
                    }
                    if(!quoted)
                    {
                        break;
                    }
                    ++end;
                }
            }

            if(end == beg)
            {
                ++end;
                continue;
            }

            String s = endpts.substring(beg, end);
            IceInternal.EndpointI endp = _instance.endpointFactoryManager().create(s, oaEndpoints);
            if(endp == null)
            {
                Ice.EndpointParseException e = new Ice.EndpointParseException();
                e.str = "invalid object adapter endpoint `" + s + "'";
                throw e;
            }
            endpoints.add(endp);

            ++end;
        }

        return endpoints;
    }

    private java.util.List<IceInternal.EndpointI>
    parsePublishedEndpoints()
    {
        //
        // Parse published endpoints. If set, these are used in proxies
        // instead of the connection factory Endpoints.
        //
        String endpts = _instance.initializationData().properties.getProperty(_name + ".PublishedEndpoints");
        java.util.List<IceInternal.EndpointI> endpoints = parseEndpoints(endpts, false);
        if(endpoints.isEmpty())
        {
            //
            // If the PublishedEndpoints property isn't set, we compute the published enpdoints
            // from the OA endpoints, expanding any endpoints that may be listening on INADDR_ANY
            // to include actual addresses in the published endpoints.
            //
            for(IceInternal.IncomingConnectionFactory factory : _incomingConnectionFactories)
            {
                endpoints.addAll(factory.endpoint().expand());
            }
        }

        if(_instance.traceLevels().network >= 1)
        {
            StringBuffer s = new StringBuffer("published endpoints for object adapter `");
            s.append(_name);
            s.append("':\n");
            boolean first = true;
            for(IceInternal.EndpointI endpoint : endpoints)
            {
                if(!first)
                {
                    s.append(":");
                }
                s.append(endpoint.toString());
                first = false;
            }
            _instance.initializationData().logger.trace(_instance.traceLevels().networkCat, s.toString());
        }
        return endpoints;
    }

    private void
    updateLocatorRegistry(IceInternal.LocatorInfo locatorInfo, Ice.ObjectPrx proxy, boolean registerProcess)
    {
        if(!registerProcess && _id.length() == 0)
        {
            return; // Nothing to update.
        }

        //
        // Call on the locator registry outside the synchronization to
        // blocking other threads that need to lock this OA.
        //
        LocatorRegistryPrx locatorRegistry = locatorInfo != null ? locatorInfo.getLocatorRegistry() : null;
        String serverId = "";
        if(registerProcess)
        {
            assert(_instance != null);
            serverId = _instance.initializationData().properties.getProperty("Ice.ServerId");

            if(locatorRegistry == null)
            {
                _instance.initializationData().logger.warning(
                    "object adapter `" + getName() + "' cannot register the process without a locator registry");
            }
            else if(serverId.length() == 0)
            {
                _instance.initializationData().logger.warning(
                    "object adapter `" + getName() + "' cannot register the process without a value for Ice.ServerId");
            }
        }

        if(locatorRegistry == null)
        {
            return;
        }

        if(_id.length() > 0)
        {
            try
            {
                if(_replicaGroupId.length() == 0)
                {
                    locatorRegistry.setAdapterDirectProxy(_id, proxy);
                }
                else
                {
                    locatorRegistry.setReplicatedAdapterDirectProxy(_id, _replicaGroupId, proxy);
                }
            }
            catch(AdapterNotFoundException ex)
            {
                if(_instance.traceLevels().location >= 1)
                {
                    StringBuilder s = new StringBuilder(128);
                    s.append("couldn't update object adapter `");
                    s.append(_id);
                    s.append("' endpoints with the locator registry:\n");
                    s.append("the object adapter is not known to the locator registry");
                    _instance.initializationData().logger.trace(_instance.traceLevels().locationCat, s.toString());
                }

                NotRegisteredException ex1 = new NotRegisteredException();
                ex1.kindOfObject = "object adapter";
                ex1.id = _id;
                throw ex1;
            }
            catch(InvalidReplicaGroupIdException ex)
            {
                if(_instance.traceLevels().location >= 1)
                {
                    StringBuilder s = new StringBuilder(128);
                    s.append("couldn't update object adapter `");
                    s.append(_id);
                    s.append("' endpoints with the locator registry:\n");
                    s.append("the replica group `");
                    s.append(_replicaGroupId);
                    s.append("' is not known to the locator registry");
                    _instance.initializationData().logger.trace(_instance.traceLevels().locationCat, s.toString());
                }

                NotRegisteredException ex1 = new NotRegisteredException();
                ex1.kindOfObject = "replica group";
                ex1.id = _replicaGroupId;
                throw ex1;
            }
            catch(AdapterAlreadyActiveException ex)
            {
                if(_instance.traceLevels().location >= 1)
                {
                    StringBuilder s = new StringBuilder(128);
                    s.append("couldn't update object adapter `");
                    s.append(_id);
                    s.append("' endpoints with the locator registry:\n");
                    s.append("the object adapter endpoints are already set");
                    _instance.initializationData().logger.trace(_instance.traceLevels().locationCat, s.toString());
                }

                ObjectAdapterIdInUseException ex1 = new ObjectAdapterIdInUseException();
                ex1.id = _id;
                throw ex1;
            }
            catch(LocalException e)
            {
                if(_instance.traceLevels().location >= 1)
                {
                    StringBuilder s = new StringBuilder(128);
                    s.append("couldn't update object adapter `");
                    s.append(_id);
                    s.append("' endpoints with the locator registry:\n");
                    s.append(e.toString());
                    _instance.initializationData().logger.trace(_instance.traceLevels().locationCat, s.toString());
                }
                throw e; // TODO: Shall we raise a special exception instead of a non obvious local exception?
            }

            if(_instance.traceLevels().location >= 1)
            {
                StringBuilder s = new StringBuilder(128);
                s.append("updated object adapter `");
                s.append(_id);
                s.append("' endpoints with the locator registry\n");
                s.append("endpoints = ");
                if(proxy != null)
                {
                    Ice.Endpoint[] endpoints = proxy.ice_getEndpoints();
                    for(int i = 0; i < endpoints.length; i++)
                    {
                        s.append(endpoints[i].toString());
                        if(i + 1 < endpoints.length)
                        {
                            s.append(":");
                        }
                    }
                }
                _instance.initializationData().logger.trace(_instance.traceLevels().locationCat, s.toString());
            }
        }

        if(registerProcess && serverId.length() > 0)
        {
            synchronized(this)
            {
                if(_processId == null)
                {
                    Process servant = new IceInternal.ProcessI(_communicator);
                    _processId = addWithUUID(servant).ice_getIdentity();
                }
            }

            try
            {
                locatorRegistry.setServerProcessProxy(serverId,
                                        ProcessPrxHelper.uncheckedCast(createDirectProxy(_processId)));
            }
            catch(ServerNotFoundException ex)
            {
                if(_instance.traceLevels().location >= 1)
                {
                    StringBuilder s = new StringBuilder(128);
                    s.append("couldn't register server `");
                    s.append(serverId);
                    s.append("' with the locator registry:\n");
                    s.append("the server is not known to the locator registry");
                    _instance.initializationData().logger.trace(_instance.traceLevels().locationCat, s.toString());
                }

                NotRegisteredException ex1 = new NotRegisteredException();
                ex1.id = serverId;
                ex1.kindOfObject = "server";
                throw ex1;
            }
            catch(LocalException ex)
            {
                if(_instance.traceLevels().location >= 1)
                {
                    StringBuilder s = new StringBuilder(128);
                    s.append("couldn't register server `");
                    s.append(serverId);
                    s.append("' with the locator registry:\n");
                    s.append(ex.toString());
                    _instance.initializationData().logger.trace(_instance.traceLevels().locationCat, s.toString());
                }
                throw ex; // TODO: Shall we raise a special exception instead of a non-obvious local exception?
            }

            if(_instance.traceLevels().location >= 1)
            {
                StringBuilder s = new StringBuilder(128);
                s.append("registered server `");
                s.append(serverId);
                s.append("' with the locator registry");
                _instance.initializationData().logger.trace(_instance.traceLevels().locationCat, s.toString());
            }
        }
    }

    static private String[] _suffixes =
    {
        "ACM",
        "AdapterId",
        "Endpoints",
        "Locator",
        "Locator.EncodingVersion",
        "Locator.EndpointSelection",
        "Locator.ConnectionCached",
        "Locator.PreferSecure",
        "Locator.CollocationOptimized",
        "Locator.Router",
        "PublishedEndpoints",
        "RegisterProcess",
        "ReplicaGroupId",
        "Router",
        "Router.EncodingVersion",
        "Router.EndpointSelection",
        "Router.ConnectionCached",
        "Router.PreferSecure",
        "Router.CollocationOptimized",
        "Router.Locator",
        "Router.Locator.EndpointSelection",
        "Router.Locator.ConnectionCached",
        "Router.Locator.PreferSecure",
        "Router.Locator.CollocationOptimized",
        "Router.Locator.LocatorCacheTimeout",
        "Router.LocatorCacheTimeout",
        "ProxyOptions",
        "ThreadPool.Size",
        "ThreadPool.SizeMax",
        "ThreadPool.SizeWarn",
        "ThreadPool.StackSize",
        "ThreadPool.Serialize"
    };

    boolean
    filterProperties(java.util.List<String> unknownProps)
    {
        //
        // Do not create unknown properties list if Ice prefix, ie Ice, Glacier2, etc
        //
        boolean addUnknown = true;
        String prefix = _name + ".";
        for(int i = 0; IceInternal.PropertyNames.clPropNames[i] != null; ++i)
        {
            if(prefix.startsWith(IceInternal.PropertyNames.clPropNames[i] + "."))
            {
                addUnknown = false;
                break;
            }
        }

        boolean noProps = true;
        java.util.Map<String, String> props = _instance.initializationData().properties.getPropertiesForPrefix(prefix);
        for(String prop : props.keySet())
        {
            boolean valid = false;
            for(String suffix : _suffixes)
            {
                if(prop.equals(prefix + suffix))
                {
                    noProps = false;
                    valid = true;
                    break;
                }
            }

            if(!valid && addUnknown)
            {
                unknownProps.add(prop);
            }
        }

        return noProps;
    }

    private boolean _deactivated;
    private IceInternal.Instance _instance;
    private Communicator _communicator;
    private IceInternal.ObjectAdapterFactory _objectAdapterFactory;
    private IceInternal.ThreadPool _threadPool;
    private boolean _hasAcmTimeout;
    private int _acmTimeout;
    private IceInternal.ServantManager _servantManager;
    private boolean _activateOneOffDone;
    final private String _name;
    final private String _id;
    final private String _replicaGroupId;
    private IceInternal.Reference _reference;
    private java.util.List<IceInternal.IncomingConnectionFactory> _incomingConnectionFactories =
        new java.util.ArrayList<IceInternal.IncomingConnectionFactory>();
    private java.util.List<IceInternal.EndpointI> _routerEndpoints = new java.util.ArrayList<IceInternal.EndpointI>();
    private IceInternal.RouterInfo _routerInfo = null;
    private java.util.List<IceInternal.EndpointI> _publishedEndpoints =
        new java.util.ArrayList<IceInternal.EndpointI>();
    private IceInternal.LocatorInfo _locatorInfo;
    private int _directCount;
    private boolean _waitForActivate;
    private int _waitForHold;
    private boolean _waitForHoldRetry;
    private boolean _destroying;
    private boolean _destroyed;
    private boolean _noConfig;
    private Identity _processId = null;
}
