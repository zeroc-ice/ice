// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
	IceInternal.LocatorInfo locatorInfo = null;
	boolean printAdapterReady = false;

	synchronized(this)
	{
	    checkForDeactivation();
	    
	    //
	    // If the one off initializations of the adapter are already
	    // done, we just need to activate the incoming connection
	    // factories and we're done.
	    //
	    if(_activateOneOffDone)
	    {
		final int sz = _incomingConnectionFactories.size();
		java.util.Enumeration e = _incomingConnectionFactories.elements();
		while(e.hasMoreElements())
		{
		    IceInternal.IncomingConnectionFactory factory =
			(IceInternal.IncomingConnectionFactory)e.nextElement();
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
	    final Properties properties = _instance.initializationData().properties;
	    printAdapterReady = properties.getPropertyAsInt("Ice.PrintAdapterReady") > 0;
	}

	try
	{
	    Ice.Identity dummy = new Ice.Identity();
	    dummy.name = "dummy";
	    updateLocatorRegistry(locatorInfo, createDirectProxy(dummy));
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
	    if(IceUtil.Debug.ASSERT)
	    {
		IceUtil.Debug.Assert(!_deactivated); // Not possible if _waitForActivate = true;
	    }
	    
	    //
	    // Signal threads waiting for the activation.
	    //
	    _waitForActivate = false;
	    notifyAll();

	    _activateOneOffDone = true;
	    
	    final int sz = _incomingConnectionFactories.size();
	    java.util.Enumeration e = _incomingConnectionFactories.elements();
	    while(e.hasMoreElements())
	    {
		IceInternal.IncomingConnectionFactory factory =
		    (IceInternal.IncomingConnectionFactory)e.nextElement();
		factory.activate();
	    }
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

    public void
    deactivate()
    {
	java.util.Vector incomingConnectionFactories;
	IceInternal.OutgoingConnectionFactory outgoingConnectionFactory;
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
	    
	    //
	    // No clone call with J2ME.
	    //
	    //incomingConnectionFactories = (java.util.Vector)_incomingConnectionFactories.clone();
	    incomingConnectionFactories = new java.util.Vector(_incomingConnectionFactories.size());
            java.util.Enumeration e = _incomingConnectionFactories.elements();
            while(e.hasMoreElements())
            {
                incomingConnectionFactories.addElement(e.nextElement());
            }
	    outgoingConnectionFactory = _instance.outgoingConnectionFactory();
	    locatorInfo = _locatorInfo;
	    
	    _deactivated = true;

	    notifyAll();
	}

	try
	{
	    updateLocatorRegistry(locatorInfo, null);
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
	// Connection::destroy() might block when sending a
	// CloseConnection message.
	//
	java.util.Enumeration e = incomingConnectionFactories.elements();
	while(e.hasMoreElements())
	{
	    IceInternal.IncomingConnectionFactory factory =
		(IceInternal.IncomingConnectionFactory)e.nextElement();
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

            incomingConnectionFactories = new IceInternal.IncomingConnectionFactory[_incomingConnectionFactories.size()];
            _incomingConnectionFactories.copyInto(incomingConnectionFactories);
        }

        //
        // Now we wait for until all incoming connection factories are
        // finished.
        //
        for(int i = 0; i < incomingConnectionFactories.length; ++i)
        {
            incomingConnectionFactories[i].waitUntilFinished();
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

        IceInternal.ObjectAdapterFactory objectAdapterFactory;

        synchronized(this)
        {
            //
            // Signal that destroying is complete.
            //
            _destroying = false;
            _destroyed = true;
            notifyAll();

            //
            // We're done, now we can throw away all incoming connection
            // factories.
            //
            // For compatibility with C#, we set _incomingConnectionFactories
            // to null so that the finalizer does not invoke methods on objects.
            //
            _incomingConnectionFactories = null;

            //
            // Remove object references (some of them cyclic).
            //
            _instance = null;
            _communicator = null;
            _routerEndpoints = null;
            _routerInfo = null;
            _publishedEndpoints = null;
            _locatorInfo = null;

            objectAdapterFactory = _objectAdapterFactory;
            _objectAdapterFactory = null;
        }

        if(objectAdapterFactory != null)
        {
            objectAdapterFactory.removeObjectAdapter(_name);
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
    createIndirectProxy(Identity ident)
    {
	checkForDeactivation();
        checkIdentity(ident);

        return newIndirectProxy(ident, "", _id);
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
        IceInternal.Reference ref = 
                _instance.referenceFactory().create(ident, null, "", IceInternal.Reference.ModeTwoway, arr);
        return _instance.proxyFactory().referenceToProxy(ref);
    }

    public synchronized void
    setLocator(LocatorPrx locator)
    {
	checkForDeactivation();

	_locatorInfo = _instance.locatorManager().get(locator);
    }

    public void
    flushBatchRequests()
    {
	java.util.Vector f;
	synchronized(this)
	{
	    //
	    // No clone() call with J2ME.
	    //
	    //f = (java.util.Vector)_incomingConnectionFactories.clone();
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
	//
	// No mutex lock necessary, _servantManager is immutable.
	//
	return _servantManager;
    }

    //
    // Only for use by IceInternal.ObjectAdapterFactory
    //
    public
    ObjectAdapter(IceInternal.Instance instance, Communicator communicator, 
		  IceInternal.ObjectAdapterFactory objectAdapterFactory, String name, String endpointInfo,
		  RouterPrx router)
    {
	_deactivated = false;
        _instance = instance;
	_communicator = communicator;
	_objectAdapterFactory = objectAdapterFactory;
	_servantManager = new IceInternal.ServantManager(instance, name);
	_activateOneOffDone = false;
        _name = name;
	_id = instance.initializationData().properties.getProperty(name + ".AdapterId");
	_replicaGroupId = instance.initializationData().properties.getProperty(name + ".ReplicaGroupId");
	_directCount = 0;
	_waitForActivate = false;
	_destroying = false;
	_destroyed = false;
	
        try
        {
            if(router == null)
            {
                String routerStr = _instance.initializationData().properties.getProperty(name + ".Router");
                if(routerStr.length() > 0)
                {
                    router = RouterPrxHelper.uncheckedCast(_instance.proxyFactory().stringToProxy(routerStr));
                }
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
                    IceInternal.Endpoint[] endpoints = _routerInfo.getServerEndpoints();
                    for(int i = 0; i < endpoints.length; ++i)
                    {
                        _routerEndpoints.addElement(endpoints[i]);
                    }
	    
                    IceUtil.Arrays.sort(_routerEndpoints); // Must be sorted.
	            //
	            // Remove duplicate endpoints, so we have a list of unique
	            // endpoints.
	            //
	            for(int i = 0; i < _routerEndpoints.size()-1; )
	            {
                        java.lang.Object o1 = _routerEndpoints.elementAt(i);
                        java.lang.Object o2 = _routerEndpoints.elementAt(i + 1);
                        if(o1.equals(o2))
                        {
                            _routerEndpoints.removeElementAt(i);
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
	        // Parse the endpoints, but don't store them in the adapter.
	        // The connection factory might change it, for example, to
	        // fill in the real port number.
	        //
	        java.util.Vector endpoints = parseEndpoints(endpointInfo);
	        for(int i = 0; i < endpoints.size(); ++i)
	        {
		    IceInternal.Endpoint endp = (IceInternal.Endpoint)endpoints.elementAt(i);
                    _incomingConnectionFactories.addElement(
		        new IceInternal.IncomingConnectionFactory(instance, endp, this));
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
	        // Parse published endpoints. These are used in proxies
	        // instead of the connection factory endpoints.
	        //
	        String endpts = _instance.initializationData().properties.getProperty(name + ".PublishedEndpoints");
	        _publishedEndpoints = parseEndpoints(endpts);
	    }

	    String locator = _instance.initializationData().properties.getProperty(name + ".Locator");
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
	    _instance.initializationData().logger.warning("object adapter `" + _name + "' has not been deactivated");
	}
	else if(!_destroyed)
	{
	    _instance.initializationData().logger.warning("object adapter `" + _name +
	    						  "' has not been destroyed");
	}
	else
	{
	    //IceUtil.Debug.FinalizerAssert(_servantManager == null); // Not cleared, it needs to be immutable.
	    IceUtil.Debug.FinalizerAssert(_communicator == null);
	    IceUtil.Debug.FinalizerAssert(_incomingConnectionFactories == null);
	    IceUtil.Debug.FinalizerAssert(_directCount == 0);
	    IceUtil.Debug.FinalizerAssert(!_waitForActivate);
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
	    _instance.referenceFactory().create(ident, null, facet, IceInternal.Reference.ModeTwoway, false, endpoints,
                                                null);
        return _instance.proxyFactory().referenceToProxy(reference);
    }

    private ObjectPrx
    newIndirectProxy(Identity ident, String facet, String id)
    {
	//
	// Create a reference with the adapter id and return a
	// proxy for the reference.
	//
	IceInternal.Endpoint[] endpoints = new IceInternal.Endpoint[0];
	Connection[] connections = new Connection[0];
	IceInternal.Reference reference =
	    _instance.referenceFactory().create(ident, null, facet, IceInternal.Reference.ModeTwoway, false, id, null,
						_locatorInfo);
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

    private void
    updateLocatorRegistry(IceInternal.LocatorInfo locatorInfo, Ice.ObjectPrx proxy)
    {
	if(_id.length() == 0)
	{
	    return; // Nothing to update.
	}

	//
	// We must get and call on the locator registry outside the
	// thread synchronization to avoid deadlocks. (we can't make
	// remote calls within the OA synchronization because the
	// remote call will indirectly call isLocal() on this OA with
	// the OA factory locked).
	//
	// TODO: This might throw if we can't connect to the
	// locator. Shall we raise a special exception for the
	// activate operation instead of a non obvious network
	// exception?
	//
	LocatorRegistryPrx locatorRegistry = locatorInfo != null ? locatorInfo.getLocatorRegistry() : null;
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
		NotRegisteredException ex1 = new NotRegisteredException();
		ex1.kindOfObject = "object adapter";
		ex1.id = _id;
		throw ex1;
	    }
	    catch(InvalidReplicaGroupIdException ex)
	    {
		NotRegisteredException ex1 = new NotRegisteredException();
		ex1.kindOfObject = "replica group";
		ex1.id = _replicaGroupId;
		throw ex1;
	    }
	    catch(AdapterAlreadyActiveException ex)
	    {
		ObjectAdapterIdInUseException ex1 = new ObjectAdapterIdInUseException();
		ex1.id = _id;
		throw ex1;
	    }
	}
    }    

    private boolean _deactivated;
    private IceInternal.Instance _instance;
    private Communicator _communicator;
    private IceInternal.ObjectAdapterFactory _objectAdapterFactory;
    private IceInternal.ServantManager _servantManager;
    private boolean _activateOneOffDone;
    private /*final*/ String _name;
    private /*final*/ String _id;
    private /*final*/ String _replicaGroupId;
    private java.util.Vector _incomingConnectionFactories = new java.util.Vector();
    private java.util.Vector _routerEndpoints = new java.util.Vector();
    private IceInternal.RouterInfo _routerInfo = null;
    private java.util.Vector _publishedEndpoints = new java.util.Vector();
    private IceInternal.LocatorInfo _locatorInfo;
    private int _directCount;
    private boolean _waitForActivate;
    private boolean _destroying;
    private boolean _destroyed;
}
