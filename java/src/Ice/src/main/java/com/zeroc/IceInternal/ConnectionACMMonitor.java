// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceInternal;

class ConnectionACMMonitor implements ACMMonitor
{
    ConnectionACMMonitor(FactoryACMMonitor parent, java.util.concurrent.ScheduledExecutorService timer,
        ACMConfig config)
    {
        _parent = parent;
        _timer = timer;
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
            com.zeroc.IceUtilInternal.Assert.FinalizerAssert(_connection == null);
        }
        catch(java.lang.Exception ex)
        {
        }
        finally
        {
            super.finalize();
        }
    }

    @Override
    public synchronized void
    add(com.zeroc.Ice.ConnectionI connection)
    {
        assert(_connection == null);
        _connection = connection;
        if(_config.timeout > 0)
        {
            _future = _timer.scheduleAtFixedRate(() -> { monitorConnection(); },
                                                 _config.timeout / 2,
                                                 _config.timeout / 2,
                                                 java.util.concurrent.TimeUnit.MILLISECONDS);
        }
    }

    @Override
    public synchronized void
    remove(com.zeroc.Ice.ConnectionI connection)
    {
        assert(_connection == connection);
        _connection = null;
        if(_config.timeout > 0)
        {
            _future.cancel(false);
            _future = null;
        }
    }

    @Override
    public void
    reap(com.zeroc.Ice.ConnectionI connection)
    {
        _parent.reap(connection);
    }

    @Override
    public ACMMonitor
    acm(java.util.OptionalInt timeout, java.util.Optional<com.zeroc.Ice.ACMClose> close,
        java.util.Optional<com.zeroc.Ice.ACMHeartbeat> heartbeat)
    {
        return _parent.acm(timeout, close, heartbeat);
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

    private void
    monitorConnection()
    {
        com.zeroc.Ice.ConnectionI connection;
        synchronized(this)
        {
            if(_connection == null)
            {
                return;
            }
            connection = _connection;
        }

        try
        {
            connection.monitor(Time.currentMonotonicTimeMillis(), _config);
        }
        catch(Exception ex)
        {
            _parent.handleException(ex);
        }
    }

    final private FactoryACMMonitor _parent;
    final private java.util.concurrent.ScheduledExecutorService _timer;
    private java.util.concurrent.Future<?> _future;
    final private ACMConfig _config;

    private com.zeroc.Ice.ConnectionI _connection;
}
