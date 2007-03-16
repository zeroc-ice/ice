// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public final class IncomingConnectionFactory
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

    public void
    waitUntilHolding()
    {
	java.util.Vector connections = null;

	synchronized(this)
	{
	    //
	    // First we wait until the connection factory itself is in holding
	    // state.
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
	    // We want to wait until all connections are in holding state
	    // outside the thread synchronization.
	    //

	    //
	    // No clone call with J2Me.
	    //
	    //connections = (java.util.Vector)_connections.clone();
	    connections = new java.util.Vector(_connections.size());
	    java.util.Enumeration e = _connections.elements();
	    while(e.hasMoreElements())
	    {
		connections.addElement(e.nextElement());
	    }
	}

	// 
	// Now we wait until each connection is in holding state.
	//
	java.util.Enumeration e = connections.elements();
	while(e.hasMoreElements())
	{
	    Ice.Connection connection = (Ice.Connection)e.nextElement();
	    connection.waitUntilHolding();
	}
    }

    public void
    waitUntilFinished()
    {
	Thread threadPerIncomingConnectionFactory = null;
	java.util.Vector connections;

	synchronized(this)
	{
	    //
	    // First we wait until the factory is destroyed. If we are using
	    // an acceptor, we also wait for it to be closed.
	    //
	    while(_state != StateClosed || _acceptor != null)
	    {
		try
		{
		    wait();
		}
		catch(InterruptedException ex)
		{
		}
	    }

	    threadPerIncomingConnectionFactory = _threadPerIncomingConnectionFactory;
	    _threadPerIncomingConnectionFactory = null;

	    //
	    // We want to wait until all connections are finished outside the
	    // thread synchronization.
	    //
	    // For consistency with C#, we set _connections to null rather than to a
	    // new empty list so that our finalizer does not try to invoke any
	    // methods on member objects.
	    //
	    connections = _connections;
	    _connections = null;
	}

	if(threadPerIncomingConnectionFactory != null)
	{
	    while(true)
	    {
	        try
	        {
		    threadPerIncomingConnectionFactory.join();
		    break;
	        }
	        catch(InterruptedException ex)
	        {
	        }
	    }
	}

        if(connections != null)
        {
	    java.util.Enumeration p = connections.elements();
	    while(p.hasMoreElements())
	    {
	        Ice.Connection connection = (Ice.Connection)p.nextElement();
	        connection.waitUntilFinished();
	    }
        }
    }

    public Endpoint
    endpoint()
    {
        // No mutex protection necessary, _endpoint is immutable.
        return _endpoint;
    }

    public synchronized Ice.Connection[]
    connections()
    {
	java.util.Vector connections = new java.util.Vector();

	//
	// Only copy connections which have not been destroyed.
	//
        java.util.Enumeration p = _connections.elements();
        while(p.hasMoreElements())
        {
            Ice.Connection connection = (Ice.Connection)p.nextElement();
            if(!connection.isDestroyed())
            {
                connections.addElement(connection);
            }
        }

        Ice.Connection[] arr = new Ice.Connection[connections.size()];
        connections.copyInto(arr);
        return arr;
    }

    public void
    flushBatchRequests()
    {
        Ice.Connection[] c = connections(); // connections() is synchronized, so no need to synchronize here.
	for(int i = 0; i < c.length; i++)
	{
	    try
	    {
		c[i].flushBatchRequests();
	    }
	    catch(Ice.LocalException ex)
	    {
		// Ignore.
	    }
	}
    }

    public synchronized String
    toString()
    {
        if(_transceiver != null)
        {
	    return _transceiver.toString();
        }

	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(_acceptor != null);	
	}
	return _acceptor.toString();
    }

    public
    IncomingConnectionFactory(Instance instance, Endpoint endpoint, Ice.ObjectAdapter adapter)
    {
        _instance = instance;
        _endpoint = endpoint;
        _adapter = adapter;
	_warn = _instance.initializationData().properties.getPropertyAsInt("Ice.Warn.Connections") > 0 ? true : false;
        _state = StateHolding;

	DefaultsAndOverrides defaultsAndOverrides = _instance.defaultsAndOverrides();
	if(defaultsAndOverrides.overrideTimeout)
	{
	    _endpoint = _endpoint.timeout(defaultsAndOverrides.overrideTimeoutValue);
	}

	EndpointHolder h = new EndpointHolder();
	h.value = _endpoint;
	_transceiver = _endpoint.serverTransceiver(h);

	try
	{
	    if(_transceiver != null)
	    {
		_endpoint = h.value;
		
		Ice.Connection connection = null;
		
		try
		{
		    connection = new Ice.Connection(_instance, _transceiver, _endpoint, _adapter);

		    //
		    // Wait for the connection to be validated by the
		    // connection thread.  Once the connection has
		    // been validated it will be activated also.
		    //
		    connection.waitForValidation();
		}
		catch(Ice.LocalException ex)
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
		
		_connections.addElement(connection);
	    }
	    else
	    {
		h.value = _endpoint;
		_acceptor = _endpoint.acceptor(h);
		_endpoint = h.value;
		if(IceUtil.Debug.ASSERT)
		{
		    IceUtil.Debug.Assert(_acceptor != null);
		}
		_acceptor.listen();
		
		//
		// If we are in thread per connection mode, we also use
		// one thread per incoming connection factory, that
		// accepts new connections on this endpoint.
		//
		try
		{
		    _threadPerIncomingConnectionFactory = new ThreadPerIncomingConnectionFactory(this);
		    _threadPerIncomingConnectionFactory.start();
		}
		catch(java.lang.Exception ex)
		{
		    error("cannot create thread for incoming connection factory", ex);
		    throw ex;
		}
	    }
	}
	catch(java.lang.Exception ex)
	{
	    
	    if(_acceptor != null)
	    {
		try
		{
		    _acceptor.close();
		}
		catch(Ice.LocalException e)
		{
		    // Here we ignore any exceptions in close().			
		}
	    }

	    //
	    // Clean up for finalizer.
	    //
	    synchronized(this)
	    {
		_state = StateClosed;
		_acceptor = null;
		_connections = null;
		_threadPerIncomingConnectionFactory = null;
	    }

	    Ice.SyscallException e = new Ice.SyscallException();
	    e.initCause(ex);
	    throw e;
	}
    }

    protected synchronized void
    finalize()
        throws Throwable
    {
	IceUtil.Debug.FinalizerAssert(_state == StateClosed);
	IceUtil.Debug.FinalizerAssert(_acceptor == null);
	IceUtil.Debug.FinalizerAssert(_connections == null);
	IceUtil.Debug.FinalizerAssert(_threadPerIncomingConnectionFactory == null);
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

                java.util.Enumeration p = _connections.elements();
                while(p.hasMoreElements())
                {
                    Ice.Connection connection = (Ice.Connection)p.nextElement();
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

                java.util.Enumeration p = _connections.elements();
                while(p.hasMoreElements())
                {
                    Ice.Connection connection = (Ice.Connection)p.nextElement();
                    connection.hold();
                }
                break;
            }

            case StateClosed:
            {
		if(_acceptor != null)
		{
		    //
		    // Connect to our own acceptor, which unblocks our
		    // thread per incoming connection factory stuck in accept().
		    //
		    _acceptor.connectToSelf();
		}

                java.util.Enumeration p = _connections.elements();
                while(p.hasMoreElements())
                {   
                    Ice.Connection connection = (Ice.Connection)p.nextElement();
                    connection.destroy(Ice.Connection.ObjectAdapterDeactivated);
                }
		break;
            }
        }

        _state = state;
	notifyAll();
    }

    public void
    warning(Ice.LocalException ex)
    {
        String s = "connection exception:\n" + ex.toString() + '\n' + _acceptor.toString();
        _instance.initializationData().logger.warning(s);
    }

    public void
    error(String msg, Exception ex)
    {
	String s = msg + ":\n" + toString() + ex.toString();
	_instance.initializationData().logger.error(s);
    }

    public void
    run()
    {
	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(_acceptor != null);
	}

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
	    catch(Ice.SocketException ex)
	    {
		// Do not ignore SocketException in Java.
		throw ex;
	    }
	    catch(Ice.TimeoutException ex)
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

	    Ice.Connection connection = null;

	    synchronized(this)
	    {
		while(_state == StateHolding)
		{
		    try
		    {
			wait();
		    }
		    catch(InterruptedException ex)
		    {
		    }
		}

		if(_state == StateClosed)
		{
		    if(transceiver != null)
		    {
			try
			{
			    transceiver.close();
			}
			catch(Ice.LocalException ex)
			{
			    // Here we ignore any exceptions in close().
			}
		    }

		    try
		    {
			_acceptor.close();
		    }
		    catch(Ice.LocalException ex)
		    {
			_acceptor = null;
			notifyAll();
			throw ex;
		    }

		    _acceptor = null;
		    notifyAll();
		    return;
		}

		if(IceUtil.Debug.ASSERT)
		{
		    IceUtil.Debug.Assert(_state == StateActive);
		}

		//
		// Reap connections for which destruction has completed.
		//
		java.util.Enumeration p = _connections.elements();
		for(int i = _connections.size(); i > 0; --i)
		{
		    Ice.Connection con = (Ice.Connection)_connections.elementAt(i - 1);
		    if(con.isFinished())
		    {
			_connections.removeElementAt(i - 1);
		    }
		}
		
		//
		// Create a connection object for the connection.
		//
		if(transceiver != null)
		{
		    try
		    {
			connection = new Ice.Connection(_instance, transceiver, _endpoint, _adapter);
		    }
		    catch(Ice.LocalException ex)
		    {
			return;
		    }

		    _connections.addElement(connection);
		}
	    }

            //
            // In thread per connection mode, the connection's thread will
            // take care of connection validation and activation. We don't want
            // to block this thread waiting until validation is complete because 
            // it is the only thread that can accept connections with this factory's
            // acceptor. Therefore we don't call validate() and activate()
            // from the connection factory in thread per connection mode.
            //
	}
    }

    private class ThreadPerIncomingConnectionFactory extends Thread
    {
	ThreadPerIncomingConnectionFactory(IncomingConnectionFactory factory)
	{
	    _factory = factory;
	}

	public void
	run()
	{
	    try
	    {
		_factory.run();
	    }
	    catch(Exception ex)
	    {
		_factory.error("exception in thread per incoming connection factory", ex);
	    }
	}

	IncomingConnectionFactory _factory;
    }
    private Thread _threadPerIncomingConnectionFactory;

    private Instance _instance;
    private Acceptor _acceptor;
    private /*final*/ Transceiver _transceiver;
    private Endpoint _endpoint;

    private /*final*/ Ice.ObjectAdapter _adapter;

    private /*final*/ boolean _warn;

    private java.util.Vector _connections = new java.util.Vector();

    private int _state;
}
