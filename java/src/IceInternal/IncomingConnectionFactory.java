// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
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
        threadPool.promoteFollower();

        if(_state != StateActive)
        {
            Thread.yield();
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
	    return;
        }
        catch(Ice.LocalException ex)
        {
            if(_warn)
            {
                warning(ex);
            }
            setState(StateClosed);
	    return;
        }

	//
	// Create and activate a connection object for the connection.
	//
	try
	{
	    assert(transceiver != null);
            Connection connection = new Connection(_instance, transceiver, _endpoint, _adapter);
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
	    // Break cyclic object dependency. This is necessary,
	    // because the object adapter never clears the list of
	    // incoming connections it keeps.
	    //
	    _adapter = null;

	    notifyAll(); // For waitUntilFinished().
        }
    }

    public void
    exception(Ice.LocalException ex)
    {
        assert(false); // Must not be called.
    }

    public
    IncomingConnectionFactory(Instance instance, Endpoint endpoint, Ice.ObjectAdapter adapter)
    {
        super(instance);
        _endpoint = endpoint;
	DefaultsAndOverrides defaultsAndOverrides = _instance.defaultsAndOverrides();
	if(defaultsAndOverrides.overrideTimeout)
	{
	    _endpoint = _endpoint.timeout(defaultsAndOverrides.overrideTimeoutValue);
	}
        _adapter = adapter;
        _state = StateHolding;
	_warn = _instance.properties().getPropertyAsInt("Ice.ConnectionWarnings") > 0 ? true : false;
	_registeredWithPool = false;

        try
        {
            EndpointHolder h = new EndpointHolder();
            h.value = _endpoint;
            _transceiver = _endpoint.serverTransceiver(h);
            if(_transceiver != null)
            {
                _endpoint = h.value;
                Connection connection = new Connection(_instance, _transceiver, _endpoint, _adapter);
                _connections.add(connection);

		//
		// We don't need an adapter anymore if we don't use an
		// acceptor. So we break cyclic object dependency
		// now. This is necessary, because the object adapter
		// never clears the list of incoming connections it keeps.
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

    public synchronized void
    waitUntilFinished()
    {
	while(_state != StateClosed || _adapter != null)
	{
	    try
	    {
		wait();
	    }
	    catch(InterruptedException ex)
	    {
	    }
	}
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

		notifyAll(); // For waitUntilFinished().

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
		if(_serverThreadPool == null)
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
    private Ice.ObjectAdapter _adapter;
    private Acceptor _acceptor;
    private Transceiver _transceiver;
    private ThreadPool _serverThreadPool;
    private java.util.LinkedList _connections = new java.util.LinkedList();
    private int _state;
    private boolean _warn;
    private boolean _registeredWithPool;
}
