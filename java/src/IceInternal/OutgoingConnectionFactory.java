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
            Connection connection = (Connection)p.next();
            connection.destroy(Connection.CommunicatorDestroyed);
        }

        _destroyed = true;
    }

    public synchronized void
    waitUntilFinished()
    {
	//
	// First we wait until the factory is destroyed.
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
	// Now we wait for until the destruction of each connection is
	// finished.
	//
        java.util.Iterator p = _connections.values().iterator();
        while(p.hasNext())
        {
            Connection connection = (Connection)p.next();
            connection.waitUntilFinished();
        }

	//
	// We're done, now we can throw away all connections.
	//
        _connections.clear();
    }

    public Connection
    create(Endpoint[] endpoints)
    {
	assert(endpoints.length > 0);

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
		Connection con = (Connection)p.next();
		if(con.isFinished())
		{
		    p.remove();
		}
	    }

	    //
	    // Modify endpoints with overrides.
	    //
	    DefaultsAndOverrides defaultsAndOverrides = _instance.defaultsAndOverrides();
	    for(int i = 0; i < endpoints.length; i++)
	    {
		if(defaultsAndOverrides.overrideTimeout)
		{
		    endpoints[i] = endpoints[i].timeout(defaultsAndOverrides.overrideTimeoutValue);
		}
	    }

	    //
	    // Search for existing connections.
	    //
	    for(int i = 0; i < endpoints.length; i++)
	    {
		Connection connection = (Connection)_connections.get(endpoints[i]);
		if(connection != null)
		{
		    //
		    // Don't return connections for which destruction
		    // has been initiated.
		    //
		    if(!connection.isDestroyed())
		    {
			return connection;
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
		    Connection connection = (Connection)_connections.get(endpoints[i]);
		    if(connection != null)
		    {
			//
			// Don't return connections for which
			// destruction has been initiated.
			//
			if(!connection.isDestroyed())
			{
			    return connection;
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

	Connection connection = null;
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
		    transceiver = connector.connect(endpoint.timeout());
		    assert(transceiver != null);
		}
		connection = new Connection(_instance, transceiver, endpoint, null);
		connection.validate();
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
		_connections.put(connection.endpoint(), connection);

		if(_destroyed)
		{
		    connection.destroy(Connection.CommunicatorDestroyed);
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
            Endpoint[] endpoints = ((Ice.ObjectPrxHelper)proxy).__reference().endpoints;
            for(int i = 0; i < endpoints.length; i++)
            {
		Endpoint endpoint = endpoints[i];
		if(defaultsAndOverrides.overrideTimeout)
		{
		    endpoint = endpoint.timeout(defaultsAndOverrides.overrideTimeoutValue);
		}

                Connection connection = (Connection)_connections.get(endpoint);
                if(connection != null)
                {
                    connection.setAdapter(adapter);
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
            Connection connection = (Connection)p.next();
            if(connection.getAdapter() == adapter)
            {
                connection.setAdapter(null);
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
