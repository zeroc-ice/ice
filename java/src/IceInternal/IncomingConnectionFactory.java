// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public final class IncomingConnectionFactory extends EventHandler implements Ice.ConnectionI.StartCallback
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
        java.util.LinkedList<Ice.ConnectionI> connections;

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
            connections = new java.util.LinkedList<Ice.ConnectionI>(_connections);
        }

        //
        // Now we wait until each connection is in holding state.
        //
        java.util.ListIterator<Ice.ConnectionI> p = connections.listIterator();
        while(p.hasNext())
        {
            Ice.ConnectionI connection = p.next();
            connection.waitUntilHolding();
        }
    }

    public void
    waitUntilFinished()
    {
        java.util.LinkedList<Ice.ConnectionI> connections = null;

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

            //
            // Clear the OA. See bug 1673 for the details of why this is necessary.
            //
            _adapter = null;

            //
            // We want to wait until all connections are finished outside the
            // thread synchronization.
            //
            if(_connections != null)
            {
                connections = new java.util.LinkedList<Ice.ConnectionI>(_connections);
            }
        }

        if(connections != null)
        {
            java.util.ListIterator<Ice.ConnectionI> p = connections.listIterator();
            while(p.hasNext())
            {
                Ice.ConnectionI connection = p.next();
                connection.waitUntilFinished();
            }
        }

        synchronized(this)
        {
            //
            // For consistency with C#, we set _connections to null rather than to a
            // new empty list so that our finalizer does not try to invoke any
            // methods on member objects.
            //
            _connections = null;
        }
    }

    public EndpointI
    endpoint()
    {
        // No mutex protection necessary, _endpoint is immutable.
        return _endpoint;
    }

    public synchronized java.util.LinkedList<Ice.ConnectionI>
    connections()
    {
        java.util.LinkedList<Ice.ConnectionI> connections = new java.util.LinkedList<Ice.ConnectionI>();

        //
        // Only copy connections which have not been destroyed.
        //
        java.util.ListIterator<Ice.ConnectionI> p = _connections.listIterator();
        while(p.hasNext())
        {
            Ice.ConnectionI connection = p.next();
            if(connection.isActiveOrHolding())
            {
                connections.add(connection);
            }
        }
        
        return connections;
    }

    public void
    flushBatchRequests()
    {
        java.util.Iterator<Ice.ConnectionI> p =
            connections().iterator(); // connections() is synchronized, no need to synchronize here.
        while(p.hasNext())
        {
            try
            {
                p.next().flushBatchRequests();
            }
            catch(Ice.LocalException ex)
            {
                // Ignore.
            }
        }
    }

    //
    // Operations from SelectorHandler.
    //

    public java.nio.channels.SelectableChannel
    fd()
    {
        assert(_acceptor != null);
        return _acceptor.fd();
    }

    public boolean
    hasMoreData()
    {
        assert(_acceptor != null);
        return false;
    }

    //
    // Operations from EventHandler.
    //

    public boolean
    datagram()
    {
        return _endpoint.datagram();
    }

    public boolean
    readable()
    {
        return false;
    }

    public boolean
    read(BasicStream unused)
    {
        assert(false); // Must not be called.
        return false;
    }

    public void
    message(BasicStream unused, ThreadPool threadPool)
    {
        Ice.ConnectionI connection = null;

        try
        {
            synchronized(this)
            {
                if(_state != StateActive)
                {
                    Thread.yield();
                    return;
                }
                
                //
                // Reap connections for which destruction has completed.
                //
                java.util.ListIterator<Ice.ConnectionI> p = _connections.listIterator();
                while(p.hasNext())
                {
                    Ice.ConnectionI con = p.next();
                    if(con.isFinished())
                    {
                        p.remove();
                    }
                }
                
                //
                // Now accept a new connection.
                //
                Transceiver transceiver = null;
                try
                {
                    transceiver = _acceptor.accept();
                }
                catch(Ice.SocketException ex)
                {
                    if(Network.noMoreFds(ex.getCause()))
                    {
                        try
                        {
                            String s = "fatal error: can't accept more connections:\n" + ex.getCause().getMessage();
                            s += '\n' + _acceptor.toString();
                            _instance.initializationData().logger.error(s);
                        }
                        finally
                        {
                            Runtime.getRuntime().halt(1);
                        }
                    }                        

                    // Ignore socket exceptions.
                    return;
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

                assert(transceiver != null);

                try
                {
                    connection = new Ice.ConnectionI(_instance, transceiver, _endpoint, _adapter);
                }
                catch(Ice.LocalException ex)
                {
		    try
		    {
			transceiver.close();
		    }
		    catch(Ice.LocalException exc)
		    {
			// Ignore
		    }

                    if(_warn)
                    {
                        warning(ex);
                    }
                    return;
                }

                _connections.add(connection);
            }
        }
        finally
        {
            //
            // This makes sure that we promote a follower before we leave the scope of the mutex
            // above, but after we call accept() (if we call it).
            //
            threadPool.promoteFollower(null);
        }

        connection.start(this);
    }

    public synchronized void
    finished(ThreadPool threadPool)
    {
        threadPool.promoteFollower(null);
        assert(threadPool == ((Ice.ObjectAdapterI)_adapter).getThreadPool() && _state == StateClosed);

        _acceptor.close();
        _acceptor = null;
        notifyAll();
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

    //
    // Operations from ConnectionI.StartCallback
    //
    public synchronized void
    connectionStartCompleted(Ice.ConnectionI connection)
    {
        //
        // Initially, connections are in the holding state. If the factory is active
        // we activate the connection.
        //
        if(_state == StateActive)
        {
            connection.activate();
        }
    }

    public synchronized void
    connectionStartFailed(Ice.ConnectionI connection, Ice.LocalException ex)
    {
        if(_state == StateClosed)
        {
            return;
        }
        
        if(_warn)
        {
            warning(ex);
        }
        
        //
        // If the connection is finished, remove it right away from
        // the connection map. Otherwise, we keep it in the map, it
        // will eventually be reaped.
        //
        if(connection.isFinished())
        {
            _connections.remove(connection);
        }
    }

    public
    IncomingConnectionFactory(Instance instance, EndpointI endpoint, Ice.ObjectAdapter adapter, String adapterName)
    {
        super(instance);
        _endpoint = endpoint;
        _adapter = adapter;
        _warn = _instance.initializationData().properties.getPropertyAsInt("Ice.Warn.Connections") > 0 ? true : false;
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
            EndpointIHolder h = new EndpointIHolder();
            h.value = _endpoint;
            _transceiver = _endpoint.transceiver(h);

            if(_transceiver != null)
            {
                _endpoint = h.value;
                
                Ice.ConnectionI connection;
		try
		{
		    connection = new Ice.ConnectionI(_instance, _transceiver, _endpoint, _adapter);
		}
		catch(Ice.LocalException ex)
		{
		    try
		    {
			_transceiver.close();
		    }
		    catch(Ice.LocalException exc)
		    {
			// Ignore
		    }
		    throw ex;
		}
                connection.start(null);
                
                _connections.add(connection);
            }
            else
            {
                h.value = _endpoint;
                _acceptor = _endpoint.acceptor(h, adapterName);
                _endpoint = h.value;
                assert(_acceptor != null);
                _acceptor.listen();
            }
        }
        catch(java.lang.Exception ex)
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
                catch(Ice.LocalException e)
                {
                    // Here we ignore any exceptions in close().                        
                }
            }

            synchronized(this)
            {
                _state = StateClosed;
                _acceptor = null;
                _connections = null;
            }

            if(ex instanceof Ice.LocalException)
            {
                throw (Ice.LocalException)ex;
            }
            else
            {
                Ice.SyscallException e = new Ice.SyscallException();
                e.initCause(ex);
                throw e;
            }
        }
    }

    protected synchronized void
    finalize()
        throws Throwable
    {
        IceUtilInternal.Assert.FinalizerAssert(_state == StateClosed);
        IceUtilInternal.Assert.FinalizerAssert(_acceptor == null);
        IceUtilInternal.Assert.FinalizerAssert(_connections == null);

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
                if(_acceptor != null)
                {
                    ((Ice.ObjectAdapterI)_adapter).getThreadPool()._register(this);
                }

                java.util.ListIterator<Ice.ConnectionI> p = _connections.listIterator();
                while(p.hasNext())
                {
                    Ice.ConnectionI connection = p.next();
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
                if(_acceptor != null)
                {
                    ((Ice.ObjectAdapterI)_adapter).getThreadPool().unregister(this);
                }

                java.util.ListIterator<Ice.ConnectionI> p = _connections.listIterator();
                while(p.hasNext())
                {
                    Ice.ConnectionI connection = p.next();
                    connection.hold();
                }
                break;
            }

            case StateClosed:
            {
                if(_acceptor != null)
                {
                    ((Ice.ObjectAdapterI)_adapter).getThreadPool().finish(this);
                }

                java.util.ListIterator<Ice.ConnectionI> p = _connections.listIterator();
                while(p.hasNext())
                {   
                    Ice.ConnectionI connection = p.next();
                    connection.destroy(Ice.ConnectionI.ObjectAdapterDeactivated);
                }
                break;
            }
        }

        _state = state;
        notifyAll();
    }

    private void
    warning(Ice.LocalException ex)
    {
        java.io.StringWriter sw = new java.io.StringWriter();
        java.io.PrintWriter pw = new java.io.PrintWriter(sw);
        ex.printStackTrace(pw);
        pw.flush();
        String s = "connection exception:\n" + sw.toString() + '\n' + _acceptor.toString();
        _instance.initializationData().logger.warning(s);
    }

    private Acceptor _acceptor;
    private final Transceiver _transceiver;
    private EndpointI _endpoint;

    private Ice.ObjectAdapter _adapter;

    private final boolean _warn;

    private java.util.List<Ice.ConnectionI> _connections = new java.util.LinkedList<Ice.ConnectionI>();

    private int _state;
}
