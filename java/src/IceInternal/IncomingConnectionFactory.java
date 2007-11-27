// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
        java.util.LinkedList connections;

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
            connections = (java.util.LinkedList)_connections.clone();
        }

        //
        // Now we wait until each connection is in holding state.
        //
        java.util.ListIterator p = connections.listIterator();
        while(p.hasNext())
        {
            Ice.ConnectionI connection = (Ice.ConnectionI)p.next();
            connection.waitUntilHolding();
        }
    }

    public void
    waitUntilFinished()
    {
        Thread threadPerIncomingConnectionFactory = null;
        java.util.LinkedList connections = null;

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
            // Clear the OA. See bug 1673 for the details of why this is necessary.
            //
            _adapter = null;

            //
            // We want to wait until all connections are finished outside the
            // thread synchronization.
            //
            if(_connections != null)
            {
                connections = new java.util.LinkedList(_connections);
            }
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
            java.util.ListIterator p = connections.listIterator();
            while(p.hasNext())
            {
                Ice.ConnectionI connection = (Ice.ConnectionI)p.next();
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

    public synchronized java.util.LinkedList
    connections()
    {
        java.util.LinkedList connections = new java.util.LinkedList();

        //
        // Only copy connections which have not been destroyed.
        //
        java.util.ListIterator p = _connections.listIterator();
        while(p.hasNext())
        {
            Ice.ConnectionI connection = (Ice.ConnectionI)p.next();
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
        java.util.Iterator p = connections().iterator(); // connections() is synchronized, no need to synchronize here.
        while(p.hasNext())
        {
            try
            {
                ((Ice.ConnectionI)p.next()).flushBatchRequests();
            }
            catch(Ice.LocalException ex)
            {
                // Ignore.
            }
        }
    }

    //
    // Operations from EventHandler.
    //

    public boolean
    datagram()
    {
        assert(!_threadPerConnection); // Only for use with a thread pool.
        return _endpoint.datagram();
    }

    public boolean
    readable()
    {
        assert(!_threadPerConnection); // Only for use with a thread pool.
        return false;
    }

    public boolean
    read(BasicStream unused)
    {
        assert(!_threadPerConnection); // Only for use with a thread pool.
        assert(false); // Must not be called.
        return false;
    }

    public boolean
    hasMoreData()
    {
        assert(!_threadPerConnection); // Only for use with a thread pool.
        assert(false); // Must not be called.
        return false;
    }

    public void
    message(BasicStream unused, ThreadPool threadPool)
    {
        assert(!_threadPerConnection); // Only for use with a thread pool.

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
                java.util.ListIterator p = _connections.listIterator();
                while(p.hasNext())
                {
                    Ice.ConnectionI con = (Ice.ConnectionI)p.next();
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
                    transceiver = _acceptor.accept(0);
                }
                catch(Ice.SocketException ex)
                {
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
                    assert(!_threadPerConnection);
                    connection = new Ice.ConnectionI(_instance, transceiver, _endpoint, _adapter, false);
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
            threadPool.promoteFollower();
        }

        connection.start(this);
    }

    public synchronized void
    finished(ThreadPool threadPool)
    {
        assert(!_threadPerConnection); // Only for use with a thread pool.

        threadPool.promoteFollower();
        assert(threadPool == ((Ice.ObjectAdapterI)_adapter).getThreadPool());

        --_finishedCount;

        if(_finishedCount == 0 && _state == StateClosed)
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
        _registeredWithPool = false;
        _finishedCount = 0;
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

        Ice.ObjectAdapterI adapterImpl = (Ice.ObjectAdapterI)_adapter;
        _threadPerConnection = adapterImpl.getThreadPerConnection();

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
		    connection = new Ice.ConnectionI(_instance, _transceiver, _endpoint, _adapter, 
                                                     _threadPerConnection);
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

                if(_threadPerConnection)
                {
                    //
                    // If we are in thread per connection mode, we also use
                    // one thread per incoming connection factory, that
                    // accepts new connections on this endpoint.
                    //
                    try
                    {
                        _threadPerIncomingConnectionFactory = new ThreadPerIncomingConnectionFactory();
                        _threadPerIncomingConnectionFactory.start();
                    }
                    catch(java.lang.Exception ex)
                    {
                        error("cannot create thread for incoming connection factory", ex);
                        throw ex;
                    }
                }
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
                _threadPerIncomingConnectionFactory = null;
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
        IceUtil.Assert.FinalizerAssert(_state == StateClosed);
        IceUtil.Assert.FinalizerAssert(_acceptor == null);
        IceUtil.Assert.FinalizerAssert(_connections == null);
        IceUtil.Assert.FinalizerAssert(_threadPerIncomingConnectionFactory == null);

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
                if(!_threadPerConnection && _acceptor != null)
                {
                    registerWithPool();
                }

                java.util.ListIterator p = _connections.listIterator();
                while(p.hasNext())
                {
                    Ice.ConnectionI connection = (Ice.ConnectionI)p.next();
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
                if(!_threadPerConnection && _acceptor != null)
                {
                    unregisterWithPool();
                }

                java.util.ListIterator p = _connections.listIterator();
                while(p.hasNext())
                {
                    Ice.ConnectionI connection = (Ice.ConnectionI)p.next();
                    connection.hold();
                }
                break;
            }

            case StateClosed:
            {
                if(_acceptor != null)
                {
                    if(_threadPerConnection)
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

                java.util.ListIterator p = _connections.listIterator();
                while(p.hasNext())
                {   
                    Ice.ConnectionI connection = (Ice.ConnectionI)p.next();
                    connection.destroy(Ice.ConnectionI.ObjectAdapterDeactivated);
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
        assert(!_threadPerConnection); // Only for use with a thread pool.
        assert(_acceptor != null);

        if(!_registeredWithPool)
        {
            ((Ice.ObjectAdapterI)_adapter).getThreadPool()._register(_acceptor.fd(), this);
            _registeredWithPool = true;
        }
    }

    private void
    unregisterWithPool()
    {
        assert(!_threadPerConnection); // Only for use with a thread pool.
        assert(_acceptor != null);

        if(_registeredWithPool)
        {
            ((Ice.ObjectAdapterI)_adapter).getThreadPool().unregister(_acceptor.fd());
            _registeredWithPool = false;
            ++_finishedCount; // For each unregistration, finished() is called once.
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
        _instance.initializationData().logger.warning(s);
    }

    private void
    error(String msg, Exception ex)
    {
        java.io.StringWriter sw = new java.io.StringWriter();
        java.io.PrintWriter pw = new java.io.PrintWriter(sw);
        ex.printStackTrace(pw);
        pw.flush();
        String s = msg + ":\n" + toString() + "\n" + sw.toString();
        _instance.initializationData().logger.error(s);
    }

    private void
    run()
    {
        assert(_acceptor != null);

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
                // Ignore socket exceptions.
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
            
            Ice.ConnectionI connection = null;
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

                assert(_state == StateActive);

                //
                // Reap connections for which destruction has completed.
                //
                java.util.ListIterator p = _connections.listIterator();
                while(p.hasNext())
                {
                    Ice.ConnectionI con = (Ice.ConnectionI)p.next();
                    if(con.isFinished())
                    {
                        p.remove();
                    }
                }

                if(transceiver == null)
                {
                    continue;
                }

                try
                {
                    connection = new Ice.ConnectionI(_instance, transceiver, _endpoint, _adapter, _threadPerConnection);
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
                    continue;
                }
                _connections.add(connection);
            }

            //
            // In thread-per-connection mode and regardless of the background mode, 
            // start() doesn't block. The connection thread is started and takes 
            // care of the connection validation and notifies the factory through
            // the callback when it's done.
            //
            connection.start(this);
        }
    }

    private class ThreadPerIncomingConnectionFactory extends Thread
    {
        public void
        run()
        {
            try
            {
                IncomingConnectionFactory.this.run();
            }
            catch(Exception ex)
            {
                IncomingConnectionFactory.this.error("exception in thread per incoming connection factory", ex);
            }
        }
    }
    private Thread _threadPerIncomingConnectionFactory;

    private Acceptor _acceptor;
    private final Transceiver _transceiver;
    private EndpointI _endpoint;

    private Ice.ObjectAdapter _adapter;

    private boolean _registeredWithPool;
    private int _finishedCount;

    private final boolean _warn;

    private java.util.LinkedList _connections = new java.util.LinkedList();

    private int _state;

    private final boolean _threadPerConnection;
}
