// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
        return main(appName, args, new InitializationData());
    }

    public final int
    main(String appName, String[] args, String configFile)
    {
        InitializationData initData = new InitializationData();
        if(configFile != null)
        {
            try
            {
                initData.properties = Util.createProperties();
                initData.properties.load(configFile);
            }
            catch(LocalException ex)
            {
                System.err.println(appName + ": " + ex);
                ex.printStackTrace();
                return 1;
            }
            catch(java.lang.Exception ex)
            {
                System.err.println(appName + ": unknown exception");
                ex.printStackTrace();
                return 1;
            }
        }
        return main(appName, args, initData);
    }

    /**
     * @deprecated This method has been deprecated.
     **/
    public final int
    main(String appName, String[] args, String configFile, Logger logger)
    {
        InitializationData initData = new InitializationData();
        if(configFile != null)
        {
            try
            {
                initData.properties = Util.createProperties();
                initData.properties.load(configFile);
            }
            catch(LocalException ex)
            {
                System.err.println(appName + ": " + ex);
                ex.printStackTrace();
                return 1;
            }
            catch(java.lang.Exception ex)
            {
                System.err.println(appName + ": unknown exception");
                ex.printStackTrace();
                return 1;
            }
        }
        initData.logger = logger;
        return main(appName, args, initData);
    }

    public final int
    main(String appName, String[] args, InitializationData initData)
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
            _communicator = Util.initialize(argHolder, initData);

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
        catch(java.lang.Error err)
        {
            //
            // We catch Error to avoid hangs in some non-fatal situations
            //
            System.err.println(_appName + ": Java error");
            err.printStackTrace();
            status = 1;
        }

        // This clears any set interrupt.
        defaultInterrupt();

        synchronized(_mutex)
        {
            while(_callbackInProgress)
            {
                try
                {
                    _mutex.wait();
                }
                catch(java.lang.InterruptedException ex)
                {
                }
            }

            if(_destroyed)
            {
                _communicator = null;
            }
            else
            {
                _destroyed = true;
                //
                // And _communicator != null, meaning will be
                // destroyed next, _destroyed = true also ensures that
                // any remaining callback won't do anything
                //
            }
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

        synchronized(_mutex)
        {
            if(_appHook != null)
            {
                _appHook.done();
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

    public static void
    destroyOnInterrupt()
    {
        synchronized(_mutex)
        {
            //
            // As soon as the destroy hook ends all the threads are
            // terminated. So the destroy hook will join the current
            // thread before to end.
            //
            try
            {
                changeHook(new DestroyHook());
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
    
    public static void
    shutdownOnInterrupt()
    {
        synchronized(_mutex)
        {
            //
            // As soon as the shutdown hook ends all the threads are
            // terminated. So the shutdown hook will join the current
            // thread before to end.
            //
            try
            {
                changeHook(new ShutdownHook());
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

    //
    // Install a custom shutdown hook. This hook is registered as a
    // shutdown hook and should do whatever is necessary to terminate
    // the application. Note that this runs as a shutdown hook so the
    // code must obey the same rules as a shutdown hook (specifically
    // don't call exit!). The shutdown and destroy shutdown interrupts
    // are cleared. This hook is automatically unregistered after
    // Application.run() returns.
    //
    public static void
    setInterruptHook(java.lang.Thread newHook) // Pun intended.
    {
        try
        {
            changeHook(new CustomHook(newHook));
        }
        catch(java.lang.IllegalStateException ex)
        {
            // Ignore.
        }
    }
    
    //
    // This clears any shutdown hooks including any custom hook.
    //
    public static void
    defaultInterrupt()
    {
        changeHook(null);
    }

    public static boolean
    interrupted()
    {
        synchronized(_mutex)
        {
            return _interrupted;
        }
    }

    private static void
    changeHook(AppHook newHook)
    {
        synchronized(_mutex)
        {
            //
            // Remove any existing shutdown hooks.
            //
            try
            {
                if(_appHook != null)
                {
                    Runtime.getRuntime().removeShutdownHook(_appHook);
                    _appHook.done();
                    _appHook = null;
                }
            }
            catch(java.lang.IllegalStateException ex)
            {
                //
                // Expected if we are in the process of shutting down.
                //
            }

            //
            // Note that we let the IllegalStateException propogate
            // out if necessary.
            //
            if(newHook != null)
            {
                Runtime.getRuntime().addShutdownHook(newHook);
                _appHook = newHook;
            }
        }
    }

    private static boolean
    setCallbackInProgress(boolean destroy)
    {
        synchronized(_mutex)
        {
            if(_destroyed)
            {
                //
                // Being destroyed by main thread
                //
                return false;
            }
            _callbackInProgress = true;
            _destroyed = destroy;
            _interrupted = true;
            return true;
        }
    }

    private static void
    clearCallbackInProgress()
    {
        synchronized(_mutex)
        {
            _callbackInProgress = false;
            _mutex.notify();
        }
    }

    static class AppHook extends Thread
    {
        void
        done()
        {
            synchronized(_doneMutex)
            {
                _done = true;
                _doneMutex.notify();
            }
        }

        protected boolean _done = false;
        protected java.lang.Object _doneMutex = new java.lang.Object();
    }

    static class DestroyHook extends AppHook
    {
        public void
        run()
        {
            synchronized(_doneMutex)
            {
                if(!setCallbackInProgress(true))
                {
                    return;
                }

                Communicator communicator = communicator();
                if(communicator != null)
                {
                    communicator.destroy();
                }

                clearCallbackInProgress();

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
    }

    static class ShutdownHook extends AppHook
    {
        public void
        run()
        {
            synchronized(_doneMutex)
            {
                if(!setCallbackInProgress(false))
                {
                    return;
                }

                Communicator communicator = communicator();
                if(communicator != null)
                {
                    communicator.shutdown();
                }

                clearCallbackInProgress();

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
    }

    // Although this class doesn't actually use any of the AppHook
    // done stuff, its more trouble than its worth to add all of the
    // code necessary to support another hook member variable and
    // support code.
    static class CustomHook extends AppHook
    {
        CustomHook(Thread hook)
        {
            _hook = hook;
        }

        public void
        run()
        {
            synchronized(_doneMutex)
            {
                if(!setCallbackInProgress(false))
                {
                    return;
                }
                
                _hook.run();

                clearCallbackInProgress();

                //
                // Don't bother to join with main, we're done.
                //
            }
        }

        private Thread _hook;
    }

    private static String _appName;
    private static Communicator _communicator;
    private static AppHook _appHook;
    private static java.lang.Object _mutex = new java.lang.Object();
    private static boolean _callbackInProgress = false;
    private static boolean _destroyed = false;
    private static boolean _interrupted = false;
}
