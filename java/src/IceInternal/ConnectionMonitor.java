// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public final class ConnectionMonitor extends Thread
{
    //
    // Renamed from destroy to _destroy to avoid a deprecation warning caused
    // by the destroy method inherited from Thread.
    //
    public void
    _destroy()
    {
        synchronized(this)
        {
            assert(_instance != null);
            
            _instance = null;
            _connections = null;
        
            notify();
        }
        
        while(true)
        {
            try
            {
                join();
                break;
            }
            catch(java.lang.InterruptedException ex)
            {
                continue;
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

        String threadName = _instance.initializationData().properties.getProperty("Ice.ProgramName");
        if(threadName.length() > 0)
        {
            threadName += "-";
        }
        setName(threadName + "Ice.ConnectionMonitor");

        assert(_interval > 0);
        start();
    }

    protected synchronized void
    finalize()
        throws Throwable
    {
        IceUtil.Assert.FinalizerAssert(_instance == null);
        IceUtil.Assert.FinalizerAssert(_connections == null);
        
        super.finalize();
    }

    public void
    run()
    {
        java.util.HashSet connections = new java.util.HashSet();

        while(true)
        {
            synchronized(this)
            {
                if(_instance != null)
                {
                    try
                    {
                        wait(_interval * 1000);
                    }
                    catch(InterruptedException ex)
                    {
                        continue;
                    }
                }

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
            java.util.Iterator iter = connections.iterator();
            while(iter.hasNext())
            {
                Ice.ConnectionI connection = (Ice.ConnectionI)iter.next();

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
                        String s = "exception in connection monitor thread " + getName() + ":\n" +
                            sw.toString();
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
                        String s = "unknown exception in connection monitor thread " + getName() + ":\n" +
                            sw.toString();
                        _instance.initializationData().logger.error(s);
                    }
                }
            }
        }
    }
    
    private Instance _instance;
    private final int _interval;
    private java.util.HashSet _connections = new java.util.HashSet();
}
