//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceInternal;

import com.zeroc.Ice.Annotations.*;

class FactoryACMMonitor implements ACMMonitor
{
    static class Change
    {
        Change(com.zeroc.Ice.ConnectionI connection, boolean remove)
        {
            this.connection = connection;
            this.remove = remove;
        }

        final com.zeroc.Ice.ConnectionI connection;
        final boolean remove;
    }

    FactoryACMMonitor(Instance instance, ACMConfig config)
    {
        _instance = instance;
        _config = config;
    }

    @SuppressWarnings("deprecation")
    @Override
    protected synchronized void
    finalize()
        throws Throwable
    {
        try
        {
            com.zeroc.IceUtilInternal.Assert.FinalizerAssert(_instance == null);
            com.zeroc.IceUtilInternal.Assert.FinalizerAssert(_connections.isEmpty());
            com.zeroc.IceUtilInternal.Assert.FinalizerAssert(_changes.isEmpty());
            com.zeroc.IceUtilInternal.Assert.FinalizerAssert(_reapedConnections.isEmpty());
        }
        catch(java.lang.Exception ex)
        {
        }
        finally
        {
            super.finalize();
        }
    }

    synchronized void
    destroy()
    {
        if(_instance == null)
        {
            //
            // Ensure all the connections have been cleared, it's important to wait here
            // to prevent the timer destruction in IceInternal::Instance::destroy.
            //
            while(!_connections.isEmpty())
            {
                try
                {
                    wait();
                }
                catch(InterruptedException ex)
                {
                }
            }
            return;
        }

        if(!_connections.isEmpty())
        {
            //
            // Cancel the scheduled timer task and schedule it again now to clear the
            // connection set from the timer thread.
            //
            assert(_future != null);
            _future.cancel(false);
            _future = null;

            _instance.timer().schedule(() -> { monitorConnections(); }, 0, java.util.concurrent.TimeUnit.MILLISECONDS);
        }

        _instance = null;
        _changes.clear();

        //
        // Wait for the connection set to be cleared by the timer thread.
        //
        while(!_connections.isEmpty())
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

    @Override
    public void
    add(com.zeroc.Ice.ConnectionI connection)
    {
        if(_config.timeout == 0)
        {
            return;
        }

        synchronized(this)
        {
            assert(_instance != null);
            if(_connections.isEmpty())
            {
                _connections.add(connection);
                assert _future == null;
                _future = _instance.timer().scheduleAtFixedRate(() -> { monitorConnections(); },
                                                                _config.timeout / 2,
                                                                _config.timeout / 2,
                                                                java.util.concurrent.TimeUnit.MILLISECONDS);
            }
            else
            {
                _changes.add(new Change(connection, false));
            }
        }
    }

    @Override
    public void
    remove(com.zeroc.Ice.ConnectionI connection)
    {
        if(_config.timeout == 0)
        {
            return;
        }

        synchronized(this)
        {
            assert(_instance != null);
            _changes.add(new Change(connection, true));
        }
    }

    @Override
    public synchronized void
    reap(com.zeroc.Ice.ConnectionI connection)
    {
        _reapedConnections.add(connection);
    }

    @Override
    public synchronized ACMMonitor
    acm(@Nullable Integer timeout, com.zeroc.Ice.@Nullable ACMClose close,
        com.zeroc.Ice.@Nullable ACMHeartbeat heartbeat)
    {
        assert(_instance != null);

        ACMConfig config = _config.clone();
        if(timeout != null)
        {
            config.timeout = timeout * 1000; // To milliseconds
        }
        if(close != null)
        {
            config.close = close;
        }
        if(heartbeat != null)
        {
            config.heartbeat = heartbeat;
        }
        return new ConnectionACMMonitor(this, _instance.timer(), config);
    }

    @Override
    public com.zeroc.Ice.ACM
    getACM()
    {
        com.zeroc.Ice.ACM acm = new com.zeroc.Ice.ACM();
        acm.timeout = _config.timeout / 1000;
        acm.close = _config.close;
        acm.heartbeat = _config.heartbeat;
        return acm;
    }

    synchronized java.util.List<com.zeroc.Ice.ConnectionI>
    swapReapedConnections()
    {
        if(_reapedConnections.isEmpty())
        {
            return null;
        }
        java.util.List<com.zeroc.Ice.ConnectionI> connections = _reapedConnections;
        _reapedConnections = new java.util.ArrayList<>();
        return connections;
    }

    private void
    monitorConnections()
    {
        synchronized(this)
        {
            if(_instance == null)
            {
                _connections.clear();
                notifyAll();
                return;
            }

            for(Change change : _changes)
            {
                if(change.remove)
                {
                    _connections.remove(change.connection);
                }
                else
                {
                    _connections.add(change.connection);
                }
            }
            _changes.clear();

            if(_connections.isEmpty())
            {
                _future.cancel(false);
                _future = null;
                return;
            }
        }

        //
        // Monitor connections outside the thread synchronization, so
        // that connections can be added or removed during monitoring.
        //
        long now = Time.currentMonotonicTimeMillis();
        for(com.zeroc.Ice.ConnectionI connection : _connections)
        {
            try
            {
                connection.monitor(now, _config);
            }
            catch(Exception ex)
            {
                handleException(ex);
            }
        }
    }

    synchronized void
    handleException(Exception ex)
    {
        if(_instance == null)
        {
            return;
        }
        _instance.initializationData().logger.error("exception in connection monitor:\n" + ex);
    }

    private Instance _instance;
    final private ACMConfig _config;

    private java.util.Set<com.zeroc.Ice.ConnectionI> _connections = new java.util.HashSet<>();
    private java.util.List<Change> _changes = new java.util.ArrayList<>();
    private java.util.List<com.zeroc.Ice.ConnectionI> _reapedConnections = new java.util.ArrayList<>();
    private java.util.concurrent.Future<?> _future;
}
