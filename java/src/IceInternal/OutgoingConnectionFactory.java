// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class OutgoingConnectionFactory
{
    public synchronized void
    destroy()
    {
        if(_destroyed)
        {
            return;
        }

        java.util.Iterator p = _connections.values().iterator();
        while(p.hasNext())
        {
	    java.util.LinkedList connectionList = (java.util.LinkedList)p.next();
		
	    java.util.Iterator q = connectionList.iterator();
	    while(q.hasNext())
	    {
		Ice.ConnectionI connection = (Ice.ConnectionI)q.next();
		connection.destroy(Ice.ConnectionI.CommunicatorDestroyed);
	    }
	}

        _destroyed = true;
        notifyAll();
    }

    public void
    waitUntilFinished()
    {
	java.util.HashMap connections;

	synchronized(this)
	{
	    //
	    // First we wait until the factory is destroyed. We also
	    // wait until there are no pending connections
	    // anymore. Only then we can be sure the _connections
	    // contains all connections.
	    //
	    while(!_destroyed || !_pending.isEmpty())
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
	    // We want to wait until all connections are finished
	    // outside the thread synchronization.
	    //
	    connections = _connections;
	    _connections = new java.util.HashMap();
	}
	
	//
	// Now we wait for until the destruction of each connection is
	// finished.
	//
        java.util.Iterator p = connections.values().iterator();
        while(p.hasNext())
        {
	    java.util.LinkedList connectionList = (java.util.LinkedList)p.next();
		
	    java.util.Iterator q = connectionList.iterator();
	    while(q.hasNext())
	    {
		Ice.ConnectionI connection = (Ice.ConnectionI)q.next();
		connection.waitUntilFinished();
	    }
        }
    }

    public Ice.ConnectionI
    create(Endpoint[] endpts, Ice.BooleanHolder compress)
    {
	assert(endpts.length > 0);
	Endpoint[] endpoints = new Endpoint[endpts.length];
	System.arraycopy(endpts, 0, endpoints, 0, endpts.length);

        DefaultsAndOverrides defaultsAndOverrides = _instance.defaultsAndOverrides();

	synchronized(this)
	{
	    if(_destroyed)
	    {
		throw new Ice.CommunicatorDestroyedException();
	    }

	    //
	    // Reap connections for which destruction has completed.
	    //
	    java.util.Iterator p = _connections.values().iterator();
	    while(p.hasNext())
	    {
		java.util.LinkedList connectionList = (java.util.LinkedList)p.next();
		
		java.util.Iterator q = connectionList.iterator();
		while(q.hasNext())
		{
		    Ice.ConnectionI con = (Ice.ConnectionI)q.next();
		    if(con.isFinished())
		    {
			q.remove();
		    }
		}

		if(connectionList.isEmpty())
		{
		    p.remove();
		}
	    }

	    //
	    // Modify endpoints with overrides.
	    //
	    for(int i = 0; i < endpoints.length; i++)
	    {
		if(defaultsAndOverrides.overrideTimeout)
		{
		    endpoints[i] = endpoints[i].timeout(defaultsAndOverrides.overrideTimeoutValue);
		}

		//
		// The Connection object does not take the compression flag of
		// endpoints into account, but instead gets the information
		// about whether messages should be compressed or not from
		// other sources. In order to allow connection sharing for
		// endpoints that differ in the value of the compression flag
		// only, we always set the compression flag to false here in
		// this connection factory.
		//
		endpoints[i] = endpoints[i].compress(false);
	    }

	    //
	    // Search for existing connections.
	    //
	    for(int i = 0; i < endpoints.length; i++)
	    {
		java.util.LinkedList connectionList = (java.util.LinkedList)_connections.get(endpoints[i]);
		if(connectionList != null)
		{
		    java.util.Iterator q = connectionList.iterator();
			
		    while(q.hasNext())
		    {
			Ice.ConnectionI connection = (Ice.ConnectionI)q.next();
			
			//
			// Don't return connections for which destruction has
			// been initiated.
			//
			if(!connection.isDestroyed())
			{
			    if(defaultsAndOverrides.overrideCompress)
			    {
				compress.value = defaultsAndOverrides.overrideCompressValue;
			    }
			    else
			    {
				compress.value = endpts[i].compress();
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
	    boolean searchAgain = false;
	    while(!_destroyed)
	    {
		int i;
		for(i = 0; i < endpoints.length; i++)
		{
		    if(_pending.contains(endpoints[i]))
		    {
			break;
		    }
		}
		
		if(i == endpoints.length)
		{
		    break;
		}
		
		searchAgain = true;

		try
		{
		    wait();
		}
		catch(InterruptedException ex)
		{
		}
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
		for(int i = 0; i < endpoints.length; i++)
		{
		    java.util.LinkedList connectionList = (java.util.LinkedList)_connections.get(endpoints[i]);
		    if(connectionList != null)
		    {
			java.util.Iterator q = connectionList.iterator();
			
			while(q.hasNext())
			{
			    Ice.ConnectionI connection = (Ice.ConnectionI)q.next();
			    
			    //
			    // Don't return connections for which destruction has
			    // been initiated.
			    //
			    if(!connection.isDestroyed())
			    {
				if(defaultsAndOverrides.overrideCompress)
				{
				    compress.value = defaultsAndOverrides.overrideCompressValue;
				}
				else
				{
				    compress.value = endpts[i].compress();
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
	    for(int i = 0; i < endpoints.length; i++)
	    {
		_pending.add(endpoints[i]);
	    }
	}

	Ice.ConnectionI connection = null;
	Ice.LocalException exception = null;

	for(int i = 0; i < endpoints.length; i++)
	{
	    Endpoint endpoint = endpoints[i];
	    
	    try
	    {
		Transceiver transceiver = endpoint.clientTransceiver();
		if(transceiver == null)
		{
		    Connector connector = endpoint.connector();
		    assert(connector != null);

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
		    assert(transceiver != null);
		}
		connection = new Ice.ConnectionI(_instance, transceiver, endpoint, null);
		connection.validate();

		if(defaultsAndOverrides.overrideCompress)
		{
		    compress.value = defaultsAndOverrides.overrideCompressValue;
		}
		else
		{
		    compress.value = endpts[i].compress();
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
		StringBuffer s = new StringBuffer();
		s.append("connection to endpoint failed");
		if(i < endpoints.length - 1)
		{
		    s.append(", trying next endpoint\n");
		}
		else
		{
		    s.append(" and no more endpoints to try\n");
		}
		s.append(exception.toString());
		_instance.logger().trace(traceLevels.retryCat, s.toString());
	    }
	}
	
	synchronized(this)
	{
	    //
	    // Signal other threads that we are done with trying to
	    // establish connections to our endpoints.
	    //
	    for(int i = 0; i < endpoints.length; i++)
	    {
		_pending.remove(endpoints[i]);
	    }
	    notifyAll();
	    
	    if(connection == null)
	    {
		assert(exception != null);
		throw exception;
	    }
	    else
	    {
		java.util.LinkedList connectionList = (java.util.LinkedList)_connections.get(connection.endpoint());
		if(connectionList == null)
		{
		    connectionList = new java.util.LinkedList();
		    _connections.put(connection.endpoint(), connectionList);
		}
		connectionList.add(connection);

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
	
	assert(connection != null);
        return connection;
    }

    public synchronized void
    setRouter(Ice.RouterPrx router)
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
            for(int i = 0; i < endpoints.length; i++)
            {
		Endpoint endpoint = endpoints[i];

		//
		// Modify endpoints with overrides.
		//
		if(defaultsAndOverrides.overrideTimeout)
		{
		    endpoint = endpoint.timeout(defaultsAndOverrides.overrideTimeoutValue);
		}

		//
		// The Connection object does not take the compression flag of
		// endpoints into account, but instead gets the information
		// about whether messages should be compressed or not from
		// other sources. In order to allow connection sharing for
		// endpoints that differ in the value of the compression flag
		// only, we always set the compression flag to false here in
		// this connection factory.
		//
		endpoint = endpoint.compress(false);

		java.util.LinkedList connectionList = (java.util.LinkedList)_connections.get(endpoints[i]);
		if(connectionList != null)
		{
		    java.util.Iterator p = connectionList.iterator();
		    
		    while(p.hasNext())
		    {
			Ice.ConnectionI connection = (Ice.ConnectionI)p.next();
			connection.setAdapter(adapter);
		    }
                }
            }
        }
    }

    public synchronized void
    removeAdapter(Ice.ObjectAdapter adapter)
    {
        if(_destroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        java.util.Iterator p = _connections.values().iterator();
        while(p.hasNext())
        {
	    java.util.LinkedList connectionList = (java.util.LinkedList)p.next();
		
	    java.util.Iterator q = connectionList.iterator();
	    while(q.hasNext())
	    {
		Ice.ConnectionI connection = (Ice.ConnectionI)q.next();
		if(connection.getAdapter() == adapter)
		{
		    connection.setAdapter(null);
		}
	    }
	}
    }

    public void
    flushBatchRequests()
    {
	java.util.LinkedList c = new java.util.LinkedList();

        synchronized(this)
	{
	    java.util.Iterator p = _connections.values().iterator();
	    while(p.hasNext())
	    {
		java.util.LinkedList connectionList = (java.util.LinkedList)p.next();
		java.util.Iterator q = connectionList.iterator();
		while(q.hasNext())
		{
		    c.add(q.next());
		}
	    }
	}

	java.util.Iterator p = c.iterator();
	while(p.hasNext())
	{
	    Ice.ConnectionI conn = (Ice.ConnectionI)p.next();
	    try
	    {
		conn.flushBatchRequests();
	    }
	    catch(Ice.LocalException ex)
	    {
		// Ignore.
	    }
	}
    }

    //
    // Only for use by Instance.
    //
    OutgoingConnectionFactory(Instance instance)
    {
        _instance = instance;
	_destroyed = false;
    }

    protected void
    finalize()
        throws Throwable
    {
        assert(_destroyed);
	assert(_connections.isEmpty());

        super.finalize();
    }

    private final Instance _instance;
    private boolean _destroyed;
    private java.util.HashMap _connections = new java.util.HashMap();
    private java.util.HashSet _pending = new java.util.HashSet();
}
