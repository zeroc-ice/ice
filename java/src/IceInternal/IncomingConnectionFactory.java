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

package IceInternal;

public class IncomingConnectionFactory extends EventHandler
{
    public synchronized void
    activate()
    {
        setState(StateActive);
    }

    public synchronized void
    hold()
    {
        setState(StateHolding);
    }

    public synchronized void
    destroy()
    {
        setState(StateClosed);
    }

    public synchronized void
    waitUntilHolding()
    {
	//
	// First we wait until the connection factory itself is in
	// holding state.
	//
	while(_state < StateHolding)
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
	// Now we wait until each connection is in holding state.
	//
	java.util.ListIterator p = _connections.listIterator();
	while(p.hasNext())
	{
	    Connection connection = (Connection)p.next();
	    connection.waitUntilHolding();
	}
    }

    public synchronized void
    waitUntilFinished()
    {
	//
	// First we wait until the factory is destroyed.
	//
	while(_acceptor != null)
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
	java.util.ListIterator p = _connections.listIterator();
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

    public Endpoint
    endpoint()
    {
        // No mutex protection necessary, _endpoint is immutable.
        return _endpoint;
    }

    public boolean
    equivalent(Endpoint endp)
    {
        if(_transceiver != null)
        {
            return endp.equivalent(_transceiver);
        }

        assert(_acceptor != null);
        return endp.equivalent(_acceptor);
    }

    public synchronized Connection[]
    connections()
    {
	java.util.LinkedList connections = new java.util.LinkedList();

	//
	// Only copy connections which have not been destroyed.
	//
        java.util.ListIterator p = _connections.listIterator();
        while(p.hasNext())
        {
            Connection connection = (Connection)p.next();
            if(!connection.isDestroyed())
            {
                connections.add(connection);
            }
        }

        Connection[] arr = new Connection[connections.size()];
        connections.toArray(arr);
        return arr;
    }

    //
    // Operations from EventHandler.
    //
    public boolean
    readable()
    {
        return false;
    }

    public void
    read(BasicStream unused)
    {
        assert(false); // Must not be called.
    }

    public void
    message(BasicStream unused, ThreadPool threadPool)
    {
	Connection connection = null;

	synchronized(this)
	{
	    if(_state != StateActive)
	    {
		Thread.yield();
		threadPool.promoteFollower();
		return;
	    }

	    //
	    // Reap connections for which destruction has completed.
	    //
	    java.util.ListIterator p = _connections.listIterator();
	    while(p.hasNext())
	    {
		Connection con = (Connection)p.next();
		if(con.isFinished())
		{
		    p.remove();
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
	    catch(Ice.TimeoutException ex)
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
	    assert(transceiver != null);
            connection = new Connection(_instance, transceiver, _endpoint, _adapter);
            _connections.add(connection);
	}

	assert(connection != null);
	
	//
	// We validate and activate outside the thread
	// synchronization, to not block the factory.
	//
	try
	{
	    connection.validate();
	}
        catch(Ice.LocalException ex)
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

    public synchronized void
    finished(ThreadPool threadPool)
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
	    notifyAll();
	}
    }

    public void
    exception(Ice.LocalException ex)
    {
        assert(false); // Must not be called.
    }

    public synchronized String
    toString()
    {
        if(_transceiver != null)
        {
	    return _transceiver.toString();
        }

        assert(_acceptor != null);	
	return _acceptor.toString();
    }

    public
    IncomingConnectionFactory(Instance instance, Endpoint endpoint, Ice.ObjectAdapter adapter)
    {
        super(instance);
        _endpoint = endpoint;
        _adapter = adapter;
	_registeredWithPool = false;
	_warn = _instance.properties().getPropertyAsInt("Ice.Warn.Connections") > 0 ? true : false;
        _state = StateHolding;

	DefaultsAndOverrides defaultsAndOverrides = _instance.defaultsAndOverrides();
	if(defaultsAndOverrides.overrideTimeout)
	{
	    _endpoint = _endpoint.timeout(defaultsAndOverrides.overrideTimeoutValue);
	}

        try
        {
            EndpointHolder h = new EndpointHolder();
            h.value = _endpoint;
            _transceiver = _endpoint.serverTransceiver(h);
            if(_transceiver != null)
            {
                _endpoint = h.value;
                Connection connection = new Connection(_instance, _transceiver, _endpoint, _adapter);
		connection.validate();
                _connections.add(connection);
	    }
            else
            {
                h.value = _endpoint;
                _acceptor = _endpoint.acceptor(h);
                _endpoint = h.value;
                assert(_acceptor != null);
                _acceptor.listen();
            }
        }
        catch(RuntimeException ex)
        {
	    _state = StateClosed;
	    _acceptor = null;
            throw ex;
        }
    }

    protected void
    finalize()
        throws Throwable
    {
	assert(_state == StateClosed);
	assert(_acceptor == null);
	assert(_connections.size() == 0);

        //
        // Destroy the EventHandler's stream, so that its buffer
        // can be reclaimed.
        //
        super._stream.destroy();

        super.finalize();
    }

    private static final int StateActive = 0;
    private static final int StateHolding = 1;
    private static final int StateClosed = 2;

    private void
    setState(int state)
    {
        if(_state == state) // Don't switch twice.
        {
            return;
        }

        switch(state)
        {
            case StateActive:
            {
                if(_state != StateHolding) // Can only switch from holding to active.
                {
                    return;
                }
                registerWithPool();

                java.util.ListIterator p = _connections.listIterator();
                while(p.hasNext())
                {
                    Connection connection = (Connection)p.next();
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
                unregisterWithPool();

                java.util.ListIterator p = _connections.listIterator();
                while(p.hasNext())
                {
                    Connection connection = (Connection)p.next();
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

                java.util.ListIterator p = _connections.listIterator();
                while(p.hasNext())
                {   
                    Connection connection = (Connection)p.next();
                    connection.destroy(Connection.ObjectAdapterDeactivated);
                }
		break;
            }
        }

        _state = state;
	notifyAll();
    }

    private void
    registerWithPool()
    {
        if(_acceptor != null && !_registeredWithPool)
	{
	    ((Ice.ObjectAdapterI)_adapter).getThreadPool()._register(_acceptor.fd(), this);
	    _registeredWithPool = true;
        }
    }

    private void
    unregisterWithPool()
    {
        if(_acceptor != null && _registeredWithPool)
	{
	    ((Ice.ObjectAdapterI)_adapter).getThreadPool().unregister(_acceptor.fd());
	    _registeredWithPool = false;
        }
    }

    private void
    warning(Ice.LocalException ex)
    {
        java.io.StringWriter sw = new java.io.StringWriter();
        java.io.PrintWriter pw = new java.io.PrintWriter(sw);
        ex.printStackTrace(pw);
        pw.flush();
        String s = "connection exception:\n" + sw.toString() + '\n' + _acceptor.toString();
        _instance.logger().warning(s);
    }

    private Acceptor _acceptor;
    private final Transceiver _transceiver;
    private Endpoint _endpoint;

    private final Ice.ObjectAdapter _adapter;

    private ThreadPool _serverThreadPool;
    private boolean _registeredWithPool;

    private final boolean _warn;

    private java.util.LinkedList _connections = new java.util.LinkedList();

    private int _state;
}
