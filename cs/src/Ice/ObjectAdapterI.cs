// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Ice
{
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
	    Ice.LocatorRegistryPrx locatorRegistry = null;
	    bool registerProcess = false;
	    string serverId = "";
	    Communicator communicator = null;
	    bool printAdapterReady = false;

	    lock(this)
	    {
		checkForDeactivation();
		
		if(!_printAdapterReadyDone)
		{
		    if(_locatorInfo != null && _id.Length > 0)
		    {
		    	locatorRegistry = _locatorInfo.getLocatorRegistry();
		    }

		    registerProcess = _instance.properties().getPropertyAsInt(_name + ".RegisterProcess") > 0;
		    serverId = _instance.properties().getProperty("Ice.ServerId");
		    printAdapterReady = _instance.properties().getPropertyAsInt("Ice.PrintAdapterReady") > 0;

		    if(registerProcess && locatorRegistry == null)
		    {
		        _instance.logger().warning("object adapter `" + _name + "' cannot register the process " +
						   "without alocator registry");
			registerProcess = false;
		    }
		    else if(registerProcess && serverId.Length == 0)
		    {
		        _instance.logger().warning("object adapter `" + _name + "' cannot register the process " +
			                           "without a value for Ice.ServerId");
			registerProcess = false;
		    }

		    communicator = _communicator;
		    _printAdapterReadyDone = true;
		}
		
		foreach(IceInternal.IncomingConnectionFactory icf in _incomingConnectionFactories)
		{
		    icf.activate();
		}
	    }

	    //
	    // We must call on the locator registry oustide the thread
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
		try {
		     Identity ident = new Identity();
		     ident.category = "";
		     ident.name = "dummy";
		     locatorRegistry.setAdapterDirectProxy(_id, newDirectProxy(ident, ""));
		}
		catch(Ice.AdapterNotFoundException)
		{
		    NotRegisteredException ex1 = new NotRegisteredException();
		    ex1.id = _id;
		    ex1.kindOfObject = "object adapter";
		    throw ex1;
		}
		catch(Ice.AdapterAlreadyActiveException)
		{
		    ObjectAdapterIdInUseException ex1 = new ObjectAdapterIdInUseException();
		    ex1.id = _id;
		    throw ex1;
		}

		if(registerProcess)
		{
		    Process servant = new ProcessI(communicator);
		    ProcessPrx proxy = ProcessPrxHelper.uncheckedCast(addWithUUID(servant));

		    try
		    {
		    	locatorRegistry.setServerProcessProxy(serverId, proxy);
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
		
		int sz = _incomingConnectionFactories.Count;
		for(int i = 0; i < sz; ++i)
		{
		    IceInternal.IncomingConnectionFactory factory =
			(IceInternal.IncomingConnectionFactory)_incomingConnectionFactories[i];
		    factory.destroy();
		}
		
		_instance.outgoingConnectionFactory().removeAdapter(this);
		
		_deactivated = true;
		
		System.Threading.Monitor.PulseAll(this);
	    }
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
	    int sz = _incomingConnectionFactories.Count;
	    for(int i = 0; i < sz; ++i)
	    {
		IceInternal.IncomingConnectionFactory factory =
		    (IceInternal.IncomingConnectionFactory)_incomingConnectionFactories[i];
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
		_incomingConnectionFactories.Clear();
		
		//
		// Remove object references (some of them cyclic).
		//
		_instance = null;
		_threadPool = null;
		_servantManager = null;
		_communicator = null;
	    }
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

		IceInternal.Reference @ref = ((ObjectPrxHelperBase)proxy).__reference();
		return findFacet(@ref.identity, @ref.facet);
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
		IceInternal.Endpoint[] endpoints = new IceInternal.Endpoint[0];
		ConnectionI[] arr = new ConnectionI[connections.Count];
		if(arr.Length != 0)
		{
		    connections.CopyTo(arr, 0);
		}
		IceInternal.Reference @ref = _instance.referenceFactory().create(ident, new Context(), "",
										 IceInternal.Reference.ModeTwoway,
										 false, "", endpoints, null, null,
										 arr, true);
		return _instance.proxyFactory().referenceToProxy(@ref);
	    }
	}
	
	public void addRouter(RouterPrx router)
	{
	    lock(this)
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
		    IceInternal.Endpoint[] endpoints = proxy.__reference().endpoints;
		    for(int i = 0; i < endpoints.Length; ++i)
		    {
			_routerEndpoints.Add(endpoints[i]);
		    }
		    _routerEndpoints.Sort(); // Must be sorted.

		    //
		    // Remove duplicate endpoints, so we have a list of unique endpoints.
		    //
		    for(int i = 0; i < _routerEndpoints.Count - 1; ++i)
		    {
			System.Object o1 = _routerEndpoints[i];
			System.Object o2 = _routerEndpoints[i + 1];
			if(o1.Equals(o2))
			{
			    _routerEndpoints.Remove(i);
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
	}
	
	public void setLocator(LocatorPrx locator)
	{
	    lock(this)
	    {
		checkForDeactivation();
		
		_locatorInfo = _instance.locatorManager().get(locator);
	    }
	}

	public LocatorPrx getLocator()
	{
	    lock(this)
	    {
		checkForDeactivation();

		LocatorPrx locator = null;

		if(_locatorInfo != null)
		{
		    locator = _locatorInfo.getLocator();
		}

		return locator;
	    }
	}

	public bool isLocal(ObjectPrx proxy)
	{
	    lock(this)
	    {
		checkForDeactivation();
		
		IceInternal.Reference r = ((ObjectPrxHelperBase)proxy).__reference();
		IceInternal.Endpoint[] endpoints = r.endpoints;
		
		if(!r.adapterId.Equals(""))
		{
		    //
		    // Proxy is local if the reference adapter id matches this
		    // adapter name.
		    //
		    return r.adapterId.Equals(_id);
		}
		
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
		for(int i = 0; i < endpoints.Length; ++i)
		{
		    if(_routerEndpoints.BinarySearch(endpoints[i]) >= 0) // _routerEndpoints is sorted.
		    {
			return true;
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

	    foreach(IceInternal.IncomingConnectionFactory factory in _incomingConnectionFactories)
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
		
		Debug.Assert(_instance != null); // Must not be called after waitForDeactivate().
		
		Debug.Assert(_directCount > 0);
		if(--_directCount == 0)
		{
		    System.Threading.Monitor.PulseAll(this);
		}
	    }
	}
	
	public IceInternal.ThreadPool getThreadPool()
	{
	    // No mutex lock necessary, _threadPool and _instance are
	    // immutable after creation until they are removed in
	    // waitForDeactivate().
	    
	    // Not check for deactivation here!
	    
	    Debug.Assert(_instance != null); // Must not be called after waitForDeactivate().
	    
	    if(_threadPool != null)
	    {
		return _threadPool;
	    }
	    else
	    {
		return _instance.serverThreadPool();
	    }
	    
	}

	public IceInternal.ServantManager getServantManager()
	{
	    // No mutex lock necessary, _threadPool and _instance are
	    // immutable after creation until they are removed in
	    // waitForDeactivate().
	    
	    // Not check for deactivation here!
	    
	    Debug.Assert(_instance != null); // Must not be called after waitForDeactivate().
	    
	    return _servantManager;
	}
	
	//
	// Only for use by IceInternal.ObjectAdapterFactory
	//
	public ObjectAdapterI(IceInternal.Instance instance, Communicator communicator, string name)
	{
	    _deactivated = false;
	    _instance = instance;
	    _communicator = communicator;
	    _servantManager = new IceInternal.ServantManager(instance, name);
	    _printAdapterReadyDone = false;
	    _name = name;
	    _id = instance.properties().getProperty(name + ".AdapterId");
	    _logger = instance.logger();
	    _incomingConnectionFactories = new ArrayList();
	    _routerEndpoints = new ArrayList();
	    _directCount = 0;
	    _waitForDeactivate = false;
	    
	    try
	    {
		//
		// Parse the endpoints, but don't store them in the adapter.
		// The connection factory might change it, for example, to
		// fill in the real port number.
		//
		string endpts = _instance.properties().getProperty(name + ".Endpoints");
		ArrayList endpoints = parseEndpoints(endpts);
		for(int i = 0; i < endpoints.Count; ++i)
		{
		    IceInternal.Endpoint endp = (IceInternal.Endpoint)endpoints[i];
		    _incomingConnectionFactories.Add(new IceInternal.IncomingConnectionFactory(instance, endp, this));
		}
		
		//
		// Parse published endpoints. These are used in proxies
		// instead of the connection factory endpoints.
		//
		endpts = _instance.properties().getProperty(name + ".PublishedEndpoints");
		_publishedEndpoints = parseEndpoints(endpts);

		string router = _instance.properties().getProperty(name + ".Router");
		if(router.Length > 0)
		{
		    addRouter(RouterPrxHelper.uncheckedCast(_instance.proxyFactory().stringToProxy(router)));
		}
		
		string locator = _instance.properties().getProperty(name + ".Locator");
		if(locator.Length > 0)
		{
		    setLocator(LocatorPrxHelper.uncheckedCast(_instance.proxyFactory().stringToProxy(locator)));
		}
		else
		{
		    setLocator(_instance.referenceFactory().getDefaultLocator());
		}
		
		int size = _instance.properties().getPropertyAsInt(_name + ".ThreadPool.Size");
		int sizeMax = _instance.properties().getPropertyAsInt(_name + ".ThreadPool.SizeMax");
		if(size > 0 || sizeMax > 0)
		{
		    _threadPool = new IceInternal.ThreadPool(_instance, _name + ".ThreadPool", 0);
		}
	    }
	    catch(LocalException ex)
	    {
		deactivate();
		waitForDeactivate();
		throw ex;
	    }
	}
	
	~ObjectAdapterI()
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
		Debug.Assert(_threadPool == null);
		Debug.Assert(_servantManager == null);
		Debug.Assert(_communicator == null);
		Debug.Assert(_incomingConnectionFactories.Count == 0);
		Debug.Assert(_directCount == 0);
		Debug.Assert(!_waitForDeactivate);
	    }
	}
	
	private ObjectPrx newProxy(Identity ident, string facet)
	{
	    if(_id.Length == 0)
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
		ConnectionI[] connections = new ConnectionI[0];
		IceInternal.Reference reference =
		    _instance.referenceFactory().create(ident, new Context(), facet, IceInternal.Reference.ModeTwoway,
							false, _id, endpoints, null, _locatorInfo, connections, true);
		return _instance.proxyFactory().referenceToProxy(reference);
	    }
	}
	
	private ObjectPrx newDirectProxy(Identity ident, string facet)
	{
	    IceInternal.Endpoint[] endpoints;

	    // 
	    // Use the published endpoints, otherwise use the endpoints from all
	    // incoming connection factories.
	    //
	    int sz = _publishedEndpoints.Count;
	    if(sz > 0)
	    {
		endpoints = new IceInternal.Endpoint[sz + _routerEndpoints.Count];
		for(int i = 0; i < sz; ++i)
		{
		    endpoints[i] = (IceInternal.Endpoint)_publishedEndpoints[i];
		}
	    }
	    else
	    {
		sz = _incomingConnectionFactories.Count;
		endpoints = new IceInternal.Endpoint[sz + _routerEndpoints.Count];
		for(int i = 0; i < sz; ++i)
		{
		    IceInternal.IncomingConnectionFactory factory =
			(IceInternal.IncomingConnectionFactory)_incomingConnectionFactories[i];
		    endpoints[i] = factory.endpoint();
		}
	    }

	    //
	    // Now we also add the endpoints of the router's server proxy, if
	    // any. This way, object references created by this object adapter
	    // will also point to the router's server proxy endpoints.
	    //
	    for(int i = 0; i < _routerEndpoints.Count; ++i)
	    {
		endpoints[sz + i] = (IceInternal.Endpoint)_routerEndpoints[i];
	    }
	    
	    //
	    // Create a reference and return a proxy for this reference.
	    //
	    ConnectionI[] connections = new ConnectionI[0];
	    IceInternal.Reference reference =
		_instance.referenceFactory().create(ident, new Context(), facet, IceInternal.Reference.ModeTwoway,
                                                    false, "", endpoints, null, _locatorInfo, connections, true);
	    return _instance.proxyFactory().referenceToProxy(reference);
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
		IceInternal.Endpoint endp = _instance.endpointFactoryManager().create(s);
		endpoints.Add(endp);

		++end;
	    }

	    return endpoints;
	}

	private sealed class ProcessI : _ProcessDisp
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
	private IceInternal.Instance _instance;
	private Communicator _communicator;
	private IceInternal.ThreadPool _threadPool;
	private IceInternal.ServantManager _servantManager;
	private bool _printAdapterReadyDone;
	private readonly string _name;
	private readonly string _id;
	private Logger _logger;
	private ArrayList _incomingConnectionFactories;
	private ArrayList _routerEndpoints;
	private ArrayList _publishedEndpoints;
	private IceInternal.LocatorInfo _locatorInfo;
	private int _directCount;
	private bool _waitForDeactivate;
    }

}
