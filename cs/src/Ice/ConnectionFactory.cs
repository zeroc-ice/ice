// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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

    public sealed class OutgoingConnectionFactory
    {
	public void destroy()
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
	
	public void waitUntilFinished()
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
		// We set _connections to null because our destructor must not
		// invoke methods on member objects.
		//
		connections = _connections;
		_connections = null;
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
	
	public Ice.ConnectionI create(EndpointI[] endpts, out bool compress)
	{
	    Debug.Assert(endpts.Length > 0);
	    EndpointI[] endpoints = new EndpointI[endpts.Length];
	    for(int i = 0; i < endpoints.Length; ++i)
	    {
		endpoints[i] = endpts[i];
	    }
	    DefaultsAndOverrides defaultsAndOverrides = instance_.defaultsAndOverrides();
	    
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
		for(int i = 0; i < endpoints.Length; i++)
		{
		    if(defaultsAndOverrides.overrideTimeout)
		    {
			endpoints[i] = endpoints[i].timeout(defaultsAndOverrides.overrideTimeoutValue);
		    }


		    //
		    // The Connection object does not take the
		    // compression flag of endpoints into account, but
		    // instead gets the information about whether
		    // messages should be compressed or not from other
		    // sources. In order to allow connection sharing
		    // for endpoints that differ in the value of the
		    // compression flag only, we always set the
		    // compression flag to false here in this
		    // connection factory.
		    //
		    endpoints[i] = endpoints[i].compress(false);
		}
		
		//
		// Search for existing connections.
		//
		for(int i = 0; i < endpoints.Length; i++)
		{
		    LinkedList connectionList = (LinkedList)_connections[endpoints[i]];
		    if(connectionList != null)
		    {
			foreach(Ice.ConnectionI conn in connectionList)
			{
                            //
                            // Don't return connections for which destruction has
                            // been initiated.
                            //
			    if(!conn.isDestroyed())
			    {
                                if(defaultsAndOverrides.overrideCompress)
                                {
                                    compress = defaultsAndOverrides.overrideCompressValue;
                                }
                                else
                                {
                                    compress = endpts[i].compress();
                                }
				return conn;
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
		    int i;
		    for(i = 0; i < endpoints.Length; i++)
		    {
			if(_pending.Contains(endpoints[i]))
			{
			    break;
			}
		    }
		    
		    if(i == endpoints.Length)
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
		    for(int i = 0; i < endpoints.Length; i++)
		    {
			LinkedList connectionList = (LinkedList)_connections[endpoints[i]];
			if(connectionList != null)
			{
                            foreach(Ice.ConnectionI conn in connectionList)
                            {
                                //
                                // Don't return connections for which destruction has
                                // been initiated.
                                //
                                if(!conn.isDestroyed())
                                {
                                    if(defaultsAndOverrides.overrideCompress)
                                    {
                                        compress = defaultsAndOverrides.overrideCompressValue;
                                    }
                                    else
                                    {
                                        compress = endpts[i].compress();
                                    }

                                    return conn;
                                }
                            }
			}
		    }
		}
		
		//
		// No connection to any of our endpoints exists yet,
		// so we will try to create one. To avoid that other
		// threads try to create connections to the same
		// endpoints, we add our endpoints to _pending.
		//
		foreach(EndpointI e in endpoints)
		{
		    _pending.Add(e);
		}
	    }
	    
	    Ice.ConnectionI connection = null;
	    Ice.LocalException exception = null;
	    
	    for(int i = 0; i < endpoints.Length; i++)
	    {
		EndpointI endpoint = endpoints[i];
		
		try
		{
		    Transceiver transceiver = endpoint.clientTransceiver();
		    if(transceiver == null)
		    {
			Connector connector = endpoint.connector();
			Debug.Assert(connector != null);

			int timeout;
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
		    connection = new Ice.ConnectionI(instance_, transceiver, endpoint, null);
		    connection.validate();

                    if(defaultsAndOverrides.overrideCompress)
                    {
                        compress = defaultsAndOverrides.overrideCompressValue;
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

		    //
		    // If a connection object was constructed, then validate()
		    // must have raised the exception.
		    //
		    if(connection != null)
		    {
			connection.waitUntilFinished(); // We must call waitUntilFinished() for cleanup.
			connection = null;
		    }
		}
		
		TraceLevels traceLevels = instance_.traceLevels();
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
		    instance_.logger().trace(traceLevels.retryCat, s.ToString());
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
		
		if(connection == null)
		{
		    Debug.Assert(exception != null);
		    throw exception;
		}
		else
		{
		    LinkedList connectionList = (LinkedList)_connections[connection.endpoint()];
		    if(connectionList == null)
		    {
			connectionList = new LinkedList();
			_connections[connection.endpoint()] = connectionList;
		    }
		    connectionList.Add(connection);
		    
		    if(_destroyed)
		    {
			connection.destroy(Ice.ConnectionI.CommunicatorDestroyed);
			throw new Ice.CommunicatorDestroyedException();
		    }
		    else
		    {
			connection.activate();
		    }
		}
	    }
	    
	    Debug.Assert(connection != null);
	    return connection;
	}
	
	public void setRouterInfo(IceInternal.RouterInfo routerInfo)
	{
	    lock(this)
	    {
		if(_destroyed)
		{
		    throw new Ice.CommunicatorDestroyedException();
		}
		
		Debug.Assert(routerInfo != null);
		//
		// Search for connections to the router's client proxy
		// endpoints, and update the object adapter for such
		// connections, so that callbacks from the router can be
		// received over such connections.
		//
		Ice.ObjectPrx proxy = routerInfo.getClientProxy();
		Ice.ObjectAdapter adapter = routerInfo.getAdapter();
		DefaultsAndOverrides defaultsAndOverrides = instance_.defaultsAndOverrides();
		EndpointI[] endpoints = ((Ice.ObjectPrxHelperBase)proxy).reference__().getEndpoints();
		for(int i = 0; i < endpoints.Length; i++)
		{
		    EndpointI endpoint = endpoints[i];

		    //
		    // Modify endpoints with overrides.
		    //
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

	public void removeAdapter(Ice.ObjectAdapter adapter)
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
	
	public void flushBatchRequests()
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
	
	//
	// Only for use by Instance.
	//
	internal OutgoingConnectionFactory(Instance instance)
	{
	    instance_ = instance;
	    _destroyed = false;
	    _connections = new Hashtable();
	    _pending = new Set();
	}
	
#if DEBUG
	~OutgoingConnectionFactory()
	{
	    lock(this)
	    {
		IceUtil.Assert.FinalizerAssert(_destroyed);
		IceUtil.Assert.FinalizerAssert(_connections == null);
	    }
	}
#endif
	
	private readonly Instance instance_;
	private bool _destroyed;
	private Hashtable _connections;
	private Set _pending;
    }

    public sealed class IncomingConnectionFactory : EventHandler
    {
	public void activate()
	{
	    lock(this)
	    {
		setState(StateActive);
	    }
	}
	
	public void hold()
	{
	    lock(this)
	    {
		setState(StateHolding);
	    }
	}
	
	public void destroy()
	{
	    lock(this)
	    {
		setState(StateClosed);
	    }
	}
	
	public void waitUntilHolding()
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
	
	public void waitUntilFinished()
	{
	    Thread threadPerIncomingConnectionFactory = null;
	    LinkedList connections;
	    
	    lock(this)
	    {
		//
		// First we wait until the factory is destroyed. If we are using
		// an acceptor, we also wait for it to be closed.
		//
		while(_state != StateClosed || _acceptor != null)
		{
		    System.Threading.Monitor.Wait(this);
		}

		threadPerIncomingConnectionFactory = _threadPerIncomingConnectionFactory;
		_threadPerIncomingConnectionFactory = null;

		//
		// We want to wait until all connections are finished
		// outside the thread synchronization.
		//
		// We set _connections to null because our destructor must not
		// invoke methods on member objects.
		//
		connections = _connections;
		_connections = null;
	    }

	    if(threadPerIncomingConnectionFactory != null)
	    {
		threadPerIncomingConnectionFactory.Join();
	    }

	    //
	    // Now we wait until the destruction of each connection is finished.
	    //
	    foreach(Ice.ConnectionI connection in connections)
	    {
		connection.waitUntilFinished();
	    }
	}
	
	public EndpointI endpoint()
	{
	    // No mutex protection necessary, _endpoint is immutable.
	    return _endpoint;
	}
	
	public bool equivalent(EndpointI endp)
	{
	    if(_transceiver != null)
	    {
		return endp.equivalent(_transceiver);
	    }
	    
	    Debug.Assert(_acceptor != null);
	    return endp.equivalent(_acceptor);
	}
	
	public Ice.ConnectionI[] connections()
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
	
	public void flushBatchRequests()
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
	    Debug.Assert(!instance_.threadPerConnection()); // Only for use with a thread pool.
	    return _endpoint.datagram();
	}
	
	public override bool readable()
	{
	    Debug.Assert(!instance_.threadPerConnection()); // Only for use with a thread pool.
	    return false;
	}
	
	public override void read(BasicStream unused)
	{
	    Debug.Assert(!instance_.threadPerConnection()); // Only for use with a thread pool.
	    Debug.Assert(false); // Must not be called.
	}
	
	public override void message(BasicStream unused, ThreadPool threadPool)
	{
	    Debug.Assert(!instance_.threadPerConnection()); // Only for use with a thread pool.

	    Ice.ConnectionI connection = null;
	    
	    lock(this)
	    {
		try
		{
		    if(_state != StateActive)
		    {
			Thread.Sleep(0);
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
		
		    Debug.Assert(transceiver != null);

		    try
		    {
			connection = new Ice.ConnectionI(instance_, transceiver, _endpoint, _adapter);
		    }
		    catch(Ice.LocalException)
		    {
			return;
		    }
		    
		    _connections.Add(connection);
		}
		finally
		{
		    //
		    // This makes sure that we promote a follower before
		    // we leave the scope of the mutex above, but after we
		    // call accept() (if we call it).
		    //
		    threadPool.promoteFollower();
		}
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
		lock(this)
		{
		    connection.waitUntilFinished(); // We must call waitUntilFinished() for cleanup.
		    LinkedList.Enumerator p = (LinkedList.Enumerator)_connections.GetEnumerator();
		    while(p.MoveNext())
		    {
			if((Ice.ConnectionI)p.Current == connection)
			{
			    p.Remove();
			    break;
			}
		    }
		    return;
		}
	    }
	    
	    connection.activate();
	}
	
	public override void finished(ThreadPool threadPool)
	{
	    Debug.Assert(!instance_.threadPerConnection()); // Only for use with a thread pool.

	    lock(this)
	    {
		threadPool.promoteFollower();
		
		--_finishedCount;

		if(_finishedCount == 0 && _state == StateClosed)
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
	
	public IncomingConnectionFactory(Instance instance, EndpointI endpoint, Ice.ObjectAdapter adapter)
	    : base(instance)
	{
	    _endpoint = endpoint;
	    _adapter = adapter;
	    _registeredWithPool = false;
	    _finishedCount = 0;
	    _warn = instance_.properties().getPropertyAsInt("Ice.Warn.Connections") > 0 ? true : false;
	    _connections = new LinkedList();
	    _state = StateHolding;
	    
	    DefaultsAndOverrides defaultsAndOverrides = instance_.defaultsAndOverrides();

	    if(defaultsAndOverrides.overrideTimeout)
	    {
		_endpoint = _endpoint.timeout(defaultsAndOverrides.overrideTimeoutValue);
	    }

	    if(defaultsAndOverrides.overrideCompress)
	    {
	        _endpoint = _endpoint.compress(defaultsAndOverrides.overrideCompressValue);
	    }
	    
	    EndpointI h = _endpoint;
	    _transceiver = _endpoint.serverTransceiver(ref h);

	    try
	    {
		if(_transceiver != null)
		{
		    _endpoint = h;
		    
		    Ice.ConnectionI connection = null;
		    
		    try
		    {
			connection = new Ice.ConnectionI(instance_, _transceiver, _endpoint, _adapter);
			connection.validate();
		    }
		    catch(Ice.LocalException)
		    {
			//
			// If a connection object was constructed, then
			// validate() must have raised the exception.
			//
			if(connection != null)
			{
			    connection.waitUntilFinished(); // We must call waitUntilFinished() for cleanup.
			}
			
			return;
		    }
		    
		    _connections.Add(connection);
		}
		else
		{
		    h = _endpoint;
		    _acceptor = _endpoint.acceptor(ref h);
		    _endpoint = h;
		    Debug.Assert(_acceptor != null);
		    _acceptor.listen();

		    if(instance_.threadPerConnection())
		    {
			try
			{
			    //
			    // If we are in thread per connection mode, we also use
			    // one thread per incoming connection factory, that
			    // accepts new connections on this endpoint.
			    //
			    _threadPerIncomingConnectionFactory =
				new Thread(new ThreadStart(ThreadPerIncomingConnectionFactory));
			    _threadPerIncomingConnectionFactory.Start();
			}
			catch(System.Exception ex)
			{
			    instance_.logger().error("cannot create thread for incoming connection factory:\n" + ex);
			    throw;
			}
		    }
		}
	    }
	    catch(System.Exception ex)
	    {
		//
		// Clean up for finalizer.
		//
		
		if(_acceptor != null)
		{
		    try
		    {
			_acceptor.close();
		    }
		    catch(Ice.LocalException)
		    {
			// Here we ignore any exceptions in close().			
		    }
		}

		lock(this)
		{
		    _state = StateClosed;
		    _acceptor = null;
		    _connections = null;
		    _threadPerIncomingConnectionFactory = null;
		}

		throw new Ice.SyscallException(ex);
	    }
	}
	
#if DEBUG
	~IncomingConnectionFactory()
	{
	    lock(this)
	    {
		IceUtil.Assert.FinalizerAssert(_state == StateClosed);
		IceUtil.Assert.FinalizerAssert(_acceptor == null);
		IceUtil.Assert.FinalizerAssert(_connections == null);
		IceUtil.Assert.FinalizerAssert(_threadPerIncomingConnectionFactory == null);
	    }
	}
#endif
	
	private const int StateActive = 0;
	private const int StateHolding = 1;
	private const int StateClosed = 2;
	
	private void setState(int state)
	{
	    if(_state == state) // Don't switch twice.
	    {
		return;
	    }
	    
	    switch (state)
	    {
		case StateActive: 
		{
		    if(_state != StateHolding) // Can only switch from holding to active.
		    {
			return;
		    }
		    if(!instance_.threadPerConnection() && _acceptor != null)
		    {
			registerWithPool();
		    }
		    
		    foreach(Ice.ConnectionI connection in _connections)
		    {
			connection.activate();
		    }
		    break;
		}
		
		case StateHolding: 
		{
		    if(_state != StateActive) // Can only switch from active to holding.
		    {
			return;
		    }
		    if(!instance_.threadPerConnection() && _acceptor != null)
		    {
			unregisterWithPool();
		    }
		    
		    foreach(Ice.ConnectionI connection in _connections)
		    {
			connection.hold();
		    }
		    break;
		}
		
		case StateClosed: 
		{
		    if(_acceptor != null)
		    {
		        if(instance_.threadPerConnection())
		        {
			    //
			    // If we are in thread per connection mode, we connect
			    // to our own acceptor, which unblocks our thread per
			    // incoming connection factory stuck in accept().
			    //
			    _acceptor.connectToSelf();
		        }
		        else
		        {
			    //
			    // Otherwise we first must make sure that we are
			    // registered, then we unregister, and let finished()
			    // do the close.
			    //
			    registerWithPool();
			    unregisterWithPool();
		        }
		    }
		    
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
	    Debug.Assert(!instance_.threadPerConnection()); // Only for use with a thread pool.
	    Debug.Assert(_acceptor != null);

	    if(!_registeredWithPool)
	    {
		((Ice.ObjectAdapterI) _adapter).getThreadPool().register(_acceptor.fd(), this);
		_registeredWithPool = true;
	    }
	}
	
	private void unregisterWithPool()
	{
	    Debug.Assert(!instance_.threadPerConnection()); // Only for use with a thread pool.
	    Debug.Assert(_acceptor != null);

	    if(_registeredWithPool)
	    {
		((Ice.ObjectAdapterI) _adapter).getThreadPool().unregister(_acceptor.fd());
		_registeredWithPool = false;
		++_finishedCount; // For each unregistration, finished() is called once.
	    }
	}
	
	private void warning(Ice.LocalException ex)
	{
	    instance_.logger().warning("connection exception:\n" + ex + '\n' + _acceptor.ToString());
	}

	private void run()
	{
	    Debug.Assert(_acceptor != null);

	    while(true)
	    {
		//
		// We must accept new connections outside the thread
		// synchronization, because we use blocking accept.
		//
		Transceiver transceiver = null;
		try
		{
		    transceiver = _acceptor.accept(-1);
		}
		catch(Ice.TimeoutException)
		{
		    // Ignore timeouts.
		}
		catch(Ice.LocalException ex)
		{
		    // Warn about other Ice local exceptions.
		    if(_warn)
		    {
			warning(ex);
		    }
		}

		Ice.ConnectionI connection = null;

		lock(this)
		{
		    while(_state == StateHolding)
		    {
			Monitor.Wait(this);
		    }

		    if(_state == StateClosed)
		    {
			if(transceiver != null)
			{
			    try
			    {
				transceiver.close();
			    }
			    catch(Ice.LocalException)
			    {
				// Here we ignore any exceptions in close().
			    }
			}

			try
			{
			    _acceptor.close();
			}
			catch(Ice.LocalException)
			{
			    _acceptor = null;
			    Monitor.PulseAll(this);
			    throw;
			}

			_acceptor = null;
			Monitor.PulseAll(this);
			return;
		    }

		    Debug.Assert(_state == StateActive);

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
		    // Create a connection object for the connection.
		    //
		    if(transceiver != null)
		    {
			try
			{
			    connection = new Ice.ConnectionI(instance_, transceiver, _endpoint, _adapter);
			}
			catch(Ice.LocalException)
			{
			    return;
			}

			_connections.Add(connection);
		    }
		}

		//
		// In thread per connection mode, the connection's thread
		// will take care of connection validation and activation
		// (for non-datagram connections). We don't want to block
		// this thread waiting until validation is complete,
		// because in contrast to thread pool mode, it is the only
		// thread that can accept connections with this factory's
		// acceptor. Therefore we don't call validate() and
		// activate() from the connection factory in thread per
		// connection mode.
		//
	    }
	}

	public void ThreadPerIncomingConnectionFactory()
	{
	    try
	    {
		run();
	    }
	    catch(Ice.Exception ex)
	    {
		instance_.logger().error("exception in thread per incoming connection factory:\n" + ToString() +
					 ex.ToString());
	    }
	    catch(System.Exception ex)
	    {
		instance_.logger().error("system exception in thread per incoming connection factory:\n" + ToString() +
					 ex.ToString());
	    }
	}

	private Thread _threadPerIncomingConnectionFactory;

	private Acceptor _acceptor;
	private readonly Transceiver _transceiver;
	private EndpointI _endpoint;
	
	private readonly Ice.ObjectAdapter _adapter;
	
	private bool _registeredWithPool;
	private int _finishedCount;
	
	private readonly bool _warn;
	
	private LinkedList _connections;
	
	private int _state;
    }

}
