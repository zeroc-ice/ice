// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


namespace IceInternal
{

    using System.Collections;
    using System.Diagnostics;
    using System.Threading;
    using IceUtil;

    public class IncomingConnectionFactory : EventHandler
    {
	public virtual void activate()
	{
	    lock(this)
	    {
		setState(StateActive);
	    }
	}
	
	public virtual void hold()
	{
	    lock(this)
	    {
		setState(StateHolding);
	    }
	}
	
	public virtual void destroy()
	{
	    lock(this)
	    {
		setState(StateClosed);
	    }
	}
	
	public virtual void waitUntilHolding()
	{
	    LinkedList connections;
	    
	    lock(this)
	    {
		//
		// First we wait until the connection factory itself is in
		// holding state.
		//
		while(_state < StateHolding)
		{
		    System.Threading.Monitor.Wait(this);
		}
		
		//
		// We want to wait until all connections are in holding state
		// outside the thread synchronization.
		//
		connections = (LinkedList)_connections.Clone();
	    }
	    
	    //
	    // Now we wait until each connection is in holding state.
	    //
	    foreach(Ice.ConnectionI connection in connections)
	    {
		connection.waitUntilHolding();
	    }
	}
	
	public virtual void waitUntilFinished()
	{
	    LinkedList connections;
	    
	    lock(this)
	    {
		//
		// First we wait until the factory is destroyed.
		//
		while(_acceptor != null)
		{
		    System.Threading.Monitor.Wait(this);
		}
		
		//
		// We want to wait until all connections are finished
		// outside the thread synchronization.
		//
		connections = _connections;
		_connections = new LinkedList();
	    }
	    
	    //
	    // Now we wait for until the destruction of each connection is
	    // finished.
	    //
	    foreach(Ice.ConnectionI connection in connections)
	    {
		connection.waitUntilFinished();
	    }
	}
	
	public virtual Endpoint endpoint()
	{
	    // No mutex protection necessary, _endpoint is immutable.
	    return _endpoint;
	}
	
	public virtual bool equivalent(Endpoint endp)
	{
	    if(_transceiver != null)
	    {
		return endp.equivalent(_transceiver);
	    }
	    
	    Debug.Assert(_acceptor != null);
	    return endp.equivalent(_acceptor);
	}
	
	public virtual Ice.ConnectionI[] connections()
	{
	    lock(this)
	    {
		LinkedList connections = new LinkedList();
		
		//
		// Only copy connections which have not been destroyed.
		//
		foreach(Ice.ConnectionI connection in _connections)
		{
		    if(!connection.isDestroyed())
		    {
			connections.Add(connection);
		    }
		}
		Ice.ConnectionI[] arr = new Ice.ConnectionI[connections.Count];
		if(arr.Length != 0)
		{
		    connections.CopyTo(arr, 0);
		}
		return arr;
	    }
	}
	
	public virtual void flushBatchRequests()
	{
	    //
	    // connections() is synchronized, so no need to synchronize here.
	    //
	    foreach(Ice.ConnectionI connection in connections())
	    {
		try
		{
		    connection.flushBatchRequests();
		}
		catch(Ice.LocalException)
		{
		    // Ignore.
		}
	    }
	}
	
	//
	// Operations from EventHandler.
	//
	
	public override bool datagram()
	{
	    return _endpoint.datagram();
	}
	
	public override bool readable()
	{
	    return false;
	}
	
	public override void read(BasicStream unused)
	{
	    Debug.Assert(false); // Must not be called.
	}
	
	public override void message(BasicStream unused, ThreadPool threadPool)
	{
	    Ice.ConnectionI connection = null;
	    
	    lock(this)
	    {
		if(_state != StateActive)
		{
		    Thread.Sleep(0);
		    threadPool.promoteFollower();
		    return;
		}
		
		//
		// Reap connections for which destruction has completed.
		//
		LinkedList.Enumerator p = (LinkedList.Enumerator)_connections.GetEnumerator();
		while(p.MoveNext())
		{
		    Ice.ConnectionI con = (Ice.ConnectionI)p.Current;
		    if(con.isFinished())
		    {
			p.Remove();
		    }
		}
		
		//
		// Now accept a new connection.
		//
		Transceiver transceiver;
		try
		{
		    transceiver = _acceptor.accept(0);
		}
		catch(Ice.TimeoutException)
		{
		    // Ignore timeouts.
		    return;
		}
		catch(Ice.LocalException ex)
		{
		    // Warn about other Ice local exceptions.
		    if(_warn)
		    {
			    warning(ex);
		    }
		    return;
		}
		finally
		{
		    //
		    // We must promote a follower after we accepted a new
		    // connection, or after an exception.
		    //
		    threadPool.promoteFollower();
		}
		
		//
		// Create a connection object for the connection.
		//
		Debug.Assert(transceiver != null);
		connection = new Ice.ConnectionI(_instance, transceiver, _endpoint, _adapter);
		_connections.Add(connection);
	    }
	    
	    Debug.Assert(connection != null);
	    
	    //
	    // We validate and activate outside the thread
	    // synchronization, to not block the factory.
	    //
	    try
	    {
		connection.validate();
	    }
	    catch(Ice.LocalException)
	    {
		//
		// Ignore all exceptions while validating the
		// connection.  Warning or error messages for such
		// exceptions are printed directly by the validation
		// code.
		//
	    }
	    
	    connection.activate();
	}
	
