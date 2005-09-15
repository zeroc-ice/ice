// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Ice
{

    using System;
    using System.Diagnostics;
    using System.Runtime.InteropServices;
    using System.Threading;

    public abstract class Application
    {
	public abstract int run(string[] args);

	public Application()
	{
#if !__MonoCS__
	    bool rc;
	    rc = SetConsoleCtrlHandler(_handler, true); 
	    Debug.Assert(rc);
#endif
	    _callback = null;
	    _callbackInProgress = false;
	    _destroyed = false;
	    _released = false;
	    _interrupted = false;
	    _nohup = false;
	}
	
	//
	// This main() must be called by the global Main(). main()
	// initializes the Communicator, calls run(), and destroys
	// the Communicator upon return from run(). It thereby handles
	// all exceptions properly, i.e., error messages are printed
	// if exceptions propagate to main(), and the Communicator is
	// always destroyed, regardless of exceptions.
	//
	public int main(string[] args)
	{
	    return main(args, null);
	}
	
	public int main(string[] args, string configFile)
	{
	    if(_communicator != null)
	    {
		Console.Error.WriteLine(_appName + ": only one instance of the Application class can be used");
		return 1;
	    }	
	    int status = 0;
	    
	    try
	    {
		if(configFile != null)
		{
		    Properties properties = Util.createProperties();
		    properties.load(configFile);
		    _communicator = Util.initializeWithProperties(ref args, properties);
		}
		else
		{
		    _communicator = Util.initialize(ref args);
		}
		
		Properties props = _communicator.getProperties();
		_nohup = props.getPropertyAsInt("Ice.Nohup") != 0;
		_appName = props.getProperty("Ice.ProgramName");

		//
		// The default is to destroy when a signal is received.
		//
		destroyOnInterrupt();
		
		status = run(args);
	    }
	    catch(Ice.Exception ex)
	    {
		Console.Error.WriteLine(_appName + ": " + ex);
		status = 1;
	    }
	    catch(System.Exception ex)
	    {
		Console.Error.WriteLine(_appName + ": unknown exception: " + ex);
		status = 1;
	    }

	    //
	    // Don't want any new interrupt. And at this point (post-run), it would not make sense to release a
	    // held signal to run shutdown or destroy.
	    //
	    ignoreInterrupt();

	    Monitor.Enter(sync);
	    while(_callbackInProgress)
	    {
	        Monitor.Wait(sync);
	    }
	    if(_destroyed)
	    {
	        _communicator = null;
	    }
	    else
	    {
	        _destroyed = true;
		//
		// And _communicator != 0, meaning will be destroyed next,
		// _destroyed = true also ensures that any remaining callback won't do anything
		//
	    }
	    Monitor.Exit(sync);

	    if(_communicator != null)
	    {
		try
		{
		    _communicator.destroy();
		}
		catch(Ice.Exception ex)
		{
		    Console.Error.WriteLine(_appName + ": " + ex);
		    status = 1;
		}
		catch(System.Exception ex)
		{
		    Console.Error.WriteLine(_appName + ": unknown exception: " + ex);
		    status = 1;
		}
		_communicator = null;
	    }	
	    return status;
	}
	

	
	//
	// Return the application name.
	//
	public static string appName()
	{
	    return _appName;
	}
	
	//
	// One limitation of this class is that there can only be one
	// Application instance, with one global Communicator, accessible
	// with this communicator() operation. This limitiation is due to
	// how the signal handling functions below operate. If you require
	// multiple Communicators, then you cannot use this Application
	// framework class.
	//
	public static Communicator communicator()
	{
	    return _communicator;
	}
	
	public static void destroyOnInterrupt()
	{   
	    Monitor.Enter(sync);
	    if(_callback == _holdCallback)
	    {
		_callback = _destroyCallback;
		_released = true;
		Monitor.Pulse(sync);
	    }
	    else
	    {
		_callback = _destroyCallback;
	    }
	    Monitor.Exit(sync);
	}
	
	public static void shutdownOnInterrupt()
	{
	    Monitor.Enter(sync);
	    if(_callback == _holdCallback)
	    {
		_callback = _shutdownCallback;
		_released = true;
		Monitor.Pulse(sync);
	    }
	    else
	    {
		_callback = _shutdownCallback;
	    }
	    Monitor.Exit(sync);
	}
	
	public static void ignoreInterrupt()
	{
	    Monitor.Enter(sync);
	    if(_callback == _holdCallback)
	    {
		_callback = null;
		_released = true;
		Monitor.Pulse(sync);
	    }
	    else
	    {
		_callback = null;
	    }
	    Monitor.Exit(sync);
	}
	
	public static void holdInterrupt()
	{
	    Monitor.Enter(sync);
	    if(_callback != _holdCallback)
	    {
		_previousCallback = _callback;
		_callback = _holdCallback;
		_released = true;
	    }
	    // else, we were already holding signals
	    Monitor.Exit(sync);
	}
	
	public static void releaseInterrupt()
	{
	    Monitor.Enter(sync);
	    if(_callback == _holdCallback)
	    {
		_callback = _previousCallback;
		_released = true;
		Monitor.Pulse(sync);
	    }
	    // Else nothing to release.
	    Monitor.Exit(sync);
	}

	public static bool interrupted()
	{
	    Monitor.Enter(sync);
	    bool rc = _interrupted;
	    Monitor.Exit(sync);
	    return rc;
	}

#if !__MonoCS__
	//
	// First-level handler
	//
	private static Boolean HandlerRoutine(int sig)
	{
	    Monitor.Enter(sync);
	    Callback callback = _callback;
	    Monitor.Exit(sync);
	    if(callback != null)
	    {
		try
		{
		    callback(sig);
		}
		catch(System.Exception)
		{
		    Debug.Assert(false);
		}
	    }
	    return true;
	}
#endif

	//
	// The callbacks to be invoked from the handler.
	//
	private static void holdInterruptCallback(int sig)
	{
	    bool destroyed;
	    Callback callback;
	    Monitor.Enter(sync);
	    while(!_released)
	    {
		Monitor.Wait(sync);
	    }

	    destroyed = _destroyed;
	    callback = _callback;
	    Monitor.Exit(sync);

	    //
	    // If not being destroyed by main thread call callback.
	    //
	    if(!destroyed && callback != null)
	    {
		callback(sig);
	    }
	}

	private static void destroyOnInterruptCallback(int sig)
	{
	    bool ignore = false;
	    Monitor.Enter(sync);
	    if(_destroyed)
	    {
	        //
		// Being destroyed by main thread
		//
	        ignore = true;
	    }
	    else if(_nohup && sig == SIGHUP)
	    {
	        ignore = true;
	    }
	    else
	    {
	    	_callbackInProgress = true;
	        _interrupted = true;
		_destroyed = true;
	    }
	    Monitor.Exit(sync);

	    if(ignore)
	    {
	        return;
	    }

	    try
	    {
		_communicator.destroy();
	    }
	    catch(System.Exception ex)
	    {
		Console.Error.WriteLine(_appName + ": while destroying in response to signal: " + ex);
	    }

	    Monitor.Enter(sync);
	    _callbackInProgress = false;
	    Monitor.Pulse(sync);
	    Monitor.Exit(sync);
	}

	private static void shutdownOnInterruptCallback(int sig)
	{
	    bool ignore = false;
	    Monitor.Enter(sync);
	    if(_destroyed)
	    {
	        //
		// Being destroyed by main thread
		//
	        ignore = true;
	    }
	    else if(_nohup && sig == SIGHUP)
	    {
	        ignore = true;
	    }
	    else
	    {
	    	_callbackInProgress = true;
	        _interrupted = true;
	    }
	    Monitor.Exit(sync);

	    if(ignore)
	    {
	        return;
	    }

	    try
	    {
		_communicator.shutdown();
	    }
	    catch(System.Exception ex)
	    {
		Console.Error.WriteLine(_appName + ": while shutting down in response to signal: " + ex);
	    }

	    Monitor.Enter(sync);
	    _callbackInProgress = false;
	    Monitor.Pulse(sync);
	    Monitor.Exit(sync);
	}

#if !__MonoCS__
	[DllImport("kernel32.dll")]
	private static extern Boolean SetConsoleCtrlHandler(Application.EventHandler eh, Boolean add);
#endif

	private static readonly object sync = typeof(Application);

	private const int SIGHUP = 5; // CTRL_LOGOFF_EVENT, from wincon.h

	private static bool _callbackInProgress;
	private static bool _destroyed;
	private static bool _interrupted;
	private static bool _released;
	private static bool _nohup;

	private delegate Boolean EventHandler(int sig);
#if !__MonoCS__
	private static readonly Application.EventHandler _handler
	    = new Application.EventHandler(HandlerRoutine); // First-level handler
#endif

	private delegate void Callback(int sig);
	private static readonly Callback _destroyCallback = new Callback(destroyOnInterruptCallback);
	private static readonly Callback _shutdownCallback = new Callback(shutdownOnInterruptCallback);
	private static readonly Callback _holdCallback = new Callback(holdInterruptCallback);

	private static Callback _callback; // Current callback
	private static Callback _previousCallback; // Remembers prev. callback when signals are held

	private static string _appName = AppDomain.CurrentDomain.FriendlyName;
	private static Communicator _communicator;
    }
}
