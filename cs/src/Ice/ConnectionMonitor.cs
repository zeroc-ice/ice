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

namespace IceInternal
{

using System.Diagnostics;
using IceUtil;

public sealed class ConnectionMonitor : SupportClass.ThreadClass
{
    //UPGRADE_TODO: The equivalent of method 'java.lang.Thread.destroy' is not an override method. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1143"'
    public void  destroy()
    {
	lock(this)
	{
	    Debug.Assert(_instance != null);
	    
	    _instance = null;
	    _connections.Clear();
	    
	    System.Threading.Monitor.Pulse(this);
	}
	
	while(true)
	{
	    try
	    {
		Join();
		break;
	    }
	    catch(System.Threading.ThreadInterruptedException)
	    {
		continue;
	    }
	}
    }
    
    public void
    add(Connection connection)
    {
	lock(this)
	{
	    Debug.Assert(_instance != null);
	    _connections.Add(connection);
	}
    }
    
    public void
    remove(Connection connection)
    {
	lock(this)
	{
	    Debug.Assert(_instance != null);
	    _connections.Remove(connection);
	}
    }
    
    //
    // Only for use by Instance.
    //
    internal
    ConnectionMonitor(Instance instance, int interval)
    {
	_instance = instance;
	_interval = interval;
	_connections = new Set();
	
	string threadName = _instance.properties().getProperty("Ice.ProgramName");
	if(threadName.Length > 0)
	{
		threadName += "-";
	}
	Name = threadName + "Ice.ConnectionMonitor";
	
	Debug.Assert(_interval > 0);
	Start();
    }
    
    ~ConnectionMonitor()
    {
	Debug.Assert(_instance == null);
	Debug.Assert(_connections.Count == 0);
    }
    
    //UPGRADE_TODO: The equivalent of method 'java.lang.Thread.run' is not an override method. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1143"'
    override public void
    Run()
    {
	Set connections = new Set();
	
	while(true)
	{
	    lock(this)
	    {
		if(_instance == null)
		{
		    return;
		}
		
		try
		{
		    System.Threading.Monitor.Wait(this, System.TimeSpan.FromMilliseconds(_interval * 1000));
		}
		catch(System.Threading.ThreadInterruptedException)
		{
		    continue;
		}
		
		if(_instance == null)
		{
		    return;
		}
		
		connections.Clear();
		foreach(Connection connection in _connections)
		{
		    connections.Add(connection);
		}
	    }
	    
	    //
	    // Monitor connections outside the thread synchronization,
	    // so that connections can be added or removed during
	    // monitoring.
	    //
	    foreach(Connection connection in connections)
	    {
		try
		{
		    connection.monitor();
		}
		catch(Ice.LocalException ex)
		{
		    lock(this)
		    {
			_instance.logger().error("exception in connection monitor thread " + Name + "::\n" + ex);
		    }
		}
		catch(System.Exception ex)
		{
		    lock(this)
		    {
			_instance.logger().error("unknown exception in connection monitor thread " + Name + ":\n" + ex);
		    }
		}
	    }
	}
    }
    
    private Instance _instance;
    private readonly int _interval;
    private Set _connections;
}

}