	public override void finished(ThreadPool threadPool)
	{
	    lock(this)
	    {
		threadPool.promoteFollower();
		
		if(_state == StateActive)
		{
		    registerWithPool();
		}
		else if(_state == StateClosed)
		{
		    _acceptor.close();
		    _acceptor = null;
		    System.Threading.Monitor.PulseAll(this);
		}
	    }
	}
	
	public override void exception(Ice.LocalException ex)
	{
	    Debug.Assert(false); // Must not be called.
	}
	
	public override string ToString()
	{
	    if(_transceiver != null)
	    {
		return _transceiver.ToString();
	    }
	    
	    Debug.Assert(_acceptor != null);
	    return _acceptor.ToString();
	}
	
	public IncomingConnectionFactory(Instance instance, Endpoint endpoint, Ice.ObjectAdapter adapter)
	    : base(instance)
	{
	    _endpoint = endpoint;
	    _adapter = adapter;
	    _registeredWithPool = false;
	    _warn = _instance.properties().getPropertyAsInt("Ice.Warn.Connections") > 0?true:false;
	    _connections = new LinkedList();
	    _state = StateHolding;
	    
	    DefaultsAndOverrides defaultsAndOverrides = _instance.defaultsAndOverrides();

	    if(defaultsAndOverrides.overrideTimeout)
	    {
		_endpoint = _endpoint.timeout(defaultsAndOverrides.overrideTimeoutValue);
	    }

	    if(defaultsAndOverrides.overrideCompress)
	    {
	        _endpoint = _endpoint.compress(defaultsAndOverrides.overrideCompressValue);
	    }
	    
	    try
	    {
		Endpoint h = _endpoint;
		_transceiver = _endpoint.serverTransceiver(ref h);
		if(_transceiver != null)
		{
		    _endpoint = h;
		    Ice.ConnectionI connection = new Ice.ConnectionI(_instance, _transceiver, _endpoint, _adapter);
		    connection.validate();
		    _connections.Add(connection);
		}
		else
		{
		    h = _endpoint;
		    _acceptor = _endpoint.acceptor(ref h);
		    _endpoint = h;
		    Debug.Assert(_acceptor != null);
		    _acceptor.listen();
		}
	    }
	    catch(System.Exception ex)
	    {
		_state = StateClosed;
		_acceptor = null;
		throw ex;
	    }
	}
	
	~IncomingConnectionFactory()
	{
	    Debug.Assert(_state == StateClosed);
	    Debug.Assert(_acceptor == null);
	    Debug.Assert(_connections.Count == 0);
	}
	
	private const int StateActive = 0;
	private const int StateHolding = 1;
	private const int StateClosed = 2;
	
	private void setState(int state)
	{
	    if(_state == state)
	    // Don't switch twice.
	    {
		return;
	    }
	    
	    switch (state)
	    {
		case StateActive: 
		{
		    if(_state != StateHolding)
		    // Can only switch from holding to active.
		    {
			return;
		    }
		    registerWithPool();
		    
		    foreach(Ice.ConnectionI connection in _connections)
		    {
			connection.activate();
		    }
		    break;
		}
		
		case StateHolding: 
		{
		    if(_state != StateActive)
		    // Can only switch from active to holding.
		    {
			return;
		    }
		    unregisterWithPool();
		    
		    foreach(Ice.ConnectionI connection in _connections)
		    {
			connection.hold();
		    }
		    break;
		}
		
		case StateClosed: 
		{
		    //
		    // If we come from holding state, we first need to
		    // register again before we unregister.
		    //
		    if(_state == StateHolding)
		    {
			registerWithPool();
		    }
		    unregisterWithPool();
		    
		    foreach(Ice.ConnectionI connection in _connections)
		    {
			connection.destroy(Ice.ConnectionI.ObjectAdapterDeactivated);
		    }
		    break;
		}
	    }
	    
	    _state = state;
	    System.Threading.Monitor.PulseAll(this);
	}

