// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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

    public synchronized void
    updateConnectionObservers()
    {
        for(Ice.ConnectionI connection : _connections)
        {
            connection.updateObserver();
        }
    }

    public void
    waitUntilHolding()
        throws InterruptedException
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
                wait();
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
        for(Ice.ConnectionI connection : connections)
        {
            connection.waitUntilHolding();
        }
    }

    public void
    waitUntilFinished()
        throws InterruptedException
    {
        java.util.LinkedList<Ice.ConnectionI> connections = null;

        synchronized(this)
        {
            //
            // First we wait until the factory is destroyed. If we are using
            // an acceptor, we also wait for it to be closed.
            //
            while(_state != StateFinished)
            {
                wait();
            }

            //
            // Clear the OA. See bug 1673 for the details of why this is necessary.
            //
            _adapter = null;

            //
            // We want to wait until all connections are finished outside the
            // thread synchronization.
            //
            connections = new java.util.LinkedList<Ice.ConnectionI>(_connections);
        }

        if(connections != null)
        {
            for(Ice.ConnectionI connection : connections)
            {
                try
                {
                    connection.waitUntilFinished();
                }
                catch(InterruptedException e)
                {
                    //
                    // Force close all of the connections.
                    //
                    for(Ice.ConnectionI c : connections)
                    {
                        c.close(true);
                    }
                    throw e;
                }
            }
        }

        synchronized(this)
        {
            if(_transceiver != null)
            {
                assert(_connections.size() <= 1); // The connection isn't monitored or reaped.
            }
            else
            {
                // Ensure all the connections are finished and reapable at this point.
                java.util.List<Ice.ConnectionI> cons = _monitor.swapReapedConnections();
                assert((cons == null ? 0 : cons.size()) == _connections.size());
                if(cons != null)
                {
                    cons.clear();
                }
            }
            _connections.clear();
        }

        //
        // Must be destroyed outside the synchronization since this might block waiting for
        // a timer task to execute.
        //
        _monitor.destroy();
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
        for(Ice.ConnectionI connection : _connections)
        {
            if(connection.isActiveOrHolding())
            {
                connections.add(connection);
            }
        }

        return connections;
    }

    public void
    flushAsyncBatchRequests(CommunicatorFlushBatch outAsync)
    {
        for(Ice.ConnectionI c : connections()) // connections() is synchronized, no need to synchronize here.
        {
            try
            {
                outAsync.flushConnection(c);
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

    @Override
    public void
    message(ThreadPoolCurrent current)
    {
        Ice.ConnectionI connection = null;
        synchronized(this)
        {
            if(_state >= StateClosed)
            {
                return;
            }
            else if(_state == StateHolding)
            {
                Thread.yield();
                return;
            }

            //
            // Reap closed connections.
            //
            java.util.List<Ice.ConnectionI> cons = _monitor.swapReapedConnections();
            if(cons != null)
            {
                for(Ice.ConnectionI c : cons)
                {
                    _connections.remove(c);
                }
            }

            //
            // Now accept a new connection.
            //
            Transceiver transceiver = null;
            try
            {
                transceiver = _acceptor.accept();

                if(_instance.traceLevels().network >= 2)
                {
                    StringBuffer s = new StringBuffer("trying to accept ");
                    s.append(_endpoint.protocol());
                    s.append(" connection\n");
                    s.append(transceiver.toString());
                    _instance.initializationData().logger.trace(_instance.traceLevels().networkCat, s.toString());
                }
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
                connection = new Ice.ConnectionI(_adapter.getCommunicator(), _instance, _monitor, transceiver, null,
                                                 _endpoint, _adapter);
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

        assert(connection != null);
        connection.start(this);
    }

    @Override
    public synchronized void
    finished(ThreadPoolCurrent current, boolean close)
    {
        assert(_state == StateClosed);
        setState(StateFinished);

        if(close)
        {
            closeAcceptor();
        }
    }

    @Override
    public synchronized String
    toString()
    {
        if(_transceiver != null)
        {
            return _transceiver.toString();
        }
        return _acceptor.toString();
    }

    @Override
    public java.nio.channels.SelectableChannel
    fd()
    {
        assert(_acceptor != null);
        return _acceptor.fd();
    }

    //
    // Operations from ConnectionI.StartCallback
    //
    @Override
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

    @Override
    public synchronized void
    connectionStartFailed(Ice.ConnectionI connection, Ice.LocalException ex)
    {
        if(_state >= StateClosed)
        {
            return;
        }
        //
        // Do not warn about connection exceptions here. The connection is not yet validated.
        //
    }

    public
    IncomingConnectionFactory(Instance instance, EndpointI endpoint, Ice.ObjectAdapterI adapter)
    {
        _instance = instance;
        _endpoint = endpoint;
        _adapter = adapter;
        _warn = _instance.initializationData().properties.getPropertyAsInt("Ice.Warn.Connections") > 0 ? true : false;
        _state = StateHolding;
        _monitor = new FactoryACMMonitor(instance, adapter.getACM());

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
            _transceiver = _endpoint.transceiver();
            if(_transceiver != null)
            {
                if(_instance.traceLevels().network >= 2)
                {
                    StringBuffer s = new StringBuffer("attempting to bind to ");
                    s.append(_endpoint.protocol());
                    s.append(" socket\n");
                    s.append(_transceiver.toString());
                    _instance.initializationData().logger.trace(_instance.traceLevels().networkCat, s.toString());
                }
                _endpoint = _transceiver.bind();

                Ice.ConnectionI connection =
                    new Ice.ConnectionI(_adapter.getCommunicator(), _instance, null, _transceiver, null, _endpoint,
                                        _adapter);
                connection.startAndWait();

                _connections.add(connection);
            }
            else
            {
                createAcceptor();
            }
        }
        catch(java.lang.Exception ex)
        {
            //
            // Clean up for finalizer.
            //
            if(_transceiver != null)
            {
                try
                {
                    _transceiver.close();
                }
                catch(Ice.LocalException e)
                {
                    // Here we ignore any exceptions in close().
                }
            }

            _state = StateFinished;
            _monitor.destroy();
            _connections.clear();

            if(ex instanceof Ice.LocalException)
            {
                throw (Ice.LocalException)ex;
            }
            else if(ex instanceof InterruptedException)
            {
                throw new Ice.OperationInterruptedException();
            }
            else
            {
                throw new Ice.SyscallException(ex);
            }
        }
    }

    @Override
    protected synchronized void
    finalize()
        throws Throwable
    {
        try
        {
            IceUtilInternal.Assert.FinalizerAssert(_state == StateFinished);
            IceUtilInternal.Assert.FinalizerAssert(_connections.isEmpty());
        }
        catch(java.lang.Exception ex)
        {
        }
        finally
        {
            super.finalize();
        }
    }

    private static final int StateActive = 0;
    private static final int StateHolding = 1;
    private static final int StateClosed = 2;
    private static final int StateFinished = 3;

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
                    if(_instance.traceLevels().network >= 1)
                    {
                        StringBuffer s = new StringBuffer("accepting ");
                        s.append(_endpoint.protocol());
                        s.append(" connections at ");
                        s.append(_acceptor.toString());
                        _instance.initializationData().logger.trace(_instance.traceLevels().networkCat, s.toString());
                    }
                    _adapter.getThreadPool().register(this, SocketOperation.Read);
                }

                for(Ice.ConnectionI connection : _connections)
                {
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
                    if(_instance.traceLevels().network >= 1)
                    {
                        StringBuffer s = new StringBuffer("holding ");
                        s.append(_endpoint.protocol());
                        s.append(" connections at ");
                        s.append(_acceptor.toString());
                        _instance.initializationData().logger.trace(_instance.traceLevels().networkCat, s.toString());
                    }
                    _adapter.getThreadPool().unregister(this, SocketOperation.Read);
                }

                for(Ice.ConnectionI connection : _connections)
                {
                    connection.hold();
                }
                break;
            }

            case StateClosed:
            {
                if(_acceptor != null)
                {
                    //
                    // If possible, close the acceptor now to prevent new connections from
                    // being accepted while we are deactivating. This is especially useful
                    // if there are no more threads in the thread pool available to dispatch
                    // the finish() call.
                    //
                    if(_adapter.getThreadPool().finish(this, true))
                    {
                        closeAcceptor();
                    }
                }
                else
                {
                    state = StateFinished;
                }

                for(Ice.ConnectionI connection : _connections)
                {
                    connection.destroy(Ice.ConnectionI.ObjectAdapterDeactivated);
                }
                break;
            }

            case StateFinished:
            {
                assert(_state == StateClosed);
                break;
            }
        }

        _state = state;
        notifyAll();
    }

    private void
    createAcceptor()
    {
        try
        {
            _acceptor = _endpoint.acceptor(_adapter.getName());
            assert(_acceptor != null);

            if(_instance.traceLevels().network >= 2)
            {
                StringBuffer s = new StringBuffer("attempting to bind to ");
                s.append(_endpoint.protocol());
                s.append(" socket ");
                s.append(_acceptor.toString());
                _instance.initializationData().logger.trace(_instance.traceLevels().networkCat, s.toString());
            }

            _endpoint = _acceptor.listen();

            if(_instance.traceLevels().network >= 1)
            {
                StringBuffer s = new StringBuffer("listening for ");
                s.append(_endpoint.protocol());
                s.append(" connections\n");
                s.append(_acceptor.toDetailedString());
                _instance.initializationData().logger.trace(_instance.traceLevels().networkCat, s.toString());
            }

            _adapter.getThreadPool().initialize(this);

            if(_state == StateActive)
            {
                _adapter.getThreadPool().register(this, SocketOperation.Read);
            }
        }
        catch(Exception ex)
        {
            if(_acceptor != null)
            {
                _acceptor.close();
            }
            throw ex;
        }
    }

    private void
    closeAcceptor()
    {
        if(_instance.traceLevels().network >= 1)
        {
            StringBuffer s = new StringBuffer("stopping to accept ");
            s.append(_endpoint.protocol());
            s.append(" connections at ");
            s.append(_acceptor.toString());
            _instance.initializationData().logger.trace(_instance.traceLevels().networkCat, s.toString());
        }
        _acceptor.close();
    }

    private void
    warning(Ice.LocalException ex)
    {
        String s = "connection exception:\n" + Ex.toString(ex) + '\n' + _acceptor.toString();
        _instance.initializationData().logger.warning(s);
    }

    private final Instance _instance;
    private final FactoryACMMonitor _monitor;

    private Acceptor _acceptor;
    private Transceiver _transceiver;
    private EndpointI _endpoint;

    private Ice.ObjectAdapterI _adapter;

    private final boolean _warn;

    private java.util.Set<Ice.ConnectionI> _connections = new java.util.HashSet<Ice.ConnectionI>();

    private int _state;
}
