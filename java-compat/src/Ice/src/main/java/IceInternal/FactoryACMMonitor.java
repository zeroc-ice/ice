// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

class FactoryACMMonitor implements ACMMonitor
{
    static class Change
    {
        Change(Ice.ConnectionI connection, boolean remove)
        {
            this.connection = connection;
            this.remove = remove;
        }

        final Ice.ConnectionI connection;
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
            IceUtilInternal.Assert.FinalizerAssert(_instance == null);
            IceUtilInternal.Assert.FinalizerAssert(_connections.isEmpty());
            IceUtilInternal.Assert.FinalizerAssert(_changes.isEmpty());
            IceUtilInternal.Assert.FinalizerAssert(_reapedConnections.isEmpty());
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

            _instance.timer().schedule(new Runnable() {
                    @Override
                    public void run()
                    {
                        monitorConnections();
                    }
                }, 0, java.util.concurrent.TimeUnit.MILLISECONDS);
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
    add(Ice.ConnectionI connection)
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
                _future = _instance.timer().scheduleAtFixedRate(new Runnable() {
                    @Override
                    public void run()
                    {
                        monitorConnections();
                    }
                },
                _config.timeout / 2, _config.timeout / 2, java.util.concurrent.TimeUnit.MILLISECONDS);
            }
            else
            {
                _changes.add(new Change(connection, false));
            }
        }
    }

    @Override
    public void
    remove(Ice.ConnectionI connection)
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
    reap(Ice.ConnectionI connection)
    {
        _reapedConnections.add(connection);
    }

    @Override
    public synchronized ACMMonitor
    acm(Ice.IntOptional timeout, Ice.Optional<Ice.ACMClose> close, Ice.Optional<Ice.ACMHeartbeat> heartbeat)
    {
        assert(_instance != null);

        ACMConfig config = _config.clone();
        if(timeout != null && timeout.isSet())
        {
            config.timeout = timeout.get() * 1000; // To milliseconds
        }
        if(close != null && close.isSet())
        {
            config.close = close.get();
        }
        if(heartbeat != null && heartbeat.isSet())
        {
            config.heartbeat = heartbeat.get();
        }
        return new ConnectionACMMonitor(this, _instance.timer(), config);
    }

    @Override
    public Ice.ACM
    getACM()
    {
        Ice.ACM acm = new Ice.ACM();
        acm.timeout = _config.timeout / 1000;
        acm.close = _config.close;
        acm.heartbeat = _config.heartbeat;
        return acm;
    }

    synchronized java.util.List<Ice.ConnectionI>
    swapReapedConnections()
    {
        if(_reapedConnections.isEmpty())
        {
            return null;
        }
        java.util.List<Ice.ConnectionI> connections = _reapedConnections;
        _reapedConnections = new java.util.ArrayList<Ice.ConnectionI>();
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
        for(Ice.ConnectionI connection : _connections)
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

    private java.util.Set<Ice.ConnectionI> _connections = new java.util.HashSet<Ice.ConnectionI>();
    private java.util.List<Change> _changes = new java.util.ArrayList<Change>();
    private java.util.List<Ice.ConnectionI> _reapedConnections = new java.util.ArrayList<Ice.ConnectionI>();
    private java.util.concurrent.Future<?> _future;
}