	private void registerWithPool()
	{
	    if(_acceptor != null && !_registeredWithPool)
	    {
		((Ice.ObjectAdapterI) _adapter).getThreadPool().register(_acceptor.fd(), this);
		_registeredWithPool = true;
	    }
	}
	
	private void unregisterWithPool()
	{
	    if(_acceptor != null && _registeredWithPool)
	    {
		((Ice.ObjectAdapterI) _adapter).getThreadPool().unregister(_acceptor.fd());
		_registeredWithPool = false;
	    }
	}
	
	private void warning(Ice.LocalException ex)
	{
	    _instance.logger().warning("connection exception:\n" + ex + '\n' + _acceptor.ToString());
	}
	
	private Acceptor _acceptor;
	private readonly Transceiver _transceiver;
	private Endpoint _endpoint;
	
	private readonly Ice.ObjectAdapter _adapter;
	
	private bool _registeredWithPool;
	
	private readonly bool _warn;
	
	private LinkedList _connections;
	
	private int _state;
    }

    public class OutgoingConnectionFactory
    {
	public virtual void destroy()
	{
	    lock(this)
	    {
		if(_destroyed)
		{
		    return;
		}
		
		foreach(LinkedList connections in _connections.Values)
		{
		    foreach(Ice.ConnectionI c in connections)
		    {
			c.destroy(Ice.ConnectionI.CommunicatorDestroyed);
		    }
		}

		_destroyed = true;
		System.Threading.Monitor.PulseAll(this);
	    }
	}
	
	public virtual void waitUntilFinished()
	{
	    Hashtable connections;
	    
	    lock(this)
	    {
		//
		// First we wait until the factory is destroyed. We also
		// wait until there are no pending connections
		// anymore. Only then we can be sure the _connections
		// contains all connections.
		//
		while(!_destroyed || _pending.Count != 0)
		{
		    System.Threading.Monitor.Wait(this);
		}
		
		//
		// We want to wait until all connections are finished
		// outside the thread synchronization.
		//
		connections = _connections;
		_connections = new Hashtable();
	    }
	    
	    //
	    // Now we wait for the destruction of each connection to be
	    // finished.
	    //
	    foreach(LinkedList cl in connections.Values)
	    {
		foreach(Ice.ConnectionI c in cl)
		{
		    c.waitUntilFinished();
		}
	    }
	}
	
