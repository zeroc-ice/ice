// **********************************************************************
//
// Copyright (c) 2003
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

namespace IceInternal
{

    using System.Collections;
    using System.Diagnostics;
    using IceUtil;

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
		    foreach(Connection c in connections)
		    {
			c.destroy(Connection.CommunicatorDestroyed);
		    }
		}

		_destroyed = true;
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
		    try
		    {
			System.Threading.Monitor.Wait(this);
		    }
		    catch(System.Threading.ThreadInterruptedException)
		    {
		    }
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
		foreach(Connection c in cl)
		{
		    c.waitUntilFinished();
		}
	    }
	}
	
	public virtual Connection create(Endpoint[] endpoints)
	{
	    Debug.Assert(endpoints.Length > 0);
	    
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
			if(((Connection)q.Current).isFinished())
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
		}
		
		//
		// Search for existing connections.
		//
		for(int j = 0; j < endpoints.Length; j++)
		{
		    LinkedList connectionList = (LinkedList)_connections[endpoints[j]];
		    if(connectionList != null)
		    {
			foreach(Connection connection in connectionList)
			{
			    if(!connection.isDestroyed())
			    {
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
		    
		    try
		    {
			System.Threading.Monitor.Wait(this);
		    }
		    catch(System.Threading.ThreadInterruptedException)
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
		    for(int j = 0; j < endpoints.Length; j++)
		    {
			LinkedList connectionList = (LinkedList)_connections[endpoints[j]];
			if(connectionList != null)
			{
			    foreach(Connection connection in connectionList)
			    {
				if(!connection.isDestroyed())
				{
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
	    
	    Connection newConnection = null;
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
			transceiver = connector.connect(endpoint.timeout());
			Debug.Assert(transceiver != null);
		    }
		    newConnection = new Connection(_instance, transceiver, endpoint, null);
		    newConnection.validate();
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
			newConnection.destroy(Connection.CommunicatorDestroyed);
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
		    Endpoint[] endpoints = ((Ice.ObjectPrxHelper) proxy).__reference().endpoints;
		    for(int i = 0; i < endpoints.Length; i++)
		    {
			Endpoint endpoint = endpoints[i];
			if(defaultsAndOverrides.overrideTimeout)
			{
			    endpoint = endpoint.timeout(defaultsAndOverrides.overrideTimeoutValue);
			}
			
			LinkedList connectionList = (LinkedList)_connections[endpoints[i]];
			if(connectionList != null)
			{
			    foreach(Connection connection in connectionList)
			    {
				connection.setAdapter(adapter);
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
		    foreach(Connection connection in connectionList)
		    {
			if(connection.getAdapter() == adapter)
			{
			    connection.setAdapter(null);
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
		    foreach(Connection conn in connectionList)
		    {
			c.Add(conn);
		    }
		}
	    }
	    
	    foreach(Connection conn in c)
	    {
		if(conn.isValidated())
		{
		    try
		    {
			conn.flushBatchRequest();
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
