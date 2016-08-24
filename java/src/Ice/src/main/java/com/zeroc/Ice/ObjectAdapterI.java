// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.Ice;

import java.util.Map;
import java.util.List;
import java.util.ArrayList;

import com.zeroc.IceInternal.IncomingConnectionFactory;

public final class ObjectAdapterI implements ObjectAdapter
{
    @Override
    public String
    getName()
    {
        //
        // No mutex lock necessary, _name is immutable.
        //
        return _noConfig ? "" : _name;
    }

    @Override
    public Communicator
    getCommunicator()
    {
        return _communicator;
    }

    @Override
    public void
    activate()
    {
        com.zeroc.IceInternal.LocatorInfo locatorInfo = null;
        boolean printAdapterReady = false;

        synchronized(this)
        {
            checkForDeactivation();

            //
            // If we've previously been initialized we just need to activate the
            // incoming connection factories and we're done.
            //
            if(_state != StateUninitialized)
            {
                for(IncomingConnectionFactory factory : _incomingConnectionFactories)
                {
                    factory.activate();
                }
                return;
            }

            //
            // One off initializations of the adapter: update the
            // locator registry and print the "adapter ready"
            // message. We set set state to StateActivating to prevent
            // deactivation from other threads while these one off
            // initializations are done.
            //
            _state = StateActivating;

            locatorInfo = _locatorInfo;
            if(!_noConfig)
            {
                final Properties properties = _instance.initializationData().properties;
                printAdapterReady = properties.getPropertyAsInt("Ice.PrintAdapterReady") > 0;
            }
        }

        try
        {
            Identity dummy = new Identity();
            dummy.name = "dummy";
            updateLocatorRegistry(locatorInfo, createDirectProxy(dummy));
        }
        catch(LocalException ex)
        {
            //
            // If we couldn't update the locator registry, we let the
            // exception go through and don't activate the adapter to
            // allow to user code to retry activating the adapter
            // later.
            //
            synchronized(this)
            {
                _state = StateUninitialized;
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
            assert(_state == StateActivating);

            //
            // Signal threads waiting for the activation.
            //
            _state = StateActive;
            notifyAll();

            for(IncomingConnectionFactory factory : _incomingConnectionFactories)
            {
                factory.activate();
            }
        }
    }

    @Override
    public synchronized void
    hold()
    {
        checkForDeactivation();
        _state = StateHeld;
        for(IncomingConnectionFactory factory : _incomingConnectionFactories)
        {
            factory.hold();
        }
    }

    @Override
    public void
    waitForHold()
    {
        if(Thread.interrupted())
        {
            throw new OperationInterruptedException();
        }

        List<IncomingConnectionFactory> incomingConnectionFactories;
        synchronized(this)
        {
            checkForDeactivation();
            incomingConnectionFactories = new ArrayList<>(_incomingConnectionFactories);
        }

        for(IncomingConnectionFactory factory : incomingConnectionFactories)
        {
            try
            {
                factory.waitUntilHolding();
            }
            catch(InterruptedException ex)
            {
                throw new OperationInterruptedException();
            }
        }
    }


    @Override
    public void
    deactivate()
    {
        if(Thread.interrupted())
        {
            throw new OperationInterruptedException();
        }

        synchronized(this)
        {
            //
            // Wait for activation to complete. This is necessary to
            // not get out of order locator updates.
            //
            while(_state == StateActivating)
            {
                try
                {
                    wait();
                }
                catch(InterruptedException ex)
                {
                    throw new OperationInterruptedException();
                }
            }
            if(_state > StateDeactivating)
            {
                return;
            }
            _state = StateDeactivating;
        }

        //
        // NOTE: the router/locator infos and incoming connection
        // facatory list are immutable at this point.
        //

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

        try
        {
            updateLocatorRegistry(_locatorInfo, null);
        }
        catch(LocalException ex)
        {
            //
            // We can't throw exceptions in deactivate so we ignore
            // failures to update the locator registry.
            //
        }

        //
        // Must be called outside the thread synchronization, because
        // Connection::destroy() might block when sending a
        // CloseConnection message.
        //
        for(IncomingConnectionFactory factory : _incomingConnectionFactories)
        {
            factory.destroy();
        }

        //
        // Must be called outside the thread synchronization, because
        // changing the object adapter might block if there are still
        // requests being dispatched.
        //
        _instance.outgoingConnectionFactory().removeAdapter(this);

        synchronized(this)
        {
            _state = StateDeactivated;
            notifyAll();
        }
    }

    @Override
    public void
    waitForDeactivate()
    {
        if(Thread.interrupted())
        {
            throw new OperationInterruptedException();
        }

        try
        {
            List<IncomingConnectionFactory> incomingConnectionFactories;
            synchronized(this)
            {
                //
                // Wait for deactivation of the adapter itself, and
                // for the return of all direct method calls using
                // this adapter.
                //
                while((_state < StateDeactivated) || _directCount > 0)
                {
                    wait();
                }
                if(_state > StateDeactivated)
                {
                    return;
                }
                incomingConnectionFactories = new ArrayList<>(_incomingConnectionFactories);
            }

            //
            // Now we wait for until all incoming connection factories are
            // finished (the incoming connection factory list is immutable
            // at this point).
            //
            for(IncomingConnectionFactory f : incomingConnectionFactories)
            {
                f.waitUntilFinished();
            }
        }
        catch(InterruptedException e)
        {
            throw new OperationInterruptedException();
        }
    }

    @Override
    public synchronized boolean
    isDeactivated()
    {
        return _state >= StateDeactivated;
    }

    @Override
    public void
    destroy()
    {
        if(Thread.interrupted())
        {
            throw new OperationInterruptedException();
        }

        //
        // Deactivate and wait for completion.
        //
        deactivate();
        waitForDeactivate();

        synchronized(this)
        {
            assert(_state >= StateDeactivated);

            //
            // Only a single thread is allowed to destroy the object
            // adapter. Other threads wait for the destruction to be
            // completed.
            //
            while(_state == StateDestroying)
            {
                try
                {
                    wait();
                }
                catch(InterruptedException ex)
                {
                    throw new OperationInterruptedException();
                }
            }
            if(_state == StateDestroyed)
            {
                return;
            }

            _state = StateDestroying;
        }

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
            try
            {
                _threadPool.joinWithAllThreads();
            }
            catch (InterruptedException e)
            {
                throw new OperationInterruptedException();
            }
        }

        _objectAdapterFactory.removeObjectAdapter(this);

        synchronized(this)
        {
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
            _objectAdapterFactory = null;

            //
            // Signal that destroying is complete.
            //
            _state = StateDestroyed;
            notifyAll();
        }
    }

