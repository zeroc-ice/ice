// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package IceInternal;

public class IncomingConnectionFactory extends EventHandler
{
    public synchronized void
    hold()
    {
        setState(StateHolding);
    }

    public synchronized void
    activate()
    {
        setState(StateActive);
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
        //
        // Reap destroyed connections.
        //
        java.util.ListIterator iter = _connections.listIterator();
        while(iter.hasNext())
        {
            Connection connection = (Connection)iter.next();
            if(connection.destroyed())
            {
                iter.remove();
            }
        }

        Connection[] arr = new Connection[_connections.size()];
        _connections.toArray(arr);
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

    public synchronized void
    message(BasicStream unused, ThreadPool threadPool)
    {
        if(_state != StateActive)
        {
            Thread.yield();
	    threadPool.promoteFollower();
            return;
        }

        //
        // Reap destroyed connections.
        //
        java.util.ListIterator iter = _connections.listIterator();
        while(iter.hasNext())
        {
            Connection connection = (Connection)iter.next();
            if(connection.destroyed())
            {
                iter.remove();
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
	    threadPool.promoteFollower();
	    return;
        }
        catch(Ice.LocalException ex)
        {
            if(_warn)
            {
                warning(ex);
            }
            setState(StateClosed);
	    threadPool.promoteFollower();
	    return;
        }
	catch(RuntimeException ex)
	{
	    threadPool.promoteFollower();
	    throw ex;
	}

	//
	// We must promote a follower after we accepted the new
	// connection.
	//
        threadPool.promoteFollower();

	//
	// Create and activate a connection object for the connection.
	//
	try
	{
	    assert(transceiver != null);
            Connection connection = new Connection(_instance, transceiver, _endpoint, _adapter);
	    connection.validate();
            connection.activate();
            _connections.add(connection);
	}
        catch(Ice.LocalException ex)
	{
	    //
	    // Ignore all exceptions while creating or activating the
	    // connection object. Warning or error messages for such
	    // exceptions must be printed directly in the connection
	    // object code.
	    //
	}
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

	    //
	    // We don't need the adapter anymore after we closed the
	    // acceptor.
	    //
	    _adapter = null;
        }
    }

    public void
    exception(Ice.LocalException ex)
    {
        assert(false); // Must not be called.
    }

    public String toString()
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
	_warn = _instance.properties().getPropertyAsInt("Ice.ConnectionWarnings") > 0 ? true : false;
        _state = StateHolding;
	_registeredWithPool = false;

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
		
		//
		// We don't need an adapter anymore if we don't use an
		// acceptor.
		//
		_adapter = null;
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
            setState(StateClosed);
            throw ex;
        }
    }

    protected void
    finalize()
        throws Throwable
    {
        assert(_state == StateClosed);
	assert(_adapter == null);

        //
        // Destroy the EventHandler's stream, so that its buffer
        // can be reclaimed.
        //
        super._stream.destroy();

        super.finalize();
    }

    public synchronized void
    destroy()
    {
        setState(StateClosed);
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

                java.util.ListIterator iter = _connections.listIterator();
                while(iter.hasNext())
                {
                    Connection connection = (Connection)iter.next();
                    connection.activate();
                }
                break;
            }

            case StateHolding:
            {
                if(_state != StateActive) // Can only switch from active to holding
                {
                    return;
                }
                unregisterWithPool();

                java.util.ListIterator iter = _connections.listIterator();
                while(iter.hasNext())
                {
                    Connection connection = (Connection)iter.next();
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

                java.util.ListIterator iter = _connections.listIterator();
                while(iter.hasNext())
                {   
                    Connection connection = (Connection)iter.next();
                    connection.destroy(Connection.ObjectAdapterDeactivated);
                }
                _connections.clear();

                break;
            }
        }

        _state = state;
    }

    private void
    registerWithPool()
    {
        if(_acceptor != null)
        {
	    if(!_registeredWithPool)
	    {
		if(_serverThreadPool == null) // Lazy initialization.
		{
		    _serverThreadPool = _instance.serverThreadPool();
		    assert(_serverThreadPool != null);
		}
		_serverThreadPool._register(_acceptor.fd(), this);
		_registeredWithPool = true;
	    }
        }
    }

    private void
    unregisterWithPool()
    {
        if(_acceptor != null)
        {
	    if(_registeredWithPool)
	    {
		assert(_serverThreadPool != null);
		_serverThreadPool.unregister(_acceptor.fd());
		_registeredWithPool = false;
	    }
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

    private Endpoint _endpoint;
    private Ice.ObjectAdapter _adapter; // Cannot be final, because it must be set to null to break cyclic dependency.
    private Acceptor _acceptor;
    private final Transceiver _transceiver;
    private ThreadPool _serverThreadPool;
    private final boolean _warn;
    private java.util.LinkedList _connections = new java.util.LinkedList();
    private int _state;
    private boolean _registeredWithPool;
}
