// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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
        for(Ice.ConnectionI connection : connections)
        {
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
            while(_state != StateFinished)
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
            connections = new java.util.LinkedList<Ice.ConnectionI>(_connections);
        }

        if(connections != null)
        {
            for(Ice.ConnectionI connection : connections)
            {
                connection.waitUntilFinished();
            }
        }

        synchronized(this)
        {
            // Ensure all the connections are finished and reapable at this point.
            java.util.List<Ice.ConnectionI> cons = _reaper.swapConnections();
            assert((cons == null ? 0 : cons.size()) == _connections.size());
            if(cons != null)
            {
                cons.clear();
            }
            _connections.clear();
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
    flushAsyncBatchRequests(CommunicatorBatchOutgoingAsync outAsync)
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
            java.util.List<Ice.ConnectionI> cons = _reaper.swapConnections();
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
                connection = new Ice.ConnectionI(_adapter.getCommunicator(), _instance, _reaper, transceiver, null,
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

    public synchronized void
    finished(ThreadPoolCurrent current)
    {
        assert(_state == StateClosed);
        setState(StateFinished);
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
        if(_state >= StateClosed)
        {
            return;
        }
        //
        // Do not warn about connection exceptions here. The connection is not yet validated.
        //
    }

    public
    IncomingConnectionFactory(Instance instance, EndpointI endpoint, Ice.ObjectAdapter adapter, String adapterName)
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
                Ice.ConnectionI connection =
                    new Ice.ConnectionI(_adapter.getCommunicator(), _instance, _reaper, _transceiver, null, _endpoint,
                                        _adapter);
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
                ((Ice.ObjectAdapterI)_adapter).getThreadPool().initialize(this);
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

            _state = StateFinished;
            _connections.clear();

            if(ex instanceof Ice.LocalException)
            {
                throw (Ice.LocalException)ex;
            }
            else
            {
                throw new Ice.SyscallException(ex);
            }
        }
    }

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
                    ((Ice.ObjectAdapterI)_adapter).getThreadPool().register(this, SocketOperation.Read);
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
                    ((Ice.ObjectAdapterI)_adapter).getThreadPool().unregister(this, SocketOperation.Read);
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
                    ((Ice.ObjectAdapterI)_adapter).getThreadPool().finish(this);
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
                if(_acceptor != null)
                {
                    _acceptor.close();
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
        String s = "connection exception:\n" + Ex.toString(ex) + '\n' + _acceptor.toString();
        _instance.initializationData().logger.warning(s);
    }

    private final Instance _instance;
    private final ConnectionReaper _reaper = new ConnectionReaper();

    private Acceptor _acceptor;
    private Transceiver _transceiver;
    private EndpointI _endpoint;

    private Ice.ObjectAdapter _adapter;

    private final boolean _warn;

    private java.util.Set<Ice.ConnectionI> _connections = new java.util.HashSet<Ice.ConnectionI>();

    private int _state;
}
