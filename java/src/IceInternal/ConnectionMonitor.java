// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

package IceInternal;

public final class ConnectionMonitor extends Thread
{
    public void
    destroy()
    {
	synchronized(this)
	{
	    assert(_instance != null);
	    
	    _instance = null;
	    _connections.clear();
	
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
    add(Connection connection)
    {
	assert(_instance != null);
	_connections.add(connection);
    }
    
    public synchronized void
    remove(Connection connection)
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

        String threadName = _instance.properties().getProperty("Ice.ProgramName");
        if(threadName.length() > 0)
        {
            threadName += "-";
        }
	setName(threadName + "Ice.ConnectionMonitor");

	assert(_interval > 0);
	start();
    }

    protected void
    finalize()
        throws Throwable
    {
	assert(_instance == null);
	assert(_connections.isEmpty());
	
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
		if(_instance == null)
		{
		    return;
		}
		
		try
		{
		    wait(_interval * 1000);
		}
		catch(InterruptedException ex)
		{
		    continue;
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
		Connection connection = (Connection)iter.next();

		try
		{	       
		    connection.monitor();
		}
		catch(Ice.LocalException ex)
		{
		    synchronized(this)
		    {
			java.io.StringWriter sw = new java.io.StringWriter();
			java.io.PrintWriter pw = new java.io.PrintWriter(sw);
			ex.printStackTrace(pw);
			pw.flush();
			String s = "exception in thread pool thread " + getName() + ":\n" + sw.toString();
			_instance.logger().error(s);
		    }
		}
		catch(RuntimeException ex)
		{
		    synchronized(this)
		    {
			java.io.StringWriter sw = new java.io.StringWriter();
			java.io.PrintWriter pw = new java.io.PrintWriter(sw);
			ex.printStackTrace(pw);
			pw.flush();
			String s = "unknown exception in thread pool thread " + getName() + ":\n" + sw.toString();
			_instance.logger().error(s);
		    }
		}
	    }
	}
    }
    
    private Instance _instance;
    private final int _interval;
    private java.util.HashSet _connections = new java.util.HashSet();
}
