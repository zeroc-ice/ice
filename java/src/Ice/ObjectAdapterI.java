// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package Ice;

public class ObjectAdapterI implements ObjectAdapter
{
    public String
    getName()
    {
        return _name; // _name is immutable
    }

    public synchronized Communicator
    getCommunicator()
    {
	if(_instance == null)
	{
            throw new ObjectAdapterDeactivatedException();
	}
	
	return _communicator;
    }

    public synchronized void
    activate()
    {
        if(_instance == null)
        {
            throw new ObjectAdapterDeactivatedException();
        }

	if(!_printAdapterReadyDone)
	{
	    if(_locatorInfo != null)
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
		    _locatorInfo.getLocatorRegistry().setAdapterDirectProxy(_name, newDirectProxy(ident));
		}
		catch(Ice.AdapterNotRegistered ex)
		{
		    throw new ObjectAdapterNotRegisteredException();
		}
		catch(Ice.AdapterAlreadyActive ex)
		{
		    throw new ObjectAdapterActiveException();
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
        if(_instance == null)
        {
            throw new ObjectAdapterDeactivatedException();
        }

        final int sz = _incomingConnectionFactories.size();
        for(int i = 0; i < sz; ++i)
        {
            IceInternal.IncomingConnectionFactory factory =
                (IceInternal.IncomingConnectionFactory)_incomingConnectionFactories.get(i);
            factory.hold();
        }
    }

    public synchronized void
    deactivate()
    {
        if(_instance == null)
        {
            //
            // Ignore deactivation requests if the Object Adapter has
            // already been deactivated.
            //
            return;
        }

        final int sz = _incomingConnectionFactories.size();
        for(int i = 0; i < sz; ++i)
        {
            IceInternal.IncomingConnectionFactory factory =
                (IceInternal.IncomingConnectionFactory)_incomingConnectionFactories.get(i);
            factory.destroy();
        }

	//
	// Don't do a _incomingConnectionFactories.clear()!
	// _incomingConnectionFactories is immutable. Even if all
	// elements are destroyed, the elements are still needed by
	// waitForDeactivate().
	//

	_instance.outgoingConnectionFactory().removeAdapter(this);

	_activeServantMap.clear();

        java.util.Iterator p = _locatorMap.values().iterator();
        while(p.hasNext())
        {
            ServantLocator locator = (ServantLocator)p.next();
            locator.deactivate();
        }
        _locatorMap.clear();

        _instance = null;
	_communicator = null;
    }

    public void
    waitForDeactivate()
    {
	//
	// _incommingConnectionFactories is immutable, thus no mutex
	// lock is necessary. (A mutex lock wouldn't work here anyway,
	// as there would be a deadlock with upcalls.)
	//
        final int sz = _incomingConnectionFactories.size();
        for(int i = 0; i < sz; ++i)
        {
            IceInternal.IncomingConnectionFactory factory =
                (IceInternal.IncomingConnectionFactory)_incomingConnectionFactories.get(i);
            factory.waitUntilFinished();
        }
    }

    public synchronized ObjectPrx
    add(Ice.Object servant, Identity ident)
    {
        if(_instance == null)
        {
            throw new ObjectAdapterDeactivatedException();
        }

        //
        // Create a copy of the Identity argument, in case the caller
        // reuses it
        //
        Identity id = new Identity();
        id.category = ident.category;
        id.name = ident.name;
        _activeServantMap.put(id, servant);

        return newProxy(id);
    }

    public synchronized ObjectPrx
    addWithUUID(Ice.Object servant)
    {
        if(_instance == null)
        {
            throw new ObjectAdapterDeactivatedException();
        }

        long now = System.currentTimeMillis();
        Identity ident = new Identity();
        ident.category = "";
        ident.name = Util.generateUUID();

        _activeServantMap.put(ident, servant);

        return newProxy(ident);
    }

    public synchronized void
    remove(Identity ident)
    {
        if(_instance == null)
        {
            throw new ObjectAdapterDeactivatedException();
        }

        _activeServantMap.remove(ident);
    }

    public synchronized void
    addServantLocator(ServantLocator locator, String prefix)
    {
        if(_instance == null)
        {
            throw new ObjectAdapterDeactivatedException();
        }

        _locatorMap.put(prefix, locator);
    }

    public synchronized void
    removeServantLocator(String prefix)
    {
        if(_instance == null)
        {
            throw new ObjectAdapterDeactivatedException();
        }

        ServantLocator locator = (ServantLocator)_locatorMap.remove(prefix);
        if(locator != null)
        {
            locator.deactivate();
        }
    }

    public synchronized ServantLocator
    findServantLocator(String prefix)
    {
        if(_instance == null)
        {
            throw new ObjectAdapterDeactivatedException();
        }

        return (ServantLocator)_locatorMap.get(prefix);
    }

    public synchronized Ice.Object
    identityToServant(Identity ident)
    {
        return (Ice.Object)_activeServantMap.get(ident);
    }

    public Ice.Object
    proxyToServant(ObjectPrx proxy)
    {
        IceInternal.Reference ref = ((ObjectPrxHelper)proxy).__reference();
        return identityToServant(ref.identity);
    }

    public synchronized ObjectPrx
    createProxy(Identity ident)
    {
        if(_instance == null)
        {
            throw new ObjectAdapterDeactivatedException();
        }

        return newProxy(ident);
    }

    public synchronized ObjectPrx
    createDirectProxy(Identity ident)
    {
        if(_instance == null)
        {
            throw new ObjectAdapterDeactivatedException();
        }

        return newDirectProxy(ident);
    }

    public synchronized ObjectPrx
    createReverseProxy(Identity ident)
    {
        if(_instance == null)
        {
            throw new ObjectAdapterDeactivatedException();
        }

        //
        // Create a reference and return a reverse proxy for this reference.
        //
        IceInternal.Endpoint[] endpoints = new IceInternal.Endpoint[0];
        IceInternal.Reference ref =
            _instance.referenceFactory().create(ident, new String[0], IceInternal.Reference.ModeTwoway, false, false,
						"", endpoints, null, null, this);

        return _instance.proxyFactory().referenceToProxy(ref);
    }

    public synchronized void
    addRouter(RouterPrx router)
    {
        if(_instance == null)
        {
            throw new ObjectAdapterDeactivatedException();
        }

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

	    //
	    // Creates proxies with endpoints instead of the adapter name.
	    //
	    _useEndpointsInProxy = true;
        }
    }

