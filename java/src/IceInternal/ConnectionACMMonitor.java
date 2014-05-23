// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

class ConnectionACMMonitor implements ACMMonitor
{
    ConnectionACMMonitor(FactoryACMMonitor parent, Timer timer, ACMConfig config)
    {
        _parent = parent;
        _timer = timer;
        _config = config;
    }

    protected synchronized void
    finalize()
        throws Throwable
    {
        try
        {
            IceUtilInternal.Assert.FinalizerAssert(_connection == null);
        }
        catch(java.lang.Exception ex)
        {
        }
        finally
        {
            super.finalize();
        }
    }

    public synchronized void
    add(Ice.ConnectionI connection)
    {
        assert(_connection == null);
        _connection = connection;
        if(_config.timeout > 0)
        {
            _timer.scheduleRepeated(this, _config.timeout / 2);
        }
    }

    public synchronized void
    remove(Ice.ConnectionI connection)
    {
        assert(_connection == connection);
        _connection = null;
        if(_config.timeout > 0)
        {
            _timer.cancel(this);
        }
    }
    
    public void
    reap(Ice.ConnectionI connection)
    {
        _parent.reap(connection);
    }
    
    public ACMMonitor
    acm(Ice.IntOptional timeout, Ice.Optional<Ice.ACMClose> close, Ice.Optional<Ice.ACMHeartbeat> heartbeat)
    {
        return _parent.acm(timeout, close, heartbeat);
    }
    
    public Ice.ACM
    getACM()
    {
        Ice.ACM acm = new Ice.ACM();
        acm.timeout = _config.timeout / 1000;
        acm.close = _config.close;
        acm.heartbeat = _config.heartbeat;
        return acm;
    }
    
    public void
    runTimerTask()
    {
        Ice.ConnectionI connection;
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
    final private Timer _timer;
    final private ACMConfig _config;

    private Ice.ConnectionI _connection;
};
