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
		    try
		    {
			System.Threading.Monitor.Wait(this);
		    }
		    catch(System.Threading.ThreadInterruptedException)
		    {
		    }
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
	    foreach(Connection connection in connections)
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
		_connections = new LinkedList();
	    }
	    
	    //
	    // Now we wait for until the destruction of each connection is
	    // finished.
	    //
	    foreach(Connection connection in connections)
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
	
	public virtual Connection[] connections()
	{
	    lock(this)
	    {
		LinkedList connections = new LinkedList();
		
		//
		// Only copy connections which have not been destroyed.
		//
		foreach(Connection connection in _connections)
		{
		    if(!connection.isDestroyed())
		    {
			connections.Add(connection);
		    }
		}
		Connection[] arr = new Connection[connections.Count];
		connections.CopyTo(arr, 0);
		return arr;
	    }
	}
	
	public virtual void flushBatchRequests()
	{
	    //
	    // connections() is synchronized, so no need to synchronize here.
	    //
	    foreach(Connection connection in connections())
	    {
		try
		{
		    connection.flushBatchRequest();
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
	    Connection connection = null;
	    
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
		    Connection con = (Connection)p.Current;
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
		connection = new Connection(_instance, transceiver, _endpoint, _adapter);
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
		// Ignore all exceptions while activating or validating
		// the connection object. Warning or error messages for
		// such exceptions must be printed directly in the
		// connection object code.
		//
	    }
	    
	    //
	    // The factory must be active at this point, so we activate
	    // the connection, too.
	    //
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
	    
	    try
	    {
		Endpoint h = _endpoint;
		_transceiver = _endpoint.serverTransceiver(ref h);
		if(_transceiver != null)
		{
		    _endpoint = h;
		    Connection connection = new Connection(_instance, _transceiver, _endpoint, _adapter);
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
		    
		    foreach(Connection connection in _connections)
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
		    
		    foreach(Connection connection in _connections)
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
		    
		    foreach(Connection connection in _connections)
		    {
			connection.destroy(Connection.ObjectAdapterDeactivated);
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
		((Ice.ObjectAdapterI) _adapter).getThreadPool()._register(_acceptor.fd(), this);
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

}
