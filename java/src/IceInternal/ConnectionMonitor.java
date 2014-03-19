// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public final class ConnectionMonitor implements IceInternal.TimerTask
{
    //
    // Renamed from destroy to _destroy to avoid a deprecation warning caused
    // by the destroy method inherited from Thread.
    //
    synchronized public void
    destroy()
    {
        assert(_instance != null);
        _instance = null;
        _connections = null;
    }

    public void
    checkIntervalForACM(int acmTimeout)
    {
        if(acmTimeout <= 0)
        {
            return;
        }

        //
        // If Ice.MonitorConnections isn't set (_interval == 0), the given ACM is used
        // to determine the check interval: 1/10 of the ACM timeout with a minmal value
        // of 5 seconds and a maximum value of 5 minutes.
        //
        // Note: if Ice.MonitorConnections is set, the timer is schedulded only if ACM 
        // is configured for the communicator or some object adapters.
        //
        int interval;
        if(_interval == 0)
        {
            interval = java.lang.Math.min(300, java.lang.Math.max(5, (int)acmTimeout / 10));
        }
        else if(_scheduledInterval == _interval)
        {
            return; // Nothing to do, the timer is already scheduled.
        }
        else
        {
            interval = _interval;
        }

        //
        // If no timer is scheduled yet or if the given ACM requires a smaller interval,
        // we re-schedule the timer.
        //
        synchronized(this)
        {
            if(_scheduledInterval == 0 || _scheduledInterval > interval)
            {
                _scheduledInterval = interval;
                _instance.timer().cancel(this);
                _instance.timer().scheduleRepeated(this, interval * 1000);
            }
        }
    }
        
    public synchronized void
    add(Ice.ConnectionI connection)
    {
        assert(_instance != null);
        _connections.add(connection);
    }
    
    public synchronized void
    remove(Ice.ConnectionI connection)
    {
        assert(_instance != null);
        _connections.remove(connection);
    }

    //
    // Only for use by Instance.
    //
    ConnectionMonitor(Instance instance, int interval)
    {
        _instance = instance;
        _interval = interval;
        _scheduledInterval = 0;
    }

    protected synchronized void
    finalize()
        throws Throwable
    {
        try
        {
            IceUtilInternal.Assert.FinalizerAssert(_instance == null);
            IceUtilInternal.Assert.FinalizerAssert(_connections == null);
        }
        catch(java.lang.Exception ex)
        {
        }
        finally
        {
            super.finalize();
        }
    }

    public void
    runTimerTask()
    {
        java.util.Set<Ice.ConnectionI> connections = new java.util.HashSet<Ice.ConnectionI>();

        synchronized(this)
        {
            if(_instance == null)
            {
                return;
            }
            
            connections.clear();
            connections.addAll(_connections);
        }
        
        //
        // Monitor connections outside the thread synchronization,
        // so that connections can be added or removed during
        // monitoring.
        //
        long now = IceInternal.Time.currentMonotonicTimeMillis();
        for(Ice.ConnectionI conn : connections)
        {
            try
            {              
                conn.monitor(now);
            }
            catch(java.lang.Exception ex)
            {
                synchronized(this)
                {
                    if(_instance == null)
                    {
                        return;
                    }
                    String s = "exception in connection monitor:\n" + Ex.toString(ex);
                    _instance.initializationData().logger.error(s);
                }
            }
        }
    }
    
    private Instance _instance;
    private final int _interval;
    private int _scheduledInterval;
    private java.util.Set<Ice.ConnectionI> _connections = new java.util.HashSet<Ice.ConnectionI>();
}