	public virtual Ice.ConnectionI create(Endpoint[] endpoints, out bool compress)
	{
	    Debug.Assert(endpoints.Length > 0);
	    
            compress = false;

	    lock(this)
	    {
		if(_destroyed)
		{
		    throw new Ice.CommunicatorDestroyedException();
		}
		
		//
		// Reap connections for which destruction has completed.
		//
		ArrayList removeList = new ArrayList();
		foreach(DictionaryEntry e in _connections)
		{
		    LinkedList cl = (LinkedList)e.Value;
		    LinkedList.Enumerator q = (LinkedList.Enumerator)cl.GetEnumerator();
		    while(q.MoveNext())
		    {
			if(((Ice.ConnectionI)q.Current).isFinished())
			{
			    q.Remove();
			}
		    }
		    if(cl.Count == 0)
		    {
			removeList.Add(e.Key);
		    }
		}
		foreach(object o in removeList)
		{
		    _connections.Remove(o);
		}
		
		//
		// Modify endpoints with overrides.
		//
		DefaultsAndOverrides defaultsAndOverrides = _instance.defaultsAndOverrides();
		for(int j = 0; j < endpoints.Length; j++)
		{
		    if(defaultsAndOverrides.overrideTimeout)
		    {
			endpoints[j] = endpoints[j].timeout(defaultsAndOverrides.overrideTimeoutValue);
		    }

                    //
		    // Do not clear the compression flag here -- we need it to set the `compress' out
		    // parameter below.
		    //
		}
		
		//
		// Search for existing connections.
		//
		for(int j = 0; j < endpoints.Length; j++)
		{
		    LinkedList connectionList = (LinkedList)_connections[endpoints[j]];
		    if(connectionList != null)
		    {
			foreach(Ice.ConnectionI connection in connectionList)
			{
                            //
                            // Don't return connections for which destruction has
                            // been initiated.
                            //
			    if(!connection.isDestroyed())
			    {
                                if(_instance.defaultsAndOverrides().overrideCompress)
                                {
                                    compress = _instance.defaultsAndOverrides().overrideCompressValue;
                                }
                                else
                                {
                                    compress = endpoints[j].compress();
                                }
				return connection;
			    }
			}
		    }
		}
		
		//
		// If some other thread is currently trying to establish a
		// connection to any of our endpoints, we wait until this
		// thread is finished.
		//
		bool searchAgain = false;
		while(!_destroyed)
		{
		    int j;
		    for(j = 0; j < endpoints.Length; j++)
		    {
			if(_pending.Contains(endpoints[j]))
			{
			    break;
			}
		    }
		    
		    if(j == endpoints.Length)
		    {
			break;
		    }
		    
		    searchAgain = true;
		    
		    System.Threading.Monitor.Wait(this);
		}
		
		if(_destroyed)
		{
		    throw new Ice.CommunicatorDestroyedException();
		}
		
		//
		// Search for existing connections again if we waited
		// above, as new connections might have been added in the
		// meantime.
		//
		if(searchAgain)
		{
		    for(int j = 0; j < endpoints.Length; j++)
		    {
			LinkedList connectionList = (LinkedList)_connections[endpoints[j]];
			if(connectionList != null)
			{
                            foreach(Ice.ConnectionI connection in connectionList)
                            {
                                //
                                // Don't return connections for which destruction has
                                // been initiated.
                                //
                                if(!connection.isDestroyed())
                                {
                                    if(_instance.defaultsAndOverrides().overrideCompress)
                                    {
                                        compress = _instance.defaultsAndOverrides().overrideCompressValue;
                                    }
                                    else
                                    {
                                        compress = endpoints[j].compress();
                                    }
                                    return connection;
                                }
                            }
			}
		    }
		}
		
		//
		// No connection to any of our endpoints exists yet, so we
		// will try to create one. To avoid that other threads try
		// to create connections to the same endpoints, we add our
		// endpoints to _pending.
		//
		foreach(Endpoint e in endpoints)
		{
		    _pending.Add(e);
		}
	    }
	    
	    Ice.ConnectionI newConnection = null;
	    Ice.LocalException exception = null;
	    
	    for(int i = 0; i < endpoints.Length; i++)
	    {
		Endpoint endpoint = endpoints[i];
		
		try
		{
		    Transceiver transceiver = endpoint.clientTransceiver();
		    if(transceiver == null)
		    {
			Connector connector = endpoint.connector();
			Debug.Assert(connector != null);

			int timeout;
			DefaultsAndOverrides defaultsAndOverrides = _instance.defaultsAndOverrides();
			if(defaultsAndOverrides.overrideConnectTimeout)
			{
			    timeout = defaultsAndOverrides.overrideConnectTimeoutValue;
			}
			// It is not necessary to check for overrideTimeout,
			// the endpoint has already been modified with this
			// override, if set.
			else
			{
			    timeout = endpoint.timeout();
			}

			transceiver = connector.connect(timeout);
			Debug.Assert(transceiver != null);
		    }
		    newConnection = new Ice.ConnectionI(_instance, transceiver, endpoint, null);
		    newConnection.validate();
                    if(_instance.defaultsAndOverrides().overrideCompress)
                    {
                        compress = _instance.defaultsAndOverrides().overrideCompressValue;
                    }
                    else
                    {
                        compress = endpoint.compress();
                    }
		    break;
		}
		catch(Ice.LocalException ex)
		{
		    exception = ex;
		}
		
		TraceLevels traceLevels = _instance.traceLevels();
		if(traceLevels.retry >= 2)
		{
		    System.Text.StringBuilder s = new System.Text.StringBuilder();
		    s.Append("connection to endpoint failed");
		    if(i < endpoints.Length - 1)
		    {
			s.Append(", trying next endpoint\n");
		    }
		    else
		    {
			s.Append(" and no more endpoints to try\n");
		    }
		    s.Append(exception);
		    _instance.logger().trace(traceLevels.retryCat, s.ToString());
		}
	    }
	    
	    lock(this)
	    {
		//
		// Signal other threads that we are done with trying to
		// establish connections to our endpoints.
		//
		for(int i = 0; i < endpoints.Length; i++)
		{
		    _pending.Remove(endpoints[i]);
		}
		System.Threading.Monitor.PulseAll(this);
		
		if(newConnection == null)
		{
		    Debug.Assert(exception != null);
		    throw exception;
		}
		else
		{
		    LinkedList connectionList = (LinkedList)_connections[newConnection.endpoint()];
		    if(connectionList == null)
		    {
			connectionList = new LinkedList();
			_connections[newConnection.endpoint()] = connectionList;
		    }
		    connectionList.Add(newConnection);
		    
		    if(_destroyed)
		    {
			newConnection.destroy(Ice.ConnectionI.CommunicatorDestroyed);
			throw new Ice.CommunicatorDestroyedException();
		    }
		    else
		    {
			newConnection.activate();
		    }
		}
	    }
	    
	    Debug.Assert(newConnection != null);
	    return newConnection;
	}
	
