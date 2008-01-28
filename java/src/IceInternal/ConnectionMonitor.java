// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
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
        assert(interval > 0);
        _instance = instance;

        _instance.timer().scheduleRepeated(this, interval * 1000);
    }

    protected synchronized void
    finalize()
        throws Throwable
    {
        IceUtilInternal.Assert.FinalizerAssert(_instance == null);
        IceUtilInternal.Assert.FinalizerAssert(_connections == null);
        
        super.finalize();
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
        java.util.Iterator<Ice.ConnectionI> iter = connections.iterator();
        while(iter.hasNext())
        {
            Ice.ConnectionI connection = iter.next();
            
            try
            {              
                connection.monitor();
            }
            catch(Ice.LocalException ex)
            {
                synchronized(this)
                {
                    if(_instance == null)
                    {
                        return;
                    }
                    
                    java.io.StringWriter sw = new java.io.StringWriter();
                    java.io.PrintWriter pw = new java.io.PrintWriter(sw);
                    ex.printStackTrace(pw);
                    pw.flush();
                    String s = "exception in connection monitor:\n" + sw.toString();
                    _instance.initializationData().logger.error(s);
                }
            }
            catch(java.lang.Exception ex)
            {
                synchronized(this)
                {
                    if(_instance == null)
                    {
                        return;
                    }
                    java.io.StringWriter sw = new java.io.StringWriter();
                    java.io.PrintWriter pw = new java.io.PrintWriter(sw);
                    ex.printStackTrace(pw);
                    pw.flush();
                    String s = "unknown exception in connection monitor:\n" + sw.toString();
                    _instance.initializationData().logger.error(s);
                }
            }
        }
    }
    
    private Instance _instance;
    private java.util.Set<Ice.ConnectionI> _connections = new java.util.HashSet<Ice.ConnectionI>();
}
