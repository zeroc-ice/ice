// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public final class ObjectAdapter
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

    public void
    activate()
    {
	Ice.LocatorRegistryPrx locatorRegistry = null;
	boolean printAdapterReady = false;

	synchronized(this)
	{
	    checkForDeactivation();
	    
	    if(!_printAdapterReadyDone)
	    {
		if(_locatorInfo != null && _id.length() > 0)
		{
		    locatorRegistry = _locatorInfo.getLocatorRegistry();
		}

		printAdapterReady = _instance.properties().getPropertyAsInt("Ice.PrintAdapterReady") > 0;
		_printAdapterReadyDone = true;
	    }
	    
	    final int sz = _incomingConnectionFactories.size();
	    java.util.Enumeration e = _incomingConnectionFactories.elements();
	    while(e.hasMoreElements())
	    {
		IceInternal.IncomingConnectionFactory factory =
		    (IceInternal.IncomingConnectionFactory)e.nextElement();
		factory.activate();
	    }
	}

	//
	// We must call on the locator registry outside the thread
	// synchronization, to avoid deadlocks.
	//
	if(locatorRegistry != null)
	{
	    //
	    // TODO: This might throw if we can't connect to the
	    // locator. Shall we raise a special exception for the
	    // activate operation instead of a non obvious network
	    // exception?
	    //
	    try
	    {
		Identity ident = new Identity();
		ident.category = "";
		ident.name = "dummy";
		locatorRegistry.setAdapterDirectProxy(_id, createDirectProxy(ident));
	    }
	    catch(ObjectAdapterDeactivatedException ex)
	    {
		// IGNORE: The object adapter is already inactive.
	    }
	    catch(AdapterNotFoundException ex)
	    {
		NotRegisteredException ex1 = new NotRegisteredException();
		ex1.id = _id;
		ex1.kindOfObject = "object adapter";
		throw ex1;
	    }
	    catch(AdapterAlreadyActiveException ex)
	    {
		ObjectAdapterIdInUseException ex1 = new ObjectAdapterIdInUseException();
		ex1.id = _id;
		throw ex1;
	    }
	}

	if(printAdapterReady)
	{
	    System.out.println(_name + " ready");
	}
    }

    public synchronized void
    hold()
    {
	checkForDeactivation();
	
	java.util.Enumeration e = _incomingConnectionFactories.elements();
	while(e.hasMoreElements())
	{
            IceInternal.IncomingConnectionFactory factory =
                (IceInternal.IncomingConnectionFactory)e.nextElement();
            factory.hold();
        }
    }

    public synchronized void
    waitForHold()
    {
	checkForDeactivation();
	
	java.util.Enumeration e = _incomingConnectionFactories.elements();
	while(e.hasMoreElements())
	{
	    IceInternal.IncomingConnectionFactory factory =
		(IceInternal.IncomingConnectionFactory)e.nextElement();
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
	
	java.util.Enumeration e = _incomingConnectionFactories.elements();
	while(e.hasMoreElements())
	{
	    IceInternal.IncomingConnectionFactory factory =
		(IceInternal.IncomingConnectionFactory)e.nextElement();
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
	if(_incomingConnectionFactories != null)
	{
	    java.util.Enumeration e = _incomingConnectionFactories.elements();
	    while(e.hasMoreElements())
	    {
		IceInternal.IncomingConnectionFactory factory =
		    (IceInternal.IncomingConnectionFactory)e.nextElement();
		factory.waitUntilFinished();
	    }
	}
	
	//
	// Now it's also time to clean up our servants and servant
	// locators.
	//
	if(_servantManager != null)
	{
	    _servantManager.destroy();
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
	    _incomingConnectionFactories = null;
	    
	    //
	    // Remove object references (some of them cyclic).
	    //
	    _instance = null;
	    _servantManager = null;
	    _communicator = null;
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
        ident.name = Util.generateUUID();

        return addFacet(object, ident, facet);
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

    public synchronized java.util.Hashtable
    removeAllFacets(Identity ident)
    {
	checkForDeactivation();
        checkIdentity(ident);

	return _servantManager.removeAllFacets(ident);
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

    public synchronized java.util.Hashtable
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
    createReverseProxy(Identity ident)
    {
	checkForDeactivation();
        checkIdentity(ident);

        //
        // Get all incoming connections for this object adapter.
        //
        java.util.Vector connections = new java.util.Vector();
        java.util.Enumeration e = _incomingConnectionFactories.elements();
	while(e.hasMoreElements())
        {
            IceInternal.IncomingConnectionFactory factory =
                (IceInternal.IncomingConnectionFactory)e.nextElement();
            Connection[] conns = factory.connections();
            for(int j = 0; j < conns.length; ++j)
            {
                connections.addElement(conns[j]);
            }
        }

        //
        // Create a reference and return a reverse proxy for this
        // reference.
        //
        IceInternal.Endpoint[] endpoints = new IceInternal.Endpoint[0];
        Connection[] arr = new Connection[connections.size()];
        connections.copyInto( arr);
        IceInternal.Reference ref = _instance.referenceFactory().create(ident, new java.util.Hashtable(), "",
                                                                        IceInternal.Reference.ModeTwoway, arr);
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
            ObjectPrxHelperBase proxy = (ObjectPrxHelperBase)routerInfo.getServerProxy();
            IceInternal.Endpoint[] endpoints = proxy.__reference().getEndpoints();
            for(int i = 0; i < endpoints.length; ++i)
            {
                _routerEndpoints.addElement(endpoints[i]);
            }
	    
            IceUtil.Arrays.sort(_routerEndpoints); // Must be sorted.
            for(int i = 0; i < _routerEndpoints.size() - 1; ++i)
            {
		//
		// TODO: Will this not fail if there are three router endpoints in row for some reason? 
		//
                java.lang.Object o1 = _routerEndpoints.elementAt(i);
                java.lang.Object o2 = _routerEndpoints.elementAt(i + 1);
                if(o1.equals(o2))
                {
                    _routerEndpoints.removeElementAt(i);
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

    public synchronized LocatorPrx
    getLocator()
    {
        checkForDeactivation();

        LocatorPrx locator = null;

        if(_locatorInfo != null)
        {
            locator = _locatorInfo.getLocator();
        }

        return locator;
    }

    public synchronized boolean
    isLocal(ObjectPrx proxy)
    {
	checkForDeactivation();

        IceInternal.Reference ref = ((ObjectPrxHelperBase)proxy).__reference();
        final IceInternal.Endpoint[] endpoints = ref.getEndpoints();

	try
	{
	    IceInternal.IndirectReference ir = (IceInternal.IndirectReference)ref;
	    if(ir.getAdapterId().length() != 0)
	    {
		//
		// Proxy is local if the reference adapter id matches this
		// adapter name.
		//
		return ir.getAdapterId().equals(_id);
	    }
	}
	catch(ClassCastException e)
	{
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
                    (IceInternal.IncomingConnectionFactory)_incomingConnectionFactories.elementAt(j);
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
	    if(IceUtil.Arrays.search(_routerEndpoints, endpoints[i]) >= 0) // _routerEndpoints is sorted.
	    {
		return true;
	    }
	}

        return false;
    }

    public void
    flushBatchRequests()
    {
	java.util.Vector f;
	synchronized(this)
	{
	    f = new java.util.Vector(_incomingConnectionFactories.size());
	    java.util.Enumeration e = _incomingConnectionFactories.elements();
	    while(e.hasMoreElements())
	    {
		f.addElement(e.nextElement());
	    }
	}
	java.util.Enumeration i = f.elements();
	while(i.hasMoreElements())
	{
	    ((IceInternal.IncomingConnectionFactory)i.nextElement()).flushBatchRequests();
	}
    }

    public synchronized void
    incDirectCount()
    {
	checkForDeactivation();

	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(_directCount >= 0);
	}
	++_directCount;
    }

    public synchronized void
    decDirectCount()
    {
	// No check for deactivation here!
	
	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(_instance != null); // Must not be called after waitForDeactivate().
	    IceUtil.Debug.Assert(_directCount > 0);
	}
	if(--_directCount == 0)
	{
	    notifyAll();
	}
    }

    public IceInternal.ServantManager
    getServantManager()
    {	
	// No mutex lock necessary, _instance is
	// immutable after creation until it is removed in
	// waitForDeactivate().
	
	// No check for deactivation here!
	
	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(_instance != null); // Must not be called after waitForDeactivate().
	}

	return _servantManager;
    }

    //
    // Only for use by IceInternal.ObjectAdapterFactory
    //
    public
    ObjectAdapter(IceInternal.Instance instance, Communicator communicator, String name)
    {
	_deactivated = false;
        _instance = instance;
	_communicator = communicator;
	_servantManager = new IceInternal.ServantManager(instance, name);
	_printAdapterReadyDone = false;
        _name = name;
	_id = instance.properties().getProperty(name + ".AdapterId");
	_directCount = 0;
	_waitForDeactivate = false;
	
        try
        {
	    //
	    // Parse the endpoints, but don't store them in the adapter.
	    // The connection factory might change it, for example, to
	    // fill in the real port number.
	    //
	    String endpts = _instance.properties().getProperty(name + ".Endpoints");
	    java.util.Vector endpoints = parseEndpoints(endpts);
	    for(int i = 0; i < endpoints.size(); ++i)
	    {
		IceInternal.Endpoint endp = (IceInternal.Endpoint)endpoints.elementAt(i);
                _incomingConnectionFactories.addElement(
		    new IceInternal.IncomingConnectionFactory(instance, endp, this));
            }

	    //
	    // Parse published endpoints. These are used in proxies
	    // instead of the connection factory endpoints.
	    //
	    endpts = _instance.properties().getProperty(name + ".PublishedEndpoints");
	    _publishedEndpoints = parseEndpoints(endpts);

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
        }
        catch(LocalException ex)
        {
	    deactivate();
	    waitForDeactivate();
            throw ex;
        }
    }

    protected synchronized void
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
	    IceUtil.Debug.FinalizerAssert(_servantManager == null);
	    IceUtil.Debug.FinalizerAssert(_communicator == null);
	    IceUtil.Debug.FinalizerAssert(_incomingConnectionFactories == null);
	    IceUtil.Debug.FinalizerAssert(_directCount == 0);
	    IceUtil.Debug.FinalizerAssert(!_waitForDeactivate);
	}
    }

    private ObjectPrx
    newProxy(Identity ident, String facet)
    {
	if(_id.length() == 0)
	{
	    return newDirectProxy(ident, facet);
	}
	else
	{	    
	    //
	    // Create a reference with the adapter id and return a
	    // proxy for the reference.
	    //
	    IceInternal.Endpoint[] endpoints = new IceInternal.Endpoint[0];
            Connection[] connections = new Connection[0];
	    IceInternal.Reference reference =
		_instance.referenceFactory().create(ident, new java.util.Hashtable(), facet,
		                                    IceInternal.Reference.ModeTwoway, false, _id, null,
                                                    _locatorInfo);
	    return _instance.proxyFactory().referenceToProxy(reference);
	}
    }

    private ObjectPrx
    newDirectProxy(Identity ident, String facet)
    {
        IceInternal.Endpoint[] endpoints;

	// 
	// Use the published endpoints, otherwise use the endpoints from all
	// incoming connection factories.
	//
	int sz = _publishedEndpoints.size();
	if(sz > 0)
	{
	    endpoints = new IceInternal.Endpoint[sz + _routerEndpoints.size()];
	    _publishedEndpoints.copyInto(endpoints);
	}
	else
	{
	    sz = _incomingConnectionFactories.size();
	    endpoints = new IceInternal.Endpoint[sz + _routerEndpoints.size()];
	    for(int i = 0; i < sz; ++i)
	    {
		IceInternal.IncomingConnectionFactory factory =
		    (IceInternal.IncomingConnectionFactory)_incomingConnectionFactories.elementAt(i);
		endpoints[i] = factory.endpoint();
	    }
	}

        //
        // Now we also add the endpoints of the router's server proxy, if
        // any. This way, object references created by this object adapter
        // will also point to the router's server proxy endpoints.
        //
        for(int i = 0; i < _routerEndpoints.size(); ++i)
        {
	    endpoints[sz + i] = (IceInternal.Endpoint)_routerEndpoints.elementAt(i);
        }

        //
        // Create a reference and return a proxy for this reference.
        //
        Connection[] connections = new Connection[0];
        IceInternal.Reference reference =
	    _instance.referenceFactory().create(ident, new java.util.Hashtable(), facet,
						IceInternal.Reference.ModeTwoway, false, endpoints, null);
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
                e.id = (Identity)ident.ice_clone();
            }
            catch(IceUtil.CloneException ex)
            {
		if(IceUtil.Debug.ASSERT)
		{
		    IceUtil.Debug.Assert(false);
		}
            }
            throw e;
        }

        if(ident.category == null)
        {
            ident.category = "";
        }
    }

    private java.util.Vector
    parseEndpoints(String endpts)
    {
        endpts = endpts.toLowerCase();

	int beg;
	int end = 0;

	final String delim = " \t\n\r";

	java.util.Vector endpoints = new java.util.Vector();
	while(end < endpts.length())
	{
	    beg = IceUtil.StringUtil.findFirstNotOf(endpts, delim, end);
	    if(beg == -1)
	    {
		break;
	    }

	    end = endpts.indexOf(':', beg);
	    if(end == -1)
	    {
		end = endpts.length();
	    }

	    if(end == beg)
	    {
		++end;
		continue;
	    }

	    String s = endpts.substring(beg, end);
	    IceInternal.Endpoint endp = _instance.endpointFactory().create(s);
	    if(endp == null)
	    {
	        Ice.EndpointParseException e = new Ice.EndpointParseException();
		e.str = s;
		throw e;
	    }
	    endpoints.addElement(endp);

	    ++end;
	}

	return endpoints;
    }

    private boolean _deactivated;
    private IceInternal.Instance _instance;
    private Communicator _communicator;
    private IceInternal.ServantManager _servantManager;
    private boolean _printAdapterReadyDone;
    final private String _name;
    final private String _id;
    private java.util.Vector _incomingConnectionFactories = new java.util.Vector();
    private java.util.Vector _routerEndpoints = new java.util.Vector();
    private java.util.Vector _publishedEndpoints;
    private IceInternal.LocatorInfo _locatorInfo;
    private int _directCount;
    private boolean _waitForDeactivate;
}
