// **********************************************************************
//
// Copyright (c) 2001
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

public class ObjectAdapterI extends LocalObjectImpl implements ObjectAdapter
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
            ObjectAdapterDeactivatedException e = new ObjectAdapterDeactivatedException();
	    e.name = _name;
	    throw e;
	}
	
	return _communicator;
    }

    public synchronized void
    activate()
    {
        if(_instance == null)
        {
            ObjectAdapterDeactivatedException e = new ObjectAdapterDeactivatedException();
	    e.name = _name;
	    throw e;
        }

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
		catch(Ice.AdapterNotRegistered ex)
		{
		    ObjectAdapterNotRegisteredException ex1 = new ObjectAdapterNotRegisteredException();
		    ex1.name = _name;
		    throw ex1;
		}
		catch(Ice.AdapterAlreadyActive ex)
		{
		    ObjectAdapterNameInUseException ex1 = new ObjectAdapterNameInUseException();
		    ex1.name = _name;
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
        if(_instance == null)
        {
            ObjectAdapterDeactivatedException e = new ObjectAdapterDeactivatedException();
	    e.name = _name;
	    throw e;
        }

        final int sz = _incomingConnectionFactories.size();
        for(int i = 0; i < sz; ++i)
        {
            IceInternal.IncomingConnectionFactory factory =
                (IceInternal.IncomingConnectionFactory)_incomingConnectionFactories.get(i);
            factory.hold();
        }
    }

    public void
    deactivate()
    {
	synchronized(this)
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
	    _incomingConnectionFactories.clear();
	    
	    _instance.outgoingConnectionFactory().removeAdapter(this);
	    
	    _instance = null;
	    _communicator = null;
	}

	decUsageCount();
    }

    public synchronized void
    waitForDeactivate()
    {
	assert(_usageCount >= 0);

	while(_usageCount > 0)
	{
	    try
	    {
		wait();
	    }
	    catch(java.lang.InterruptedException ex)
	    {
	    }
	}

	assert(_usageCount == 0);
    }

    public synchronized ObjectPrx
    add(Ice.Object servant, Identity ident)
    {
        if(_instance == null)
        {
            ObjectAdapterDeactivatedException e = new ObjectAdapterDeactivatedException();
	    e.name = _name;
	    throw e;
        }

        checkIdentity(ident);

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
            ObjectAdapterDeactivatedException e = new ObjectAdapterDeactivatedException();
	    e.name = _name;
	    throw e;
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
            ObjectAdapterDeactivatedException e = new ObjectAdapterDeactivatedException();
	    e.name = _name;
	    throw e;
        }

        checkIdentity(ident);

        _activeServantMap.remove(ident);
    }

    public synchronized void
    addServantLocator(ServantLocator locator, String prefix)
    {
        if(_instance == null)
        {
            ObjectAdapterDeactivatedException e = new ObjectAdapterDeactivatedException();
	    e.name = _name;
	    throw e;
        }

        _locatorMap.put(prefix, locator);
    }

    public synchronized void
    removeServantLocator(String prefix)
    {
        if(_instance == null)
        {
            ObjectAdapterDeactivatedException e = new ObjectAdapterDeactivatedException();
	    e.name = _name;
	    throw e;
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
	//
	// Don't check whether deactivation has been initiated. This
	// operation might be called (e.g., from Incoming or Direct)
	// after deactivation has been initiated, but before
	// deactivation has been completed.
	//
	/*
        if(_instance == null)
        {
            ObjectAdapterDeactivatedException e = new ObjectAdapterDeactivatedException();
	    e.name = _name;
	    throw e;
        }
	*/

        return (ServantLocator)_locatorMap.get(prefix);
    }

    public synchronized Ice.Object
    identityToServant(Identity ident)
    {
	//
	// Don't check whether deactivation has been initiated. This
	// operation might be called (e.g., from Incoming or Direct)
	// after deactivation has been initiated, but before
	// deactivation has been completed.
	//
	/*
        if(_instance == null)
        {
            ObjectAdapterDeactivatedException e = new ObjectAdapterDeactivatedException();
	    e.name = _name;
	    throw e;
        }
	*/

	//
	// Don't call checkIdentity. We simply want null to be
	// returned (e.g., for Direct, Incoming) in case the identity
	// is incorrect and therefore no servant can be found.
	//
	/*
        checkIdentity(ident);
	*/

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
            ObjectAdapterDeactivatedException e = new ObjectAdapterDeactivatedException();
	    e.name = _name;
	    throw e;
        }

        checkIdentity(ident);

        return newProxy(ident);
    }

    public synchronized ObjectPrx
    createDirectProxy(Identity ident)
    {
        if(_instance == null)
        {
            ObjectAdapterDeactivatedException e = new ObjectAdapterDeactivatedException();
	    e.name = _name;
	    throw e;
        }

        checkIdentity(ident);

        return newDirectProxy(ident);
    }

    public synchronized ObjectPrx
    createReverseProxy(Identity ident)
    {
        if(_instance == null)
        {
            ObjectAdapterDeactivatedException e = new ObjectAdapterDeactivatedException();
	    e.name = _name;
	    throw e;
        }

        checkIdentity(ident);

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
            ObjectAdapterDeactivatedException e = new ObjectAdapterDeactivatedException();
	    e.name = _name;
	    throw e;
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
        }
    }

    public synchronized void
    setLocator(LocatorPrx locator)
    {
	if(_instance == null)
	{
	    ObjectAdapterDeactivatedException e = new ObjectAdapterDeactivatedException();
	    e.name = _name;
	    throw e;
	}

	_locatorInfo = _instance.locatorManager().get(locator);
    }

    public synchronized IceInternal.Connection[]
    getIncomingConnections()
    {
	if(_instance == null)
	{
	    throw new ObjectAdapterDeactivatedException();
	}

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
    incUsageCount()
    {
	//
	// Don't check whether deactivation has been initiated. This
	// operation might be called (e.g., from Incoming or Direct)
	// after deactivation has been initiated, but before
	// deactivation has been completed.
	//
	/*
	assert(_instance != null);
	*/
	assert(_usageCount >= 0);
	++_usageCount;
    }

    public synchronized void
    decUsageCount()
    {
	//
	// The object adapter may already be deactivated when the
	// usage count is decremented, thus no check for prior
	// deactivation.
	//

	assert(_usageCount > 0);
	--_usageCount;
	if(_usageCount == 0)
	{
	    _activeServantMap.clear();
	    
	    java.util.Iterator p = _locatorMap.entrySet().iterator();
	    while(p.hasNext())
	    {
                java.util.Map.Entry e = (java.util.Map.Entry)p.next();
		ServantLocator locator = (ServantLocator)e.getValue();
                try
                {
                    locator.deactivate();
                }
                catch(RuntimeException ex)
                {
                    java.io.StringWriter sw = new java.io.StringWriter();
                    java.io.PrintWriter pw = new java.io.PrintWriter(sw);
                    ex.printStackTrace(pw);
                    pw.flush();
                    String s = "exception during locator deactivation:\n" + "object adapter: `" + _name + "'\n" +
                        "locator prefix: `" + e.getKey() + "'\n" + sw.toString();
                    _logger.error(s);
                }
	    }
	    _locatorMap.clear();
            _logger = null;

	    notifyAll();
        }
    }

    //
    // Only for use by IceInternal.ObjectAdapterFactory
    //
    public
    ObjectAdapterI(IceInternal.Instance instance, Communicator communicator, String name, String endpts, String id)
    {
        _instance = instance;
	_communicator = communicator;
	_printAdapterReadyDone = false;
        _name = name;
	_id = id;
        _logger = instance.logger();
	_usageCount = 1;
	
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
    }

    protected void
    finalize()
        throws Throwable
    {
	assert(_usageCount == 0);

        if(_instance != null)
        {
            _instance.logger().warning("object adapter has not been deactivated");
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
	    IceInternal.Reference reference = _instance.referenceFactory().create(ident, new String[0],
										  IceInternal.Reference.ModeTwoway,
										  false, false, _id, 
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
    private String _id;
    private Logger _logger;
    private java.util.HashMap _activeServantMap = new java.util.HashMap();
    private java.util.HashMap _locatorMap = new java.util.HashMap();
    private java.util.ArrayList _incomingConnectionFactories = new java.util.ArrayList();
    private java.util.ArrayList _routerEndpoints = new java.util.ArrayList();
    private IceInternal.LocatorInfo _locatorInfo;
    private int _usageCount;
}