	virtual public void setRouter(Ice.RouterPrx router)
	{
	    lock(this)
	    {
		if(_destroyed)
		{
		    throw new Ice.CommunicatorDestroyedException();
		}
		
		RouterInfo routerInfo = _instance.routerManager().get(router);
		if(routerInfo != null)
		{
		    //
		    // Search for connections to the router's client proxy
		    // endpoints, and update the object adapter for such
		    // connections, so that callbacks from the router can be
		    // received over such connections.
		    //
		    Ice.ObjectPrx proxy = routerInfo.getClientProxy();
		    Ice.ObjectAdapter adapter = routerInfo.getAdapter();
		    DefaultsAndOverrides defaultsAndOverrides = _instance.defaultsAndOverrides();
		    Endpoint[] endpoints = ((Ice.ObjectPrxHelperBase)proxy).__reference().getEndpoints();
		    for(int i = 0; i < endpoints.Length; i++)
		    {
			Endpoint endpoint = endpoints[i];
			if(defaultsAndOverrides.overrideTimeout)
			{
			    endpoint = endpoint.timeout(defaultsAndOverrides.overrideTimeoutValue);
			}

			//
                        // The Ice.ConnectionI object does not take the compression flag of
                        // endpoints into account, but instead gets the information
                        // about whether messages should be compressed or not from
                        // other sources. In order to allow connection sharing for
                        // endpoints that differ in the value of the compression flag
                        // only, we always set the compression flag to false here in
                        // this connection factory.
                        //
                        endpoint = endpoint.compress(false);
			
			LinkedList connectionList = (LinkedList)_connections[endpoints[i]];
			if(connectionList != null)
			{
			    foreach(Ice.ConnectionI connection in connectionList)
			    {
			        try
				{
				    connection.setAdapter(adapter);
				}
				catch(Ice.LocalException)
				{
				    //
				    // Ignore, the connection is being closed or closed.
				    //
				}
			    }
			}
		    }
		}
	    }
	}

	public virtual void removeAdapter(Ice.ObjectAdapter adapter)
	{
	    lock(this)
	    {
		if(_destroyed)
		{
		    throw new Ice.CommunicatorDestroyedException();
		}
		
		foreach(LinkedList connectionList in _connections.Values)
		{
		    foreach(Ice.ConnectionI connection in connectionList)
		    {
			if(connection.getAdapter() == adapter)
			{
			    try
			    {
			        connection.setAdapter(null);
			    }
			    catch(Ice.LocalException)
			    {
			        //
			        // Ignore, the connection is being closed or closed.
			        //
			    }
			}
		    }
		}
	    }
	}
	
	public virtual void flushBatchRequests()
	{
	    LinkedList c = new LinkedList();
	    
	    lock(this)
	    {
		foreach(LinkedList connectionList in _connections.Values)
		{
		    foreach(Ice.ConnectionI conn in connectionList)
		    {
			c.Add(conn);
		    }
		}
	    }
	    
	    foreach(Ice.ConnectionI conn in c)
	    {
		if(conn.isValidated())
		{
		    try
		    {
			conn.flushBatchRequests();
		    }
		    catch(Ice.LocalException)
		    {
			// Ignore.
		    }
		}
	    }
	}
	
	//
	// Only for use by Instance.
	//
	internal OutgoingConnectionFactory(Instance instance)
	{
	    _instance = instance;
	    _destroyed = false;
	    _connections = new Hashtable();
	    _pending = new Set();
	}
	
	~OutgoingConnectionFactory()
	{
	    Debug.Assert(_destroyed);
	    Debug.Assert(_connections.Count == 0);
	}
	
	private readonly Instance _instance;
	private bool _destroyed;
	private Hashtable _connections;
	private Set _pending;
    }

}