    public synchronized void
    setLocator(LocatorPrx locator)
    {
	if(_instance == null)
	{
	    throw new ObjectAdapterDeactivatedException();
	}

	_locatorInfo = _instance.locatorManager().get(locator);
	if(_locatorInfo != null)
	{	
	    //
	    // If a locator is set, we create proxies with adapter names in
	    // the reference instead of endpoints. If it's not set, we create
	    // proxies with endpoints if there's at least one incoming
	    // connection factory or router endpoints.
	    //
	    _useEndpointsInProxy = false;
	}
	else
	{
	    _useEndpointsInProxy = _incomingConnectionFactories.size() > 0 || _routerEndpoints.size() > 0;
	}
    }

    public IceInternal.Connection[]
    getIncomingConnections()
    {
	//
	// _incommingConnectionFactories is immutable, thus no mutex lock
	// is necessary.
	//
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

    //
    // Only for use by IceInternal.ObjectAdapterFactory
    //
    public
    ObjectAdapterI(IceInternal.Instance instance, Communicator communicator, String name, String endpts)
    {
        _instance = instance;
	_communicator = communicator;
	_printAdapterReadyDone = false;
        _name = name;
	
	String s = endpts.toLowerCase();

        int beg = 0;
        int end;

        try
        {
            while(true)
            {
                end = s.indexOf(':', beg);
                if(end == -1)
                {
                    end = s.length();
                }

                if(end == beg)
                {
                    break;
                }

                String es = s.substring(beg, end);

                //
                // Don't store the endpoint in the adapter. The Connection
                // might change it, for example, to fill in the real port
                // number if a zero port number is given.
                //
                IceInternal.Endpoint endp = instance.endpointFactoryManager().create(es);
                _incomingConnectionFactories.add(new IceInternal.IncomingConnectionFactory(instance, endp, this));

                if(end == s.length())
                {
                    break;
                }

                beg = end + 1;
            }
        }
        catch(LocalException ex)
        {
	    deactivate();
            throw ex;
        }

	//
	// Create proxies with the adapter endpoints only if there's
	// incoming connection factories. If there's no incoming
	// connection factories we will create proxies with the adapter
	// name in the reference (to allow collocation to work).
	//
	_useEndpointsInProxy = _incomingConnectionFactories.size() > 0;

//
// Object Adapters without incoming connection factories are
// permissible, as such Object Adapters can still be used with a
// router. (See addRouter.)
//
/*
        if(_incomingConnectionFactories.isEmpty())
        {
            throw new EndpointParseException();
        }
*/
    }

    protected void
    finalize()
        throws Throwable
    {
        if(_instance != null)
        {
            _instance.logger().warning("object adapter has not been deactivated");
        }

        super.finalize();
    }

    private ObjectPrx
    newProxy(Identity ident)
    {
	if(_useEndpointsInProxy)
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
	    IceInternal.Reference reference = _instance.referenceFactory().create(ident, new String[0],
										  IceInternal.Reference.ModeTwoway,
										  false, false, _name, 
										  endpoints, null, null, null);
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
        IceInternal.Reference reference = _instance.referenceFactory().create(ident, new String[0],
									      IceInternal.Reference.ModeTwoway,
									      false, false, "",
									      endpoints, null, null, null);
        return _instance.proxyFactory().referenceToProxy(reference);
    }

    public boolean
    isLocal(ObjectPrx proxy)
    {
        IceInternal.Reference ref = ((ObjectPrxHelper)proxy).__reference();
        final IceInternal.Endpoint[] endpoints = ref.endpoints;

	if(!ref.adapterId.equals(""))
	{
	    //
	    // Proxy is local if the reference adapter id matches this
	    // adapter name.
	    //
	    return ref.adapterId.equals(_name);
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
	// Must be synchronized, because _routerEndpoints is not
	// immutable, and because this operation is called
	// unsynchronized from
	// ObjectAdapterFactory::findObjectAdapter().
	//
	synchronized(this)
	{
	    //
	    // Proxies which have at least one endpoint in common with the
	    // router's server proxy endpoints (if any), are also considered
	    // local.
	    //
	    for(int i = 0; i < endpoints.length; ++i)
	    {
		// _routerEndpoints is sorted.
		if(java.util.Collections.binarySearch(_routerEndpoints, endpoints[i]) >= 0)
		{
		    return true;
		}
	    }
	    
	}

        return false;
    }

    private IceInternal.Instance _instance;
    private Communicator _communicator;
    private boolean _printAdapterReadyDone;
    private String _name;
    private boolean _useEndpointsInProxy;
    private java.util.HashMap _activeServantMap = new java.util.HashMap();
    private java.util.HashMap _locatorMap = new java.util.HashMap();
    private java.util.ArrayList _incomingConnectionFactories = new java.util.ArrayList();
    private java.util.ArrayList _routerEndpoints = new java.util.ArrayList();
    private IceInternal.LocatorInfo _locatorInfo;
}
