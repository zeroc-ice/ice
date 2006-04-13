// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Ice
{
    using System;
    using System.Collections;
    using System.Diagnostics;

    public sealed class ObjectAdapterI : LocalObjectImpl, ObjectAdapter
    {
	public string getName()
	{
	    //
	    // No mutex lock necessary, _name is immutable.
	    //
	    return _name;
	}

	public Communicator getCommunicator()
	{
	    lock(this)
	    {
		checkForDeactivation();
		
		return _communicator;
	    }
	}

	public void activate()
	{
	    IceInternal.LocatorInfo locatorInfo = null;
	    bool registerProcess = false;
	    string serverId = "";
	    Communicator communicator = null;
	    bool printAdapterReady = false;

	    lock(this)
	    {
		checkForDeactivation();
		
		if(!_printAdapterReadyDone)
		{
		    locatorInfo = _locatorInfo;
		    registerProcess = 
		        instance_.initializationData().properties.getPropertyAsInt(_name + ".RegisterProcess") > 0;
		    serverId = instance_.initializationData().properties.getProperty("Ice.ServerId");
		    printAdapterReady =
		        instance_.initializationData().properties.getPropertyAsInt("Ice.PrintAdapterReady") > 0;
		    communicator = _communicator;
		    _printAdapterReadyDone = true;
		}
		
		foreach(IceInternal.IncomingConnectionFactory icf in _incomingConnectionFactories)
		{
		    icf.activate();
		}
	    }

	    if(registerProcess || _id.Length > 0)
	    {
		//
		// We must get and call on the locator registry outside the thread
		// synchronization to avoid deadlocks. (we can't make remote calls
		// within the OA synchronization because the remote call will
		// indirectly call isLocal() on this OA with the OA factory
		// locked).
		//

		LocatorRegistryPrx locatorRegistry = null;
		if(locatorInfo != null)
		{
		    //
		    // TODO: This might throw if we can't connect to the
		    // locator. Shall we raise a special exception for the
		    // activate operation instead of a non obvious network
		    // exception?
		    //
		    locatorRegistry = locatorInfo.getLocatorRegistry();
		}
	    
		if(locatorRegistry != null && _id.Length > 0)
		{
		    try 
		    {
			Identity ident = new Identity();
			ident.category = "";
			ident.name = "dummy";
			if(_replicaGroupId.Length == 0)
			{
			    locatorRegistry.setAdapterDirectProxy(_id, createDirectProxy(ident));
			}
			else
			{
			    locatorRegistry.setReplicatedAdapterDirectProxy(_id, _replicaGroupId, 
									    createDirectProxy(ident));
			}
		    }
		    catch(Ice.ObjectAdapterDeactivatedException)
		    {
			// IGNORE: The object adapter is already inactive.
		    }
		    catch(Ice.AdapterNotFoundException)
		    {
			NotRegisteredException ex1 = new NotRegisteredException();
			ex1.kindOfObject = "object adapter";
			ex1.id = _id;
			throw ex1;
		    }
		    catch(Ice.InvalidReplicaGroupIdException)
		    {
			NotRegisteredException ex1 = new NotRegisteredException();
			ex1.kindOfObject = "replica group";
			ex1.id = _replicaGroupId;
			throw ex1;
		    }
		    catch(Ice.AdapterAlreadyActiveException)
		    {
			ObjectAdapterIdInUseException ex1 = new ObjectAdapterIdInUseException();
			ex1.id = _id;
			throw ex1;
		    }
		}

		if(registerProcess)
		{
		    if(locatorRegistry == null)
		    {
			communicator.getLogger().warning("object adapter `" + _name + "' cannot register the " +
							 "process without alocator registry");
		    }
		    else if(serverId.Length == 0)
		    {
			communicator.getLogger().warning("object adapter `" + _name + "' cannot register the " +
							 "process without a value for Ice.ServerId");
		    }
		    else
		    {
			try
			{
			    Process servant = new ProcessI(communicator);
			    Ice.ObjectPrx proxy = createDirectProxy(addWithUUID(servant).ice_getIdentity());
			    locatorRegistry.setServerProcessProxy(serverId, ProcessPrxHelper.uncheckedCast(proxy));
			} 
			catch(Ice.ObjectAdapterDeactivatedException)
			{
			    // IGNORE: The object adapter is already inactive.
			}
			catch(ServerNotFoundException)
			{
			    NotRegisteredException ex1 = new NotRegisteredException();
			    ex1.id = serverId;
			    ex1.kindOfObject = "server";
			    throw ex1;
			}
		    }
		}
	    }

	    if(printAdapterReady)
	    {
		System.Console.Out.WriteLine(_name + " ready");
	    }
	}
	
	public void hold()
	{
	    lock(this)
	    {
		checkForDeactivation();
		
		int sz = _incomingConnectionFactories.Count;
		for(int i = 0; i < sz; ++i)
		{
		    IceInternal.IncomingConnectionFactory factory =
			(IceInternal.IncomingConnectionFactory)_incomingConnectionFactories[i];
		    factory.hold();
		}
	    }
	}
	
	public void waitForHold()
	{
	    lock(this)
	    {
		checkForDeactivation();
		
		int sz = _incomingConnectionFactories.Count;
		for(int i = 0; i < sz; ++i)
		{
		    IceInternal.IncomingConnectionFactory factory =
			(IceInternal.IncomingConnectionFactory)_incomingConnectionFactories[i];
		    factory.waitUntilHolding();
		}
	    }
	}
	
	public void deactivate()
	{
	    IceInternal.OutgoingConnectionFactory outgoingConnectionFactory;
	    ArrayList incomingConnectionFactories;

	    lock(this)
	    {
		//
		// Ignore deactivation requests if the object adapter has
		// already been deactivated.
		//
		if(_deactivated)
		{
		    return;
		}

		if(_routerInfo != null)
		{
		    //
		    // Remove entry from the router manager.
		    //
		    instance_.routerManager().erase(_routerInfo.getRouter());

		    //
		    // Clear this object adapter with the router.
		    //
		    _routerInfo.setAdapter(null);

		    //
		    // Update all existing outgoing connections.
		    //
		    instance_.outgoingConnectionFactory().setRouterInfo(_routerInfo);
		}
		
		incomingConnectionFactories = new ArrayList(_incomingConnectionFactories);
		outgoingConnectionFactory = instance_.outgoingConnectionFactory();
		
		_deactivated = true;
		
		System.Threading.Monitor.PulseAll(this);
	    }

	    //
	    // Must be called outside the thread synchronization, because
	    // Connection::destroy() might block when sending a CloseConnection
	    // message.
	    //
	    int sz = incomingConnectionFactories.Count;
	    for(int i = 0; i < sz; ++i)
	    {
		IceInternal.IncomingConnectionFactory factory =
		    (IceInternal.IncomingConnectionFactory)incomingConnectionFactories[i];
		factory.destroy();
	    }

	    //
	    // Must be called outside the thread synchronization, because
	    // changing the object adapter might block if there are still
	    // requests being dispatched.
	    //
	    outgoingConnectionFactory.removeAdapter(this);
	}
	
	public void waitForDeactivate()
	{
	    lock(this)
	    {
		//
		// First we wait for deactivation of the adapter itself, and
		// for the return of all direct method calls using this
		// adapter.
		//
		while(!_deactivated || _directCount > 0)
		{
		    System.Threading.Monitor.Wait(this);
		}
		
		//
		// If some other thread is currently deactivating, we wait
		// until this thread is finished.
		//
		while(_waitForDeactivate)
		{
		    System.Threading.Monitor.Wait(this);
		}
		_waitForDeactivate = true;
	    }
	    
	    
	    //
	    // Now we wait for until all incoming connection factories are
	    // finished.
	    //
	    if(_incomingConnectionFactories != null)
	    {
		int sz = _incomingConnectionFactories.Count;
		for(int i = 0; i < sz; ++i)
		{
		    IceInternal.IncomingConnectionFactory factory =
			(IceInternal.IncomingConnectionFactory)_incomingConnectionFactories[i];
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
	    
	    //
	    // Destroy the thread pool.
	    //
	    if(_threadPool != null)
	    {
		_threadPool.destroy();
		_threadPool.joinWithAllThreads();
	    }

	    IceInternal.ObjectAdapterFactory objectAdapterFactory;
	    
	    lock(this)
	    {
		//
		// Signal that waiting is complete.
		//
		_waitForDeactivate = false;
		System.Threading.Monitor.PulseAll(this);
		
		//
		// We're done, now we can throw away all incoming connection
		// factories.
		//
		// We set _incomingConnectionFactories to null because the finalizer
		// must not invoke methods on objects.
		//
		_incomingConnectionFactories = null;
		
		//
		// Remove object references (some of them cyclic).
		//
		instance_ = null;
		_threadPool = null;
		_servantManager = null;
		_communicator = null;
		_incomingConnectionFactories = null;
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

	public void destroy()
	{
	    deactivate();
	    waitForDeactivate();
	}
	
	public ObjectPrx add(Ice.Object obj, Identity ident)
	{
	    return addFacet(obj, ident, "");
	}

	public ObjectPrx addFacet(Ice.Object obj, Identity ident, string facet)
	{
	    lock(this)
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

		_servantManager.addServant(obj, id, facet);

		return newProxy(id, facet);
	    }
	}
	
	public ObjectPrx addWithUUID(Ice.Object obj)
	{
	    return addFacetWithUUID(obj, "");
	}

	public ObjectPrx addFacetWithUUID(Ice.Object obj, string facet)
	{
	    Identity ident = new Identity();
	    ident.category = "";
	    ident.name = Util.generateUUID();
	    
	    return addFacet(obj, ident, facet);
	}
	
	public Ice.Object remove(Identity ident)
	{
	    return removeFacet(ident, "");
	}

	public Ice.Object removeFacet(Identity ident, string facet)
	{
	    lock(this)
	    {
		checkForDeactivation();
		checkIdentity(ident);
		
		return _servantManager.removeServant(ident, facet);
	    }
	}

	public FacetMap removeAllFacets(Identity ident)
	{
	    lock(this)
	    {
	        checkForDeactivation();
		checkIdentity(ident);

		return _servantManager.removeAllFacets(ident);
	    }
	}

	public Ice.Object find(Identity ident)
	{
	    return findFacet(ident, "");
	}

	public Ice.Object findFacet(Identity ident, string facet)
	{
	    lock(this)
	    {
		checkForDeactivation();
		checkIdentity(ident);

		return _servantManager.findServant(ident, facet);
	    }
	}

	public FacetMap findAllFacets(Identity ident)
	{
	    lock(this)
	    {
		checkForDeactivation();
		checkIdentity(ident);

		return _servantManager.findAllFacets(ident);
	    }
	}

	public Ice.Object findByProxy(ObjectPrx proxy)
	{
	    lock(this)
	    {
		checkForDeactivation();

		IceInternal.Reference @ref = ((ObjectPrxHelperBase)proxy).reference__();
		return findFacet(@ref.getIdentity(), @ref.getFacet());
	    }
	}
	
	public void addServantLocator(ServantLocator locator, string prefix)
	{
	    lock(this)
	    {
		checkForDeactivation();
		
		_servantManager.addServantLocator(locator, prefix);
	    }
	}
	
	public ServantLocator findServantLocator(string prefix)
	{
	    lock(this)
	    {
		checkForDeactivation();
		
		return _servantManager.findServantLocator(prefix);
	    }
	}
	
	public ObjectPrx createProxy(Identity ident)
	{
	    lock(this)
	    {
		checkForDeactivation();
		checkIdentity(ident);
		
		return newProxy(ident, "");
	    }
	}
	
	public ObjectPrx createDirectProxy(Identity ident)
	{
	    lock(this)
	    {
		checkForDeactivation();
		checkIdentity(ident);
		
		return newDirectProxy(ident, "");
	    }
	}
	
	public ObjectPrx createIndirectProxy(Identity ident)
	{
	    lock(this)
	    {
		checkForDeactivation();
		checkIdentity(ident);
		
		return newIndirectProxy(ident, "", _id);
	    }
	}
	
	public ObjectPrx createReverseProxy(Identity ident)
	{
	    lock(this)
	    {
		checkForDeactivation();
		checkIdentity(ident);
		
		//
		// Get all incoming connections for this object adapter.
		//
		ArrayList connections = new ArrayList();
		int sz = _incomingConnectionFactories.Count;
		for(int i = 0; i < sz; ++i)
		{
		    IceInternal.IncomingConnectionFactory factory
			= (IceInternal.IncomingConnectionFactory)_incomingConnectionFactories[i];
		    ConnectionI[] cons = factory.connections();
		    for(int j = 0; j < cons.Length; j++)
		    {
			connections.Add(cons[j]);
		    }
		}

		//
		// Create a reference and return a reverse proxy for this
		// reference.
		//
		ConnectionI[] arr = new ConnectionI[connections.Count];
		if(arr.Length != 0)
		{
		    connections.CopyTo(arr, 0);
		}
                IceInternal.Reference @ref = instance_.referenceFactory().create(
		    ident, instance_.initializationData().defaultContext, "", IceInternal.Reference.Mode.ModeTwoway, 
		    arr);
		return instance_.proxyFactory().referenceToProxy(@ref);
	    }
	}
	
	public void setLocator(LocatorPrx locator)
	{
	    lock(this)
	    {
		checkForDeactivation();
		
		_locatorInfo = instance_.locatorManager().get(locator);
	    }
	}

	public bool isLocal(ObjectPrx proxy)
	{
	    IceInternal.Reference r = ((ObjectPrxHelperBase)proxy).reference__();
	    IceInternal.EndpointI[] endpoints;
	    
	    try
	    {
		IceInternal.IndirectReference ir = (IceInternal.IndirectReference)r;
		if(ir.getAdapterId().Length != 0)
		{
		    //
		    // Proxy is local if the reference adapter id matches this
		    // adapter name.
		    //
		    return ir.getAdapterId().Equals(_id);
		}
		IceInternal.LocatorInfo info = ir.getLocatorInfo();
		if(info != null)
		{
		    bool isCached;
		    endpoints = info.getEndpoints(ir, ir.getLocatorCacheTimeout(), out isCached);
		}
		else
		{
		    return false;
		}
	    }
	    catch(InvalidCastException)
	    {
		endpoints = r.getEndpoints();
	    }
	    
	    lock(this)
	    {
		checkForDeactivation();

		//
		// Proxies which have at least one endpoint in common with the
		// endpoints used by this object adapter's incoming connection
		// factories are considered local.
		//
		for(int i = 0; i < endpoints.Length; ++i)
		{
		    int sz = _incomingConnectionFactories.Count;
		    for(int j = 0; j < sz; j++)
		    {
			IceInternal.IncomingConnectionFactory factory
			    = (IceInternal.IncomingConnectionFactory)_incomingConnectionFactories[j];
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
		if(_routerInfo != null && _routerInfo.getRouter().Equals(proxy.ice_getRouter()))
		{
		    for(int i = 0; i < endpoints.Length; ++i)
		    {
		        if(_routerEndpoints.BinarySearch(endpoints[i]) >= 0) // _routerEndpoints is sorted.
		        {
			    return true;
		        }
		    }
		}
		
		return false;
	    }
	}
	
	public void flushBatchRequests()
	{
	    ArrayList f;
	    lock(this)
	    {
		f = new ArrayList(_incomingConnectionFactories);
	    }

	    foreach(IceInternal.IncomingConnectionFactory factory in f)
	    {
		factory.flushBatchRequests();
	    }
	}

	public void incDirectCount()
	{
	    lock(this)
	    {
		checkForDeactivation();
		
		Debug.Assert(_directCount >= 0);
		++_directCount;
	    }
	}
	
	public void decDirectCount()
	{
	    lock(this)
	    {
		// Not check for deactivation here!
		
		Debug.Assert(instance_ != null); // Must not be called after waitForDeactivate().
		
		Debug.Assert(_directCount > 0);
		if(--_directCount == 0)
		{
		    System.Threading.Monitor.PulseAll(this);
		}
	    }
	}
	
	public IceInternal.ThreadPool getThreadPool()
	{
	    // No mutex lock necessary, _threadPool and instance_ are
	    // immutable after creation until they are removed in
	    // waitForDeactivate().
	    
	    // Not check for deactivation here!
	    
	    Debug.Assert(instance_ != null); // Must not be called after waitForDeactivate().
	    
	    if(_threadPool != null)
	    {
		return _threadPool;
	    }
	    else
	    {
		return instance_.serverThreadPool();
	    }
	    
	}

	public IceInternal.ServantManager getServantManager()
	{
	    // No mutex lock necessary, _threadPool and instance_ are
	    // immutable after creation until they are removed in
	    // waitForDeactivate().
	    
	    // Not check for deactivation here!
	    
	    Debug.Assert(instance_ != null); // Must not be called after waitForDeactivate().
	    
	    return _servantManager;
	}
	
	//
	// Only for use by IceInternal.ObjectAdapterFactory
	//
	public ObjectAdapterI(IceInternal.Instance instance, Communicator communicator,
			      IceInternal.ObjectAdapterFactory objectAdapterFactory, string name, 
			      string endpointInfo, RouterPrx router)
	{
	    _deactivated = false;
	    instance_ = instance;
	    _communicator = communicator;
	    _objectAdapterFactory = objectAdapterFactory;
	    _servantManager = new IceInternal.ServantManager(instance, name);
	    _printAdapterReadyDone = false;
	    _name = name;
	    _id = instance.initializationData().properties.getProperty(name + ".AdapterId");
	    _replicaGroupId = instance.initializationData().properties.getProperty(name + ".ReplicaGroupId");
	    _incomingConnectionFactories = new ArrayList();
	    _publishedEndpoints = new ArrayList();
	    _routerEndpoints = new ArrayList();
	    _routerInfo = null;
	    _directCount = 0;
	    _waitForDeactivate = false;
	    
	    try
	    {
	        if(router == null)
		{
		    string routerStr = instance_.initializationData().properties.getProperty(name + ".Router");
		    if(routerStr.Length > 0)
		    {
		        router = RouterPrxHelper.uncheckedCast(instance_.proxyFactory().stringToProxy(routerStr));
		    }
		}
		if(router != null)
		{
		    _routerInfo = instance_.routerManager().get(router);
		    if(_routerInfo != null)
		    {
                        //
                        // Make sure this router is not already registered with another adapter.
                        //
                        if(_routerInfo.getAdapter() != null)
                        {
			    Ice.AlreadyRegisteredException ex = new Ice.AlreadyRegisteredException();
			    ex.kindOfObject = "object adapter with router";
			    ex.id = Ice.Util.identityToString(router.ice_getIdentity());
			    throw ex;
                        }

		        //
		        // Add the router's server proxy endpoints to this object
		        // adapter.
		        //
		        ObjectPrxHelperBase proxy = (ObjectPrxHelperBase)_routerInfo.getServerProxy();
		        IceInternal.EndpointI[] endpoints = proxy.reference__().getEndpoints();
		        for(int i = 0; i < endpoints.Length; ++i)
		        {
			    _routerEndpoints.Add(endpoints[i]);
		        }
		        _routerEndpoints.Sort(); // Must be sorted.

		        //
		        // Remove duplicate endpoints, so we have a list of unique endpoints.
		        //
		        for(int i = 0; i < _routerEndpoints.Count-1;)
		        {
			    System.Object o1 = _routerEndpoints[i];
			    System.Object o2 = _routerEndpoints[i + 1];
			    if(o1.Equals(o2))
			    {
			        _routerEndpoints.RemoveAt(i);
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
		        instance_.outgoingConnectionFactory().setRouterInfo(_routerInfo);
		    }
		}
		else
		{
		    //
		    // Parse the endpoints, but don't store them in the adapter.
		    // The connection factory might change it, for example, to
		    // fill in the real port number.
		    //
		    ArrayList endpoints = parseEndpoints(endpointInfo);
		    for(int i = 0; i < endpoints.Count; ++i)
		    {
		        IceInternal.EndpointI endp = (IceInternal.EndpointI)endpoints[i];
		        _incomingConnectionFactories.Add(new IceInternal.IncomingConnectionFactory(instance, endp, this));
		    }
		
		    //
		    // Parse published endpoints. If set, these are used in proxies
		    // instead of the connection factory endpoints.
		    //
		    string endpts = instance_.initializationData().properties.getProperty(name + ".PublishedEndpoints");
		    _publishedEndpoints = parseEndpoints(endpts);
		    if(_publishedEndpoints.Count == 0)
		    {
		        foreach(IceInternal.IncomingConnectionFactory factory in _incomingConnectionFactories)
		        {
		            _publishedEndpoints.Add(factory.endpoint());
		        }
		    }

		    //
		    // Filter out any endpoints that are not meant to be published.
		    //
		    ArrayList tmp = new ArrayList();
		    foreach(IceInternal.EndpointI endpoint in _publishedEndpoints)
		    {
		        if(endpoint.publish())
		        {
		            tmp.Add(endpoint);
		        }
		    }
		    _publishedEndpoints = tmp;
		}

		string locator = instance_.initializationData().properties.getProperty(name + ".Locator");
		if(locator.Length > 0)
		{
		    setLocator(LocatorPrxHelper.uncheckedCast(instance_.proxyFactory().stringToProxy(locator)));
		}
		else
		{
		    setLocator(instance_.referenceFactory().getDefaultLocator());
		}
		
		if(!instance_.threadPerConnection())
		{
		    int size = instance_.initializationData().properties.getPropertyAsInt(_name + ".ThreadPool.Size");
		    int sizeMax =
		        instance_.initializationData().properties.getPropertyAsInt(_name + ".ThreadPool.SizeMax");
		    if(size > 0 || sizeMax > 0)
		    {
			_threadPool = new IceInternal.ThreadPool(instance_, _name + ".ThreadPool", 0);
		    }
		}
	    }
	    catch(LocalException)
	    {
		deactivate();
		waitForDeactivate();
		throw;
	    }
	}
	
#if DEBUG
        ~ObjectAdapterI()
        {   
            lock(this)
            {
		if(!_deactivated)
		{
		    if(!Environment.HasShutdownStarted)
		    {
			instance_.initializationData().logger.warning("object adapter `" + _name +
								      "' has not been deactivated");
		    }
		    else
		    {
			Console.Error.WriteLine("object adapter `" + _name + "' has not been deactivated");
		    }
		}
		else if(instance_ != null)
		{
		    if(!Environment.HasShutdownStarted)
		    {
			instance_.initializationData().logger.warning("object adapter `" + _name +
			                           "' deactivation had not been waited for");
		    }
		    else
		    {
			Console.Error.WriteLine("object adapter `" + _name + "' deactivation had not been waited for");
		    }
		}
		else
		{
		    IceUtil.Assert.FinalizerAssert(_threadPool == null);
		    IceUtil.Assert.FinalizerAssert(_servantManager == null);
		    IceUtil.Assert.FinalizerAssert(_communicator == null);
		    IceUtil.Assert.FinalizerAssert(_incomingConnectionFactories == null);
		    IceUtil.Assert.FinalizerAssert(_directCount == 0);
		    IceUtil.Assert.FinalizerAssert(!_waitForDeactivate);
		}
            }   
        }
#endif          

	private ObjectPrx newProxy(Identity ident, string facet)
	{
	    if(_id.Length == 0)
	    {
		return newDirectProxy(ident, facet);
	    }
	    else if(_replicaGroupId.Length == 0)
	    {
		return newIndirectProxy(ident, facet, _id);
	    }
	    else
	    {
		return newIndirectProxy(ident, facet, _replicaGroupId);
	    }
	}
	
	private ObjectPrx newDirectProxy(Identity ident, string facet)
	{
	    IceInternal.EndpointI[] endpoints;

	    // 
	    // Use the published endpoints, otherwise use the endpoints from all
	    // incoming connection factories.
	    //
	    int sz = _publishedEndpoints.Count;
	    endpoints = new IceInternal.EndpointI[sz + _routerEndpoints.Count];
	    for(int i = 0; i < sz; ++i)
	    {
	        endpoints[i] = (IceInternal.EndpointI)_publishedEndpoints[i];
	    }

	    //
	    // Now we also add the endpoints of the router's server proxy, if
	    // any. This way, object references created by this object adapter
	    // will also point to the router's server proxy endpoints.
	    //
	    for(int i = 0; i < _routerEndpoints.Count; ++i)
	    {
		endpoints[sz + i] = (IceInternal.EndpointI)_routerEndpoints[i];
	    }
	    
	    //
	    // Create a reference and return a proxy for this reference.
	    //
	    IceInternal.Reference reference =
		instance_.referenceFactory().create(ident, instance_.initializationData().defaultContext, facet,
						    IceInternal.Reference.Mode.ModeTwoway, false, endpoints,
						    null, instance_.defaultsAndOverrides().
						    defaultCollocationOptimization);
	    return instance_.proxyFactory().referenceToProxy(reference);
	}
	
	private ObjectPrx newIndirectProxy(Identity ident, string facet, string id)
	{
	    //
	    // Create a reference with the adapter id and return a
	    // proxy for the reference.
	    //
	    IceInternal.Reference reference =
		instance_.referenceFactory().create(ident, instance_.initializationData().defaultContext, facet,
						    IceInternal.Reference.Mode.ModeTwoway, 
						    false, id, null, _locatorInfo, 
						    instance_.defaultsAndOverrides().defaultCollocationOptimization,
						    instance_.defaultsAndOverrides().defaultLocatorCacheTimeout);
	    return instance_.proxyFactory().referenceToProxy(reference);
	}

	private void checkForDeactivation()
	{
	    if(_deactivated)
	    {
		ObjectAdapterDeactivatedException ex = new ObjectAdapterDeactivatedException();
		ex.name = _name;
		throw ex;
	    }
	}
	
	private static void checkIdentity(Identity ident)
	{
	    if(ident.name == null || ident.name.Length == 0)
	    {
		IllegalIdentityException e = new IllegalIdentityException();
		e.id.name = ident.name;
		e.id.category = ident.category;
		throw e;
	    }	    
	    if(ident.category == null)
	    {
		ident.category = "";
	    }
	}

	private ArrayList parseEndpoints(string endpts)
	{
	    endpts = endpts.ToLower();

	    int beg;
	    int end = 0;

	    string delim = " \t\n\r";

	    ArrayList endpoints = new ArrayList();
	    while(end < endpts.Length)
	    {
		beg = IceUtil.StringUtil.findFirstNotOf(endpts, delim, end);
		if(beg == -1)
		{
		    break;
		}

		end = endpts.IndexOf((System.Char) ':', beg);
		if(end == -1)
		{
		    end = endpts.Length;
		}

		if(end == beg)
		{
		    ++end;
		    continue;
		}

		string s = endpts.Substring(beg, (end) - (beg));
		IceInternal.EndpointI endp = instance_.endpointFactoryManager().create(s);
		if(endp == null)
		{
		    Ice.EndpointParseException e2 = new Ice.EndpointParseException();
		    e2.str = s;
		    throw e2;
		}
		ArrayList endps = endp.expand(true);
		endpoints.AddRange(endps);

		++end;
	    }

	    return endpoints;
	}

	private sealed class ProcessI : ProcessDisp_
	{
	    public ProcessI(Communicator communicator)
	    {
		_communicator = communicator;
	    }

	    public override void shutdown(Ice.Current current)
	    {
		_communicator.shutdown();
	    }

	    public override void writeMessage(string message, int fd, Ice.Current current)
	    {
		switch(fd)
		{
		    case 1:
		    {
			System.Console.Out.WriteLine(message);
			break;
		    }
		    case 2:
		    {
			System.Console.Error.WriteLine(message);
			break;
		    }
		}
	    }	


	    private Communicator _communicator;
	}
	
	private bool _deactivated;
	private IceInternal.Instance instance_;
	private Communicator _communicator;
	private IceInternal.ObjectAdapterFactory _objectAdapterFactory;
	private IceInternal.ThreadPool _threadPool;
	private IceInternal.ServantManager _servantManager;
	private bool _printAdapterReadyDone;
	private readonly string _name;
	private readonly string _id;
	private readonly string _replicaGroupId;
	private ArrayList _incomingConnectionFactories;
	private ArrayList _routerEndpoints;
	private IceInternal.RouterInfo _routerInfo;
	private ArrayList _publishedEndpoints;
	private IceInternal.LocatorInfo _locatorInfo;
	private int _directCount;
	private bool _waitForDeactivate;
    }

}
