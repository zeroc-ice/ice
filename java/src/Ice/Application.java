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

    //
    // TODO: These need to be implemented.
    //
    public static void
    shutdownOnInterrupt()
    {
    }

    public static void
    ignoreInterrupt()
    {
    }

    public static void
    defaultInterrupt()
    {
    }


    private static String _appName;
    private static Communicator _communicator;
}
