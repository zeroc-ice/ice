// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package Ice;

public abstract class Application
{
    public
    Application()
    {
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
            System.err.println(appName + ": only one instance of the " +
                               "Application class can be used");
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
            status = run(argHolder.value);
        }
        catch(LocalException ex)
        {
            System.err.println(_appName + ": " + ex);
            ex.printStackTrace();
            status = 1;
        }
        catch(Exception ex)
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
            catch(Exception ex)
            {
                System.err.println(_appName + ": unknown exception");
                ex.printStackTrace();
                status = 1;
            }
            _communicator = null;
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
    shutdownOnInterrupt()
    {
	if(_shutdownHook == null)
	{
	    //
	    // As soon as the shutdown hook ends all the threads are
	    // terminated. So the shutdown hook will join the current
	    // thread before to end.
	    //
	    _shutdownHook = new ShutdownHook(Thread.currentThread());
	    Runtime.getRuntime().addShutdownHook(_shutdownHook);
	}
    }
    
    synchronized public static void
    ignoreInterrupt()
    {
	//
	// We cannot ignore, only set back to default.
	//
	defaultInterrupt();
    }

    synchronized public static void
    defaultInterrupt()
    {
	if(_shutdownHook != null)
	{
	    try
	    {
		Runtime.getRuntime().removeShutdownHook(_shutdownHook);
	    }
	    catch(java.lang.IllegalStateException ex)
	    {
		//
		// Expected if we are in the process of shutting down.
		//
	    }

	    _shutdownHook = null;
	}
    }

    private static String _appName;
    private static Communicator _communicator;
    private static ShutdownHook _shutdownHook;
}
