// **********************************************************************
//
// Copyright (c) 2002
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

public final class ObjectAdapterI extends LocalObjectImpl implements ObjectAdapter
{
    public String
    getName()
    {
	//
        // No mutex lock necessary, _name is immutable.
	//
        return _name;
    }

    public synchronized Communicator
    getCommunicator()
    {
	checkForDeactivation();
	
	return _communicator;
    }

    public synchronized void
    activate()
    {
	checkForDeactivation();
	
	if(!_printAdapterReadyDone)
	{
	    if(_locatorInfo != null && _id.length() > 0)
	    {
		Identity ident = new Identity();
		ident.category = "";
		ident.name = "dummy";

		//
		// TODO: This might throw if we can't connect to the
		// locator. Shall we raise a special exception for the
		// activate operation instead of a non obvious network
		// exception?
		//
		try
		{
		    _locatorInfo.getLocatorRegistry().setAdapterDirectProxy(_id, newDirectProxy(ident));
		}
		catch(Ice.AdapterNotFoundException ex)
		{
		    NotRegisteredException ex1 = new NotRegisteredException();
		    ex1.id = _id;
		    ex1.kindOfObject = "object adapter";
		    throw ex1;
		}
		catch(Ice.AdapterAlreadyActiveException ex)
		{
		    ObjectAdapterIdInUseException ex1 = new ObjectAdapterIdInUseException();
		    ex1.id = _id;
		    throw ex1;
		}
	    }
	}

        final int sz = _incomingConnectionFactories.size();
        for(int i = 0; i < sz; ++i)
        {
            IceInternal.IncomingConnectionFactory factory =
                (IceInternal.IncomingConnectionFactory)_incomingConnectionFactories.get(i);
            factory.activate();
        }

	if(!_printAdapterReadyDone)
	{
	    if(_instance.properties().getPropertyAsInt("Ice.PrintAdapterReady") > 0)
	    {
		System.out.println(_name + " ready");
	    }
		
	    _printAdapterReadyDone = true;
	}
    }

    public synchronized void
    hold()
    {
	checkForDeactivation();
	
        final int sz = _incomingConnectionFactories.size();
        for(int i = 0; i < sz; ++i)
        {
            IceInternal.IncomingConnectionFactory factory =
                (IceInternal.IncomingConnectionFactory)_incomingConnectionFactories.get(i);
            factory.hold();
        }
    }

    public synchronized void
    waitForHold()
    {
	checkForDeactivation();

	final int sz = _incomingConnectionFactories.size();
	for(int i = 0; i < sz; ++i)
	{
	    IceInternal.IncomingConnectionFactory factory =
		(IceInternal.IncomingConnectionFactory)_incomingConnectionFactories.get(i);
	    factory.waitUntilHolding();
	}
    } 

    public synchronized void
    deactivate()
    {
	//
	// Ignore deactivation requests if the object adapter has
	// already been deactivated.
	//
	if(_deactivated)
	{
	    return;
	}
	
	final int sz = _incomingConnectionFactories.size();
	for(int i = 0; i < sz; ++i)
	{
	    IceInternal.IncomingConnectionFactory factory =
		(IceInternal.IncomingConnectionFactory)_incomingConnectionFactories.get(i);
	    factory.destroy();
	}
	
	_instance.outgoingConnectionFactory().removeAdapter(this);
	
	_deactivated = true;
	
	notifyAll();
    }

    public void
    waitForDeactivate()
    {
	synchronized(this)
	{
	    //
	    // First we wait for deactivation of the adapter itself, and
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
	    
	    //
	    // If some other thread is currently deactivating, we wait
	    // until this thread is finished.
	    //
	    while(_waitForDeactivate)
	    {
		try
		{
		    wait();
		}
		catch(InterruptedException ex)
		{
		}
	    }
	    _waitForDeactivate = true;
	}
	
	
	//
	// Now we wait for until all incoming connection factories are
	// finished.
	//
	final int sz = _incomingConnectionFactories.size();
	for(int i = 0; i < sz; ++i)
	{
	    IceInternal.IncomingConnectionFactory factory =
		(IceInternal.IncomingConnectionFactory)_incomingConnectionFactories.get(i);
	    factory.waitUntilFinished();
	}
	
	//
	// Now it's also time to clean up our servants and servant
	// locators.
	//
	if(_servantManager != null)
	{
	    _servantManager.destroy();
	}
	
	//
	// Destroy the thread pool.
	//
	if(_threadPool != null)
	{
	    _threadPool.destroy();
	    _threadPool.joinWithAllThreads();
	}

	synchronized(this)
	{
	    //
	    // Signal that waiting is complete.
	    //
	    _waitForDeactivate = false;
	    notifyAll();

	    //
	    // We're done, now we can throw away all incoming connection
	    // factories.
	    //
	    _incomingConnectionFactories.clear();
	    
	    //
	    // Remove object references (some of them cyclic).
	    //
	    _instance = null;
	    _threadPool = null;
	    _servantManager = null;
	    _communicator = null;
	}
    }

