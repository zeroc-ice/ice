// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public abstract class Application
{
    public
    Application()
    {
    	_interrupted = false;
    }

    //
    // This main() must be called by the global main(). main()
    // initializes the Communicator, calls run(), and destroys
    // the Communicator upon return from run(). It thereby handles
    // all exceptions properly, i.e., error messages are printed
    // if exceptions propagate to main(), and the Communicator is
    // always destroyed, regardless of exceptions.
    //
    public final int
    main(String appName, String[] args)
    {
        return main(appName, args, null);
    }

    public final int
    main(String appName, String[] args, String configFile)
    {
        if(_communicator != null)
        {
            System.err.println(appName + ": only one instance of the Application class can be used");
            return 1;
        }

        _appName = appName;

        int status = 0;

        try
        {
	    StringSeqHolder argHolder = new StringSeqHolder(args);
            if(configFile != null)
            {
                Properties properties = Util.createProperties(argHolder);
                properties.load(configFile);
                _communicator = Util.initializeWithProperties(argHolder, properties);
            }
            else
            {
                _communicator = Util.initialize(argHolder);
            }

            //
            // The default is to destroy when a signal is received.
            //
            destroyOnInterrupt();

            status = run(argHolder.value);
        }
        catch(LocalException ex)
        {
            System.err.println(_appName + ": " + ex);
            ex.printStackTrace();
            status = 1;
        }
        catch(java.lang.Exception ex)
        {
            System.err.println(_appName + ": unknown exception");
            ex.printStackTrace();
            status = 1;
        }

        if(_communicator != null)
        {
            try
            {
                _communicator.destroy();
            }
            catch(LocalException ex)
            {
                System.err.println(_appName + ": " + ex);
                ex.printStackTrace();
                status = 1;
            }
            catch(java.lang.Exception ex)
            {
                System.err.println(_appName + ": unknown exception");
                ex.printStackTrace();
                status = 1;
            }
            _communicator = null;
        }

        synchronized(this)
        {
            if(_destroyHook != null)
            {
                _destroyHook.done();
            }

            if(_shutdownHook != null)
            {
                _shutdownHook.done();
            }
        }

        return status;
    }

    public abstract int
    run(String[] args);

    //
    // Return the application name, i.e., argv[0].
    //
    public static String
    appName()
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
    public static Communicator
    communicator()
    {
        return _communicator;
    }

    synchronized public static void
    destroyOnInterrupt()
    {
	//
	// Remove the shutdown hook it's set.
	//
	if(_shutdownHook != null)
	{
	    try
	    {
		Runtime.getRuntime().removeShutdownHook(_shutdownHook);
                _shutdownHook = null;
	    }
	    catch(java.lang.IllegalStateException ex)
	    {
		//
		// Expected if we are in the process of shutting down.
		//
	    }
	}

	if(_destroyHook == null)
	{
	    //
	    // As soon as the destroy hook ends all the threads are
	    // terminated. So the destroy hook will join the current
	    // thread before to end.
	    //
	    _destroyHook = new DestroyHook();
	    try
	    {
		Runtime.getRuntime().addShutdownHook(_destroyHook);
	    }
	    catch(java.lang.IllegalStateException ex)
	    {
		if(_communicator != null)
		{
		    _communicator.destroy();
		}
	    }
	}
    }
    
    synchronized public static void
    shutdownOnInterrupt()
    {
	//
	// Remove the destroy hook if it's set.
	//
	if(_destroyHook != null)
	{
	    try
	    {
		Runtime.getRuntime().removeShutdownHook(_destroyHook);
                _destroyHook = null;
	    }
	    catch(java.lang.IllegalStateException ex)
	    {
		//
		// Expected if we are in the process of shutting down.
		//
	    }
	}

	if(_shutdownHook == null)
	{
	    //
	    // As soon as the shutdown hook ends all the threads are
	    // terminated. So the shutdown hook will join the current
	    // thread before to end.
	    //
	    _shutdownHook = new ShutdownHook();
	    try
	    {
		Runtime.getRuntime().addShutdownHook(_shutdownHook);
	    }
	    catch(java.lang.IllegalStateException ex)
	    {
		if(_communicator != null)
		{
		    _communicator.shutdown();
		}
	    }
	}
    }
    
    synchronized public static void
    defaultInterrupt()
    {
	if(_shutdownHook != null)
	{
	    try
	    {
		Runtime.getRuntime().removeShutdownHook(_shutdownHook);
                _shutdownHook = null;
	    }
	    catch(java.lang.IllegalStateException ex)
	    {
		//
		// Expected if we are in the process of shutting down.
		//
	    }
	}

	if(_destroyHook != null)
	{
	    try
	    {
		Runtime.getRuntime().removeShutdownHook(_destroyHook);
                _destroyHook = null;
	    }
	    catch(java.lang.IllegalStateException ex)
	    {
		//
		// Expected if we are in the process of shutting down.
		//
	    }
	}
    }

    synchronized public static boolean
    interrupted()
    {
    	return _interrupted;
    }

    synchronized private static void
    setInterrupt()
    {
    	_interrupted = true;
    }

    static class DestroyHook extends Thread
    {
        DestroyHook()
        {
            _done = false;
        }

        public void
        run()
        {
            synchronized(_doneMutex)
            {
		setInterrupt();

                Communicator communicator = communicator();
                if(communicator != null)
                {
                    communicator.destroy();
                }

                while(!_done)
                {
                    try
                    {
                        _doneMutex.wait();
                    }
                    catch(InterruptedException ex)
                    {
                    }
                }
            }
        }

        void
        done()
        {
            synchronized(_doneMutex)
            {
                _done = true;
                _doneMutex.notify();
            }
        }

        private boolean _done;
        private java.lang.Object _doneMutex = new java.lang.Object();
    }

    static class ShutdownHook extends Thread
    {
        ShutdownHook()
        {
            _done = false;
        }

        public void
        run()
        {
            synchronized(_doneMutex)
            {
		setInterrupt();

                Communicator communicator = communicator();
                if(communicator != null)
                {
                    communicator.shutdown();
                }

                while(!_done)
                {
                    try
                    {
                        _doneMutex.wait();
                    }
                    catch(InterruptedException ex)
                    {
                    }
                }
            }
        }

        void
        done()
        {
            synchronized(_doneMutex)
            {
                _done = true;
                _doneMutex.notify();
            }
        }

        private boolean _done;
        private java.lang.Object _doneMutex = new java.lang.Object();
    }

    private static String _appName;
    private static Communicator _communicator;
    private static DestroyHook _destroyHook;
    private static ShutdownHook _shutdownHook;
    private static boolean _interrupted;
}