    @Override
    public ObjectPrx
    add(com.zeroc.Ice.Object object, Identity ident)
    {
        return addFacet(object, ident, "");
    }

    @Override
    public synchronized ObjectPrx
    addFacet(com.zeroc.Ice.Object object, Identity ident, String facet)
    {
        checkForDeactivation();
        checkIdentity(ident);
        checkServant(object);

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

    @Override
    public ObjectPrx
    addWithUUID(com.zeroc.Ice.Object object)
    {
        return addFacetWithUUID(object, "");
    }

    @Override
    public ObjectPrx
    addFacetWithUUID(com.zeroc.Ice.Object object, String facet)
    {
        Identity ident = new Identity();
        ident.category = "";
        ident.name = java.util.UUID.randomUUID().toString();

        return addFacet(object, ident, facet);
    }

    @Override
    public synchronized void
    addDefaultServant(com.zeroc.Ice.Object servant, String category)
    {
        checkServant(servant);
        checkForDeactivation();

        _servantManager.addDefaultServant(servant, category);
    }

    @Override
    public com.zeroc.Ice.Object
    remove(Identity ident)
    {
        return removeFacet(ident, "");
    }

    @Override
    public synchronized com.zeroc.Ice.Object
    removeFacet(Identity ident, String facet)
    {
        checkForDeactivation();
        checkIdentity(ident);

        return _servantManager.removeServant(ident, facet);
    }

    @Override
    public synchronized Map<String, Object>
    removeAllFacets(Identity ident)
    {
        checkForDeactivation();
        checkIdentity(ident);

        return _servantManager.removeAllFacets(ident);
    }

    @Override
    public synchronized com.zeroc.Ice.Object
    removeDefaultServant(String category)
    {
        checkForDeactivation();

        return _servantManager.removeDefaultServant(category);
    }

    @Override
    public com.zeroc.Ice.Object
    find(Identity ident)
    {
        return findFacet(ident, "");
    }

    @Override
    public synchronized com.zeroc.Ice.Object
    findFacet(Identity ident, String facet)
    {
        checkForDeactivation();
        checkIdentity(ident);

        return _servantManager.findServant(ident, facet);
    }

    @Override
    public synchronized java.util.Map<String, com.zeroc.Ice.Object>
    findAllFacets(Identity ident)
    {
        checkForDeactivation();
        checkIdentity(ident);

        return _servantManager.findAllFacets(ident);
    }

    @Override
    public synchronized com.zeroc.Ice.Object
    findByProxy(ObjectPrx proxy)
    {
        checkForDeactivation();

        com.zeroc.IceInternal.Reference ref = ((_ObjectPrxI)proxy).__reference();
        return findFacet(ref.getIdentity(), ref.getFacet());
    }

    @Override
    public synchronized com.zeroc.Ice.Object
    findDefaultServant(String category)
    {
        checkForDeactivation();

        return _servantManager.findDefaultServant(category);
    }

    @Override
    public synchronized void
    addServantLocator(ServantLocator locator, String prefix)
    {
        checkForDeactivation();

        _servantManager.addServantLocator(locator, prefix);
    }

    @Override
    public synchronized ServantLocator
    removeServantLocator(String prefix)
    {
        checkForDeactivation();

        return _servantManager.removeServantLocator(prefix);
    }

    @Override
    public synchronized ServantLocator
    findServantLocator(String prefix)
    {
        checkForDeactivation();

        return _servantManager.findServantLocator(prefix);
    }

    @Override
    public synchronized ObjectPrx
    createProxy(Identity ident)
    {
        checkForDeactivation();
        checkIdentity(ident);

        return newProxy(ident, "");
    }

    @Override
    public synchronized ObjectPrx
    createDirectProxy(Identity ident)
    {
        checkForDeactivation();
        checkIdentity(ident);

        return newDirectProxy(ident, "");
    }

    @Override
    public synchronized ObjectPrx
    createIndirectProxy(Identity ident)
    {
        checkForDeactivation();
        checkIdentity(ident);

        return newIndirectProxy(ident, "", _id);
    }

    @Override
    public synchronized void
    setLocator(LocatorPrx locator)
    {
        checkForDeactivation();

        _locatorInfo = _instance.locatorManager().get(locator);
    }

    @Override
    public synchronized LocatorPrx
    getLocator()
    {
        if(_locatorInfo == null)
        {
            return null;
        }
        else
        {
            return _locatorInfo.getLocator();
        }
    }

    @Override
    public void
    refreshPublishedEndpoints()
    {
        com.zeroc.IceInternal.LocatorInfo locatorInfo = null;
        List<com.zeroc.IceInternal.EndpointI> oldPublishedEndpoints;

        synchronized(this)
        {
            checkForDeactivation();

            oldPublishedEndpoints = _publishedEndpoints;
            _publishedEndpoints = parsePublishedEndpoints();

            locatorInfo = _locatorInfo;
        }

        try
        {
            Identity dummy = new Identity();
            dummy.name = "dummy";
            updateLocatorRegistry(locatorInfo, createDirectProxy(dummy));
        }
        catch(LocalException ex)
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

    @Override
    public synchronized Endpoint[]
    getEndpoints()
    {
        List<Endpoint> endpoints = new ArrayList<>();
        for(IncomingConnectionFactory factory : _incomingConnectionFactories)
        {
            endpoints.add(factory.endpoint());
        }
        return endpoints.toArray(new Endpoint[0]);
    }

    @Override
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

        com.zeroc.IceInternal.Reference ref = ((_ObjectPrxI)proxy).__reference();
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
            com.zeroc.IceInternal.EndpointI[] endpoints = ref.getEndpoints();

            synchronized(this)
            {
                checkForDeactivation();

                //
                // Proxies which have at least one endpoint in common with the
                // endpoints used by this object adapter's incoming connection
                // factories are considered local.
                //
                for(com.zeroc.IceInternal.EndpointI endpoint : endpoints)
                {
                    for(com.zeroc.IceInternal.EndpointI p : _publishedEndpoints)
                    {
                        if(endpoint.equivalent(p))
                        {
                            return true;
                        }
                    }
                    for(IncomingConnectionFactory p : _incomingConnectionFactories)
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
                    for(com.zeroc.IceInternal.EndpointI endpoint : endpoints)
                    {
                        for(com.zeroc.IceInternal.EndpointI p : _routerEndpoints)
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
    flushAsyncBatchRequests(com.zeroc.IceInternal.CommunicatorFlushBatch outAsync)
    {
        List<IncomingConnectionFactory> f;
        synchronized(this)
        {
            f = new ArrayList<>(_incomingConnectionFactories);
        }
        for(IncomingConnectionFactory p : f)
        {
            p.flushAsyncBatchRequests(outAsync);
        }
    }

    public void
    updateConnectionObservers()
    {
        List<IncomingConnectionFactory> f;
        synchronized(this)
        {
            f = new ArrayList<>(_incomingConnectionFactories);
        }
        for(IncomingConnectionFactory p : f)
        {
            p.updateConnectionObservers();
        }
    }

    public void
    updateThreadObservers()
    {
        com.zeroc.IceInternal.ThreadPool threadPool = null;
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

    public com.zeroc.IceInternal.ThreadPool
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
            return _instance.serverThreadPool();
        }
    }

    public com.zeroc.IceInternal.ServantManager
    getServantManager()
    {
        //
        // No mutex lock necessary, _servantManager is immutable.
        //
        return _servantManager;
    }

    public com.zeroc.IceInternal.ACMConfig
    getACM()
    {
        // No check for deactivation here!
        assert(_instance != null); // Must not be called after destroy().
        return _acm;
    }

    public int
    messageSizeMax()
    {
        // No mutex lock, immutable.
        return _messageSizeMax;
    }

    //
    // Only for use by com.zeroc.IceInternal.ObjectAdapterFactory
    //
    public
    ObjectAdapterI(com.zeroc.IceInternal.Instance instance, Communicator communicator,
                   com.zeroc.IceInternal.ObjectAdapterFactory objectAdapterFactory, String name,
                   RouterPrx router, boolean noConfig)
    {
        _instance = instance;
        _communicator = communicator;
        _objectAdapterFactory = objectAdapterFactory;
        _servantManager = new com.zeroc.IceInternal.ServantManager(instance, name);
        _name = name;
        _directCount = 0;
        _noConfig = noConfig;

        if(_noConfig)
        {
            _id = "";
            _replicaGroupId = "";
            _reference = _instance.referenceFactory().create("dummy -t", "");
            _acm = _instance.serverACM();
            _messageSizeMax = _instance.messageSizeMax();
            return;
        }

        final Properties properties = _instance.initializationData().properties;
        List<String> unknownProps = new ArrayList<>();
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
            _state = StateDestroyed;
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

        _acm = new com.zeroc.IceInternal.ACMConfig(properties, communicator.getLogger(), _name + ".ACM",
                                                   instance.serverACM());

        {
            final int defaultMessageSizeMax = instance.messageSizeMax() / 1024;
            int num = properties.getPropertyAsIntWithDefault(_name + ".MessageSizeMax", defaultMessageSizeMax);
            if(num < 1 || num > 0x7fffffff / 1024)
            {
                _messageSizeMax = 0x7fffffff;
            }
            else
            {
                _messageSizeMax = num * 1024; // Property is in kilobytes, _messageSizeMax in bytes
            }
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
                _threadPool = new com.zeroc.IceInternal.ThreadPool(_instance, _name + ".ThreadPool", 0);
            }

            if(router == null)
            {
                router = RouterPrx.uncheckedCast(_instance.proxyFactory().propertyToProxy(name + ".Router"));
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
                                                             Util.identityToString(router.ice_getIdentity()));
                    }

                    //
                    // Add the router's server proxy endpoints to this object
                    // adapter.
                    //
                    com.zeroc.IceInternal.EndpointI[] endpoints = _routerInfo.getServerEndpoints();
                    for(com.zeroc.IceInternal.EndpointI endpoint : endpoints)
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
                        com.zeroc.IceInternal.EndpointI e1 = _routerEndpoints.get(i);
                        com.zeroc.IceInternal.EndpointI e2 = _routerEndpoints.get(i + 1);
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
                List<com.zeroc.IceInternal.EndpointI> endpoints =
                    parseEndpoints(properties.getProperty(_name + ".Endpoints"), true);
                for(com.zeroc.IceInternal.EndpointI endp : endpoints)
                {
                    IncomingConnectionFactory factory = new IncomingConnectionFactory(instance, endp, this);
                    _incomingConnectionFactories.add(factory);
                }
                if(endpoints.size() == 0)
                {
                    com.zeroc.IceInternal.TraceLevels tl = _instance.traceLevels();
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
                setLocator(LocatorPrx.uncheckedCast(_instance.proxyFactory().propertyToProxy(_name + ".Locator")));
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

    @Override
    protected synchronized void
    finalize()
        throws Throwable
    {
        try
        {
            if(_state < StateDeactivated)
            {
                _instance.initializationData().logger.warning("object adapter `" + getName() +
                                                              "' has not been deactivated");
            }
            else if(_state != StateDestroyed)
            {
                _instance.initializationData().logger.warning("object adapter `" + getName() +
                                                              "' has not been destroyed");
            }
            else
            {
                com.zeroc.IceUtilInternal.Assert.FinalizerAssert(_threadPool == null);
                // Not cleared, it needs to be immutable.
                //com.zeroc.IceUtilInternal.Assert.FinalizerAssert(_servantManager == null);
                //com.zeroc.IceUtilInternal.Assert.FinalizerAssert(_incomingConnectionFactories.isEmpty());
                com.zeroc.IceUtilInternal.Assert.FinalizerAssert(_directCount == 0);
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
        com.zeroc.IceInternal.EndpointI[] endpoints;

        int sz = _publishedEndpoints.size();
        endpoints = new com.zeroc.IceInternal.EndpointI[sz + _routerEndpoints.size()];
        _publishedEndpoints.toArray(endpoints);

        //
        // Now we also add the endpoints of the router's server proxy, if
        // any. This way, object references created by this object adapter
        // will also point to the router's server proxy endpoints.
        //
        for(int i = 0; i < _routerEndpoints.size(); ++i)
        {
            endpoints[sz + i] = _routerEndpoints.get(i);
        }

        //
        // Create a reference and return a proxy for this reference.
        //
        com.zeroc.IceInternal.Reference ref = _instance.referenceFactory().create(ident, facet, _reference, endpoints);
        return _instance.proxyFactory().referenceToProxy(ref);
    }

    private ObjectPrx
    newIndirectProxy(Identity ident, String facet, String id)
    {
        //
        // Create a reference with the adapter id and return a proxy
        // for the reference.
        //
        com.zeroc.IceInternal.Reference ref = _instance.referenceFactory().create(ident, facet, _reference, id);
        return _instance.proxyFactory().referenceToProxy(ref);
    }

    private void
    checkForDeactivation()
    {
        if(_state >= StateDeactivating)
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
            throw new IllegalIdentityException(ident);
        }

        if(ident.category == null)
        {
            ident.category = "";
        }
    }

    private static void
    checkServant(com.zeroc.Ice.Object servant)
    {
        if(servant == null)
        {
            throw new IllegalServantException("cannot add null servant to Object Adapter");
        }
    }

    private List<com.zeroc.IceInternal.EndpointI>
    parseEndpoints(String endpts, boolean oaEndpoints)
    {
        int beg;
        int end = 0;

        final String delim = " \t\n\r";

        List<com.zeroc.IceInternal.EndpointI> endpoints = new ArrayList<>();
        while(end < endpts.length())
        {
            beg = com.zeroc.IceUtilInternal.StringUtil.findFirstNotOf(endpts, delim, end);
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
            com.zeroc.IceInternal.EndpointI endp = _instance.endpointFactoryManager().create(s, oaEndpoints);
            if(endp == null)
            {
                EndpointParseException e = new EndpointParseException();
                e.str = "invalid object adapter endpoint `" + s + "'";
                throw e;
            }
            endpoints.add(endp);

            ++end;
        }

        return endpoints;
    }

    private List<com.zeroc.IceInternal.EndpointI>
    parsePublishedEndpoints()
    {
        //
        // Parse published endpoints. If set, these are used in proxies
        // instead of the connection factory Endpoints.
        //
        String endpts = _instance.initializationData().properties.getProperty(_name + ".PublishedEndpoints");
        List<com.zeroc.IceInternal.EndpointI> endpoints = parseEndpoints(endpts, false);
        if(endpoints.isEmpty())
        {
            //
            // If the PublishedEndpoints property isn't set, we compute the published enpdoints
            // from the OA endpoints, expanding any endpoints that may be listening on INADDR_ANY
            // to include actual addresses in the published endpoints.
            //
            for(IncomingConnectionFactory factory : _incomingConnectionFactories)
            {
                endpoints.addAll(factory.endpoint().expand());
            }
        }

        if(_instance.traceLevels().network >= 1 && !endpoints.isEmpty())
        {
            StringBuffer s = new StringBuffer("published endpoints for object adapter `");
            s.append(_name);
            s.append("':\n");
            boolean first = true;
            for(com.zeroc.IceInternal.EndpointI endpoint : endpoints)
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
    updateLocatorRegistry(com.zeroc.IceInternal.LocatorInfo locatorInfo, ObjectPrx proxy)
    {
        if(_id.length() == 0 || locatorInfo == null)
        {
            return; // Nothing to update.
        }

        //
        // Call on the locator registry outside the synchronization to
        // blocking other threads that need to lock this OA.
        //
        LocatorRegistryPrx locatorRegistry = locatorInfo.getLocatorRegistry();
        if(locatorRegistry == null)
        {
            return;
        }

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
        catch(ObjectAdapterDeactivatedException e)
        {
            // Expected if collocated call and OA is deactivated, ignore.
        }
        catch(CommunicatorDestroyedException e)
        {
            // Ignore
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
                Endpoint[] endpoints = proxy.ice_getEndpoints();
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

    static private String[] _suffixes =
    {
        "ACM",
        "ACM.Timeout",
        "ACM.Heartbeat",
        "ACM.Close",
        "AdapterId",
        "Endpoints",
        "Locator",
        "Locator.EncodingVersion",
        "Locator.EndpointSelection",
        "Locator.ConnectionCached",
        "Locator.PreferSecure",
        "Locator.CollocationOptimized",
        "Locator.Router",
        "MessageSizeMax",
        "PublishedEndpoints",
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
        "Router.Locator.InvocationTimeout",
        "Router.LocatorCacheTimeout",
        "Router.InvocationTimeout",
        "ProxyOptions",
        "ThreadPool.Size",
        "ThreadPool.SizeMax",
        "ThreadPool.SizeWarn",
        "ThreadPool.StackSize",
        "ThreadPool.Serialize"
    };

    boolean
    filterProperties(List<String> unknownProps)
    {
        //
        // Do not create unknown properties list if Ice prefix, ie Ice, Glacier2, etc
        //
        boolean addUnknown = true;
        String prefix = _name + ".";
        for(int i = 0; com.zeroc.IceInternal.PropertyNames.clPropNames[i] != null; ++i)
        {
            if(prefix.startsWith(com.zeroc.IceInternal.PropertyNames.clPropNames[i] + "."))
            {
                addUnknown = false;
                break;
            }
        }

        boolean noProps = true;
        Map<String, String> props = _instance.initializationData().properties.getPropertiesForPrefix(prefix);
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

    private static final int StateUninitialized = 0; // Just constructed.
    private static final int StateHeld = 1;
    private static final int StateActivating = 2;
    private static final int StateActive = 3;
    private static final int StateDeactivating = 4;
    private static final int StateDeactivated = 5;
    private static final int StateDestroying = 6;
    private static final int StateDestroyed  = 7;

    private int _state = StateUninitialized;
    private com.zeroc.IceInternal.Instance _instance;
    private Communicator _communicator;
    private com.zeroc.IceInternal.ObjectAdapterFactory _objectAdapterFactory;
    private com.zeroc.IceInternal.ThreadPool _threadPool;
    private com.zeroc.IceInternal.ACMConfig _acm;
    private com.zeroc.IceInternal.ServantManager _servantManager;
    final private String _name;
    final private String _id;
    final private String _replicaGroupId;
    private com.zeroc.IceInternal.Reference _reference;
    private List<IncomingConnectionFactory> _incomingConnectionFactories = new ArrayList<>();
    private List<com.zeroc.IceInternal.EndpointI> _routerEndpoints = new ArrayList<>();
    private com.zeroc.IceInternal.RouterInfo _routerInfo = null;
    private List<com.zeroc.IceInternal.EndpointI> _publishedEndpoints = new ArrayList<>();
    private com.zeroc.IceInternal.LocatorInfo _locatorInfo;
    private int _directCount; // The number of direct proxies dispatching on this object adapter.
    private boolean _noConfig;
    private final int _messageSizeMax;
}