    public synchronized ObjectPrx
    add(Ice.Object servant, Identity ident)
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

	_servantManager.addServant(servant, id);

        return newProxy(id);
    }

    public ObjectPrx
    addWithUUID(Ice.Object servant)
    {
        Identity ident = new Identity();
        ident.category = "";
        ident.name = Util.generateUUID();

        return add(servant, ident);
    }

    public synchronized void
    remove(Identity ident)
    {
	checkForDeactivation();
        checkIdentity(ident);

	_servantManager.removeServant(ident);
    }

    public synchronized void
    addServantLocator(ServantLocator locator, String prefix)
    {
	checkForDeactivation();

	_servantManager.addServantLocator(locator, prefix);
    }

    public synchronized void
    removeServantLocator(String prefix)
    {
	checkForDeactivation();

	_servantManager.removeServantLocator(prefix);
    }

    public synchronized ServantLocator
    findServantLocator(String prefix)
    {
	checkForDeactivation();

	return _servantManager.findServantLocator(prefix);
    }

    public synchronized Ice.Object
    identityToServant(Identity ident)
    {
	checkForDeactivation();
	checkIdentity(ident);

	return _servantManager.findServant(ident);
    }

    public synchronized Ice.Object
    proxyToServant(ObjectPrx proxy)
    {
	checkForDeactivation();

        IceInternal.Reference ref = ((ObjectPrxHelper)proxy).__reference();
        return identityToServant(ref.identity);
    }

    public synchronized ObjectPrx
    createProxy(Identity ident)
    {
	checkForDeactivation();
        checkIdentity(ident);

        return newProxy(ident);
    }

    public synchronized ObjectPrx
    createDirectProxy(Identity ident)
    {
	checkForDeactivation();
        checkIdentity(ident);

        return newDirectProxy(ident);
    }

    public synchronized ObjectPrx
    createReverseProxy(Identity ident)
    {
	checkForDeactivation();
        checkIdentity(ident);

        //
        // Create a reference and return a reverse proxy for this
        // reference.
        //
        IceInternal.Endpoint[] endpoints = new IceInternal.Endpoint[0];
        IceInternal.Reference ref =
            _instance.referenceFactory().create(ident, new String[0], IceInternal.Reference.ModeTwoway,
						false, "", endpoints, null, null, this, true);

        return _instance.proxyFactory().referenceToProxy(ref);
    }

    public synchronized void
    addRouter(RouterPrx router)
    {
	checkForDeactivation();

        IceInternal.RouterInfo routerInfo = _instance.routerManager().get(router);
        if(routerInfo != null)
        {
            //
            // Add the router's server proxy endpoints to this object
            // adapter.
            //
            ObjectPrxHelper proxy = (ObjectPrxHelper)routerInfo.getServerProxy();
            IceInternal.Endpoint[] endpoints = proxy.__reference().endpoints;
            for(int i = 0; i < endpoints.length; ++i)
            {
                _routerEndpoints.add(endpoints[i]);
            }
            java.util.Collections.sort(_routerEndpoints); // Must be sorted.
            for(int i = 0; i < _routerEndpoints.size() - 1; ++i)
            {
                java.lang.Object o1 = _routerEndpoints.get(i);
                java.lang.Object o2 = _routerEndpoints.get(i + 1);
                if(o1.equals(o2))
                {
                    _routerEndpoints.remove(i);
                }
            }

            //
            // Associate this object adapter with the router. This way,
            // new outgoing connections to the router's client proxy will
            // use this object adapter for callbacks.
            //
            routerInfo.setAdapter(this);

            //
            // Also modify all existing outgoing connections to the
            // router's client proxy to use this object adapter for
            // callbacks.
            //      
            _instance.outgoingConnectionFactory().setRouter(routerInfo.getRouter());
        }
    }

    public synchronized void
    setLocator(LocatorPrx locator)
    {
	checkForDeactivation();

	_locatorInfo = _instance.locatorManager().get(locator);
    }

    public synchronized boolean
    isLocal(ObjectPrx proxy)
    {
	checkForDeactivation();

        IceInternal.Reference ref = ((ObjectPrxHelper)proxy).__reference();
        final IceInternal.Endpoint[] endpoints = ref.endpoints;

	if(!ref.adapterId.equals(""))
	{
	    //
	    // Proxy is local if the reference adapter id matches this
	    // adapter name.
	    //
	    return ref.adapterId.equals(_id);
	}

        //
        // Proxies which have at least one endpoint in common with the
        // endpoints used by this object adapter's incoming connection
        // factories are considered local.
        //
        for(int i = 0; i < endpoints.length; ++i)
        {
            final int sz = _incomingConnectionFactories.size();
            for(int j = 0; j < sz; j++)
            {
                IceInternal.IncomingConnectionFactory factory =
                    (IceInternal.IncomingConnectionFactory)_incomingConnectionFactories.get(j);
                if(factory.equivalent(endpoints[i]))
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
	for(int i = 0; i < endpoints.length; ++i)
	{
	    if(java.util.Collections.binarySearch(_routerEndpoints, endpoints[i]) >= 0) // _routerEndpoints is sorted.
	    {
		return true;
	    }
	}

        return false;
    }

    public synchronized IceInternal.Connection[]
    getIncomingConnections()
    {
	checkForDeactivation();

        java.util.ArrayList connections = new java.util.ArrayList();
        final int sz = _incomingConnectionFactories.size();
        for(int i = 0; i < sz; ++i)
        {
            IceInternal.IncomingConnectionFactory factory =
                (IceInternal.IncomingConnectionFactory)_incomingConnectionFactories.get(i);
            IceInternal.Connection[] cons = factory.connections();
            for(int j = 0; j < cons.length; j++)
            {
                connections.add(cons[j]);
            }
        }
        IceInternal.Connection[] arr = new IceInternal.Connection[connections.size()];
        connections.toArray(arr);
        return arr;
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
	
	assert(_instance != null); // Must not be called after waitForDeactivate().
	
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
	// waitForDeactivate().
	
	// Not check for deactivation here!
	
	assert(_instance != null); // Must not be called after waitForDeactivate().

	if(_threadPool != null)
	{
	    return _threadPool;
	}
	else
	{
	    return _instance.serverThreadPool();
	}
    }

    public IceInternal.ServantManager
    getServantManager()
    {
	// No mutex lock necessary, _servantManager is immutable after
	// creation until it is removed in waitForDeactivate().
	
	// Not check for deactivation here!
	
	assert(_servantManager != null); // Must not be called after waitForDeactivate().
	return _servantManager;
    }

    //
    // Only for use by IceInternal.ObjectAdapterFactory
    //
    public
    ObjectAdapterI(IceInternal.Instance instance, Communicator communicator, String name)
    {
	_deactivated = false;
        _instance = instance;
	_communicator = communicator;
	_servantManager = new IceInternal.ServantManager(instance, name);
	_printAdapterReadyDone = false;
        _name = name;
	_id = instance.properties().getProperty(name + ".AdapterId");
        _logger = instance.logger();
	_directCount = 0;
	_waitForDeactivate = false;
	
        try
        {
	    String endpts = _instance.properties().getProperty(name + ".Endpoints").toLowerCase();
	    
	    int beg = 0;
	    int end;

            while(true)
            {
                end = endpts.indexOf(':', beg);
                if(end == -1)
                {
                    end = endpts.length();
                }

                if(end == beg)
                {
                    break;
                }

                String s = endpts.substring(beg, end);

                //
                // Don't store the endpoint in the adapter. The Connection
                // might change it, for example, to fill in the real port
                // number if a zero port number is given.
                //
                IceInternal.Endpoint endp = instance.endpointFactoryManager().create(s);
                _incomingConnectionFactories.add(new IceInternal.IncomingConnectionFactory(instance, endp, this));

                if(end == s.length())
                {
                    break;
                }

                beg = end + 1;
            }

	    String router = _instance.properties().getProperty(name + ".Router");
	    if(router.length() > 0)
	    {
		addRouter(RouterPrxHelper.uncheckedCast(_instance.proxyFactory().stringToProxy(router)));
	    }

	    String locator = _instance.properties().getProperty(name + ".Locator");
	    if(locator.length() > 0)
	    {
		setLocator(LocatorPrxHelper.uncheckedCast(_instance.proxyFactory().stringToProxy(locator)));
	    }
	    else
	    {
		setLocator(_instance.referenceFactory().getDefaultLocator());
	    }

	    int threadNum = _instance.properties().getPropertyAsInt(_name + ".ThreadPool.Size");
	    if(threadNum > 0)
	    {
		_threadPool = new IceInternal.ThreadPool(_instance, threadNum, 0, _name + ".ThreadPool");
	    }
        }
        catch(LocalException ex)
        {
	    deactivate();
	    waitForDeactivate();
            throw ex;
        }
    }

    protected void
    finalize()
        throws Throwable
    {
        if(!_deactivated)
        {
            _instance.logger().warning("object adapter `" + _name + "' has not been deactivated");
        }
        else if(_instance != null)
        {
            _instance.logger().warning("object adapter `" + _name + "' deactivation had not been waited for");
        }
	else
	{
	    assert(_threadPool == null);
	    assert(_servantManager == null);
	    assert(_communicator == null);
	    assert(_incomingConnectionFactories.isEmpty());
	    assert(_directCount == 0);
	    assert(!_waitForDeactivate);
	}

        super.finalize();
    }

    private ObjectPrx
    newProxy(Identity ident)
    {
	if(_id.length() == 0)
	{
	    return newDirectProxy(ident);
	}
	else
	{	    
	    //
	    // Create a reference with the adapter id and return a
	    // proxy for the reference.
	    //
	    IceInternal.Endpoint[] endpoints = new IceInternal.Endpoint[0];
	    IceInternal.Reference reference =
		_instance.referenceFactory().create(ident, new String[0], IceInternal.Reference.ModeTwoway,
						    false, _id, endpoints, null, null, null, true);
	    return _instance.proxyFactory().referenceToProxy(reference);
	}
    }

    private ObjectPrx
    newDirectProxy(Identity ident)
    {
        IceInternal.Endpoint[] endpoints =
            new IceInternal.Endpoint[_incomingConnectionFactories.size() + _routerEndpoints.size()];

        //
        // First we add all endpoints from all incoming connection
        // factories.
        //
        int sz = _incomingConnectionFactories.size();
        for(int i = 0; i < sz; ++i)
        {
            IceInternal.IncomingConnectionFactory factory =
                (IceInternal.IncomingConnectionFactory)_incomingConnectionFactories.get(i);
            endpoints[i] = factory.endpoint();
        }

        //
        // Now we also add the endpoints of the router's server proxy, if
        // any. This way, object references created by this object adapter
        // will also point to the router's server proxy endpoints.
        //
        sz = _routerEndpoints.size();
        for(int i = 0; i < sz; ++i)
        {
            endpoints[_incomingConnectionFactories.size() + i] = (IceInternal.Endpoint)_routerEndpoints.get(i);
        }

        //
        // Create a reference and return a proxy for this reference.
        //
        IceInternal.Reference reference =
	    _instance.referenceFactory().create(ident, new String[0], IceInternal.Reference.ModeTwoway,
						false, "", endpoints, null, null, null, true);
        return _instance.proxyFactory().referenceToProxy(reference);
    }

    private void
    checkForDeactivation()
    {
	if(_deactivated)
	{
            ObjectAdapterDeactivatedException ex = new ObjectAdapterDeactivatedException();
	    ex.name = _name;
	    throw ex;
	}
    }

    private static void
    checkIdentity(Identity ident)
    {
        if(ident.name == null || ident.name.length() == 0)
        {
            IllegalIdentityException e = new IllegalIdentityException();
            try
            {
                e.id = (Identity)ident.clone();
            }
            catch(CloneNotSupportedException ex)
            {
                assert(false);
            }
            throw e;
        }

        if(ident.category == null)
        {
            ident.category = "";
        }
    }

    private boolean _deactivated;
    private IceInternal.Instance _instance;
    private Communicator _communicator;
    private IceInternal.ThreadPool _threadPool;
    private IceInternal.ServantManager _servantManager;
    private boolean _printAdapterReadyDone;
    final private String _name;
    final private String _id;
    private Logger _logger;
    private java.util.ArrayList _incomingConnectionFactories = new java.util.ArrayList();
    private java.util.ArrayList _routerEndpoints = new java.util.ArrayList();
    private IceInternal.LocatorInfo _locatorInfo;
    private int _directCount;
    private boolean _waitForDeactivate;
}
