// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{

    using System.Diagnostics;
    using System.Threading;
    using IceUtil;

    public sealed class ConnectionMonitor
    {
	public void destroy()
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
		_thread.Join();
		break;
	    }
	}
	
	public void add(Ice.ConnectionI connection)
	{
	    lock(this)
	    {
		Debug.Assert(_instance != null);
		_connections.Add(connection);
	    }
	}
	
	public void remove(Ice.ConnectionI connection)
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
	internal ConnectionMonitor(Instance instance, int interval)
	{
	    _instance = instance;
	    _interval = interval;
	    _connections = new Set();
	    
	    string threadName = _instance.properties().getProperty("Ice.ProgramName");
	    if(threadName.Length > 0)
	    {
		threadName += "-";
	    }
	    
	    Debug.Assert(_interval > 0);

	    _thread = new Thread(new ThreadStart(Run));
	    _thread.Name = threadName + "Ice.ConnectionMonitor";
	    _thread.Start();
	}
	
	~ConnectionMonitor()
	{
	    Debug.Assert(_instance == null);
	    Debug.Assert(_connections.Count == 0);
	}
	
	public void Run()
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
		    
		    System.Threading.Monitor.Wait(this, System.TimeSpan.FromMilliseconds(_interval * 1000));
		    
		    if(_instance == null)
		    {
			return;
		    }
		    
		    connections.Clear();
		    foreach(Ice.ConnectionI connection in _connections)
		    {
			connections.Add(connection);
		    }
		}
		
		//
		// Monitor connections outside the thread synchronization,
		// so that connections can be added or removed during
		// monitoring.
		//
		foreach(Ice.ConnectionI connection in connections)
		{
		    try
		    {
			connection.monitor();
		    }
		    catch(Ice.LocalException ex)
		    {
			lock(this)
			{
			    _instance.logger().error("exception in connection monitor thread " + _thread.Name + "::\n" + ex);
			}
		    }
		    catch(System.Exception ex)
		    {
			lock(this)
			{
			    _instance.logger().error("unknown exception in connection monitor thread " + _thread.Name + ":\n" + ex);
			}
		    }
		}
	    }
	}
	
	private Instance _instance;
	private readonly int _interval;
	private Set _connections;
	private Thread _thread;
    }

}
