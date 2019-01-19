//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * Utility base class that makes it easy to correctly initialize and finalize
 * the Ice run time, as well as handle signals. Unless the application specifies
 * a logger, <code>Application</code> installs a per-process logger that logs to the standard
 * error output.
 * <p>
 * Applications must create a derived class that implements the {@link #run} method.
 * <p>
 * A program can contain only one instance of this class.
 *
 * @see #run
 * @see Communicator
 * @see Logger
 **/
public abstract class Application
{
    /**
     * Initializes an instance that calls {@link Communicator#destroy} if
     * a signal is received.
     **/
    public
    Application()
    {
    }

    /**
     * Initializes an instance that handles signals according to the signal
     * policy.
     *
     * @param signalPolicy Determines how to respond to signals.
     *
     * @see SignalPolicy
     **/
    public
    Application(SignalPolicy signalPolicy)
    {
        _signalPolicy = signalPolicy;
    }

    /**
     * The application must call <code>main</code> after it has
     * instantiated the derived class. <code>main</code> creates
     * a communicator, establishes the specified signal policy, and,
     * once <code>run</code> returns, destroys the communicator.
     * <p>
     * The method prints an error message for any exception that propagates
     * out of <code>run</code> and ensures that the communicator is
     * destroyed correctly even if <code>run</code> completes abnormally.
     *
     * @param appName The name of the application. This parameter is used to initialize
     * the value of the <code>Ice.ProgramName</code> property.
     * @param args The arguments for the application (as passed to <code>Main(String[])</code>
     * by the operating system.
     * @return The value returned by <code>run</code>. If <code>run</code> terminates with an exception,
     * the return value is non-zero.
     **/
    public final int
    main(String appName, String[] args)
    {
        return main(appName, args, new InitializationData());
    }

    /**
     * The application must call <code>main</code> after it has
     * instantiated the derived class. <code>main</code> creates
     * a communicator, establishes the specified signal policy, and,
     * once <code>run</code> returns, destroys the communicator.
     * <p>
     * The method prints an error message for any exception that propagates
     * out of <code>run</code> and ensures that the communicator is
     * destroyed correctly even if <code>run</code> completes abnormally.
     *
     * @param appName The name of the application. This parameter is used to initialize
     * the value of the <code>Ice.ProgramName</code> property.
     * @param args Command-line arguments
     * @param configFile The configuration file with which to initialize
     * Ice properties.
     * @return The value returned by <code>run</code>. If <code>run</code> terminates with an exception,
     * the return value is non-zero.
     **/
    public final int
    main(String appName, String[] args, String configFile)
    {
        if(Util.getProcessLogger() instanceof LoggerI)
        {
            Util.setProcessLogger(new LoggerI(appName, ""));
        }

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
                Util.getProcessLogger().error(com.zeroc.IceInternal.Ex.toString(ex));
                return 1;
            }
            catch(java.lang.Exception ex)
            {
                Util.getProcessLogger().error("unknown exception: " + com.zeroc.IceInternal.Ex.toString(ex));
                return 1;
            }
        }
        return main(appName, args, initData);
    }

    /**
     * The application must call <code>main</code> after it has
     * instantiated the derived class. <code>main</code> creates
     * a communicator, establishes the specified signal policy, and,
     * once <code>run</code> returns, destroys the communicator.
     * <p>
     * The method prints an error message for any exception that propagates
     * out of <code>run</code> and ensures that the communicator is
     * destroyed correctly even if <code>run</code> completes abnormally.
     *
     * @param appName The name of the application. This parameter is used to initialize
     * the value of the <code>Ice.ProgramName</code> property.
     * @param args The arguments for the application (as passed to <code>Main(String[])</code>.
     * @param initializationData Additional data used to initialize the communicator.
     * @return The value returned by <code>run</code>. If <code>run</code> terminates with an exception,
     * the return value is non-zero.
     *
     * @see InitializationData
     **/
    public final int
    main(String appName, String[] args, InitializationData initializationData)
    {
        if(Util.getProcessLogger() instanceof LoggerI)
        {
            Util.setProcessLogger(new LoggerI(appName, ""));
        }

        if(_communicator != null)
        {
            Util.getProcessLogger().error("only one instance of the Application class can be used");
            return 1;
        }

        _appName = appName;

        //
        // We parse the properties here to extract Ice.ProgramName.
        //
        InitializationData initData;
        if(initializationData != null)
        {
            initData = initializationData.clone();
        }
        else
        {
            initData = new InitializationData();
        }

        java.util.List<String> remainingArgs = new java.util.ArrayList<>();
        try
        {
            initData.properties = Util.createProperties(args, initData.properties, remainingArgs);
        }
        catch(LocalException ex)
        {
            Util.getProcessLogger().error(com.zeroc.IceInternal.Ex.toString(ex));
            return 1;
        }
        catch(java.lang.Exception ex)
        {
            Util.getProcessLogger().error("unknown exception: " + com.zeroc.IceInternal.Ex.toString(ex));
            return 1;
        }
        _appName = initData.properties.getPropertyWithDefault("Ice.ProgramName", _appName);

        //
        // If the process logger is the default logger, we replace it with a
        // a logger which is using the program name for the prefix.
        //
        if(!initData.properties.getProperty("Ice.ProgramName").equals("") && Util.getProcessLogger() instanceof LoggerI)
        {
            Util.setProcessLogger(new LoggerI(initData.properties.getProperty("Ice.ProgramName"), ""));
        }

        return doMain(remainingArgs.toArray(new String[remainingArgs.size()]), initData);
    }

    protected int doMain(String[] args, InitializationData initData)
    {
        int status = 0;

        try
        {
            java.util.List<String> remainingArgs = new java.util.ArrayList<>();
            _communicator = Util.initialize(args, initData, remainingArgs);

            //
            // The default is to destroy when a signal is received.
            //
            if(_signalPolicy == SignalPolicy.HandleSignals)
            {
                destroyOnInterrupt();
            }

            status = run(remainingArgs.toArray(new String[remainingArgs.size()]));
        }
        catch(LocalException ex)
        {
            Util.getProcessLogger().error(com.zeroc.IceInternal.Ex.toString(ex));
            status = 1;
        }
        catch(java.lang.Exception ex)
        {
            Util.getProcessLogger().error("unknown exception: " + com.zeroc.IceInternal.Ex.toString(ex));
            status = 1;
        }
        catch(java.lang.Error err)
        {
            //
            // We catch Error to avoid hangs in some non-fatal situations
            //
            Util.getProcessLogger().error("Java error: " + com.zeroc.IceInternal.Ex.toString(err));
            status = 1;
        }

        // This clears any set interrupt.
        if(_signalPolicy == SignalPolicy.HandleSignals)
        {
            defaultInterrupt();
        }

        synchronized(_mutex)
        {
            boolean interrupted = false;
            while(_callbackInProgress)
            {
                try
                {
                    _mutex.wait();
                }
                catch(InterruptedException ex)
                {
                    interrupted = true;
                }
            }
            if(interrupted)
            {
                Thread.currentThread().interrupt();
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
                try
                {
                    _communicator.destroy();
                }
                catch(OperationInterruptedException ex)
                {
                    Util.getProcessLogger().error(com.zeroc.IceInternal.Ex.toString(ex));
                    // Retry communicator destroy in case of an operation
                    // interrupt exception, but don't do so in a loop
                    // otherwise it could go on forever.
                    _communicator.destroy();
                }
            }
            catch(LocalException ex)
            {
                Util.getProcessLogger().error(com.zeroc.IceInternal.Ex.toString(ex));
                status = 1;
            }
            catch(java.lang.Exception ex)
            {
                Util.getProcessLogger().error("unknown exception: " + com.zeroc.IceInternal.Ex.toString(ex));
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

    /**
     * Called once the communicator has been initialized. The derived class must
     * implement <code>run</code>, which is the application's starting method.
     *
     * @param args The argument vector for the application. <code>Application</code>
     * scans the argument vector passed to <code>main</code> for options that are
     * specific to the Ice run time and removes them; therefore, the vector passed
     * to <code>run</code> is free from Ice-related options and contains only options
     * and arguments that are application-specific.
     *
     * @return The <code>run</code> method should return zero for successful termination, and
     * non-zero otherwise. <code>Application.main</code> returns the value returned by <code>run</code>.
     **/
    public abstract int
    run(String[] args);

    /**
     * Returns the value of <code>appName</code> that is passed to <code>main</code> (which is also the
     * the value of <code>Ice.ProgramName</code>). This method is useful mainly for error messages that
     * include the application name. Because <code>appName</code> is a static method, it is available from anywhere
     * in the program.
     *
     * @return The name of the application.
     **/
    public static String
    appName()
    {
        return _appName;
    }

    /**
     * Returns the communicator for the application. Because <code>communicator</code> is a static method,
     * it permits access to the communicator from anywhere in the program. Note that, as a consequence,
     * you cannot have more than one instance of <code>Application</code> in a program.
     *
     * @return The communicator for the application.
     **/
    public static Communicator
    communicator()
    {
        return _communicator;
    }

    /**
     * Instructs <code>Application</code> to call {@link Communicator#destroy} on receipt of a signal.
     * This is default signal handling policy established by the default constructor.
     *
     * @see Communicator#destroy
     **/
    public static void
    destroyOnInterrupt()
    {
        if(_signalPolicy == SignalPolicy.HandleSignals)
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
        else
        {
            Util.getProcessLogger().warning(
                "interrupt method called on Application configured to not handle interrupts.");
        }
    }

    /**
     * Instructs <code>Application</code> to call {@link Communicator#shutdown} on receipt of a signal.
     *
     * @see Communicator#shutdown
     **/
    public static void
    shutdownOnInterrupt()
    {
        if(_signalPolicy == SignalPolicy.HandleSignals)
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
        else
        {
            Util.getProcessLogger().warning(
                "interrupt method called on Application configured to not handle interrupts.");
        }
    }

    /**
     * Installs a custom shutdown hook. The implementation of the shutdown
     * hook can do whatever cleanup is necessary to shut down the application.
     * The hook is unregistered once <code>run</code> returns.
     * Note that the hook must obey the rules for shutdown hooks; specifically,
     * it must not call <code>exit</code>.
     *
     * @param newHook The Runnable to run on shutdown.
     *
     * @see java.lang.Runtime#addShutdownHook
     **/
    public static void
    setInterruptHook(Runnable newHook)
    {
        if(_signalPolicy == SignalPolicy.HandleSignals)
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
        else
        {
            Util.getProcessLogger().warning(
                "interrupt method called on Application configured to not handle interrupts.");
        }
    }

    /**
     * Clears any shutdown hooks, including any hook established with {@link #destroyOnInterrupt} or
     * {@link #shutdownOnInterrupt}.
     **/
    public static void
    defaultInterrupt()
    {
        if(_signalPolicy == SignalPolicy.HandleSignals)
        {
            changeHook(null);
        }
        else
        {
            Util.getProcessLogger().warning(
                "interrupt method called on Application configured to not handle interrupts.");
        }
    }

    /**
     * Determines whether the application shut down intentionally or was forced to shut down by the JVM. This
     * is useful for logging purposes.
     *
     * @return <code>true</code> if a shutdown hook caused the communicator to shut down; false otherwise.
     **/
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
            // Note that we let the IllegalStateException propagate
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

    // For use by Glacier2.Application
    /** @hidden */
    static public class AppHook extends Thread
    {
        public void
        done()
        {
            synchronized(_doneMutex)
            {
                _done = true;
                _doneMutex.notify();
            }
        }

        protected boolean _done = false;
        protected final java.lang.Object _doneMutex = new java.lang.Object();
    }

    static class DestroyHook extends AppHook
    {
        @Override
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
                        break;
                    }
                }
            }
        }
    }

    static class ShutdownHook extends AppHook
    {
        @Override
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
                        break;
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
        CustomHook(Runnable hook)
        {
            _hook = hook;
        }

        @Override
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

        private Runnable _hook;
    }

    /** @hidden */
    protected static String _appName;

    /** @hidden */
    protected static Communicator _communicator;

    /** @hidden */
    protected static AppHook _appHook;

    /** @hidden */
    protected final static java.lang.Object _mutex = new java.lang.Object();

    /** @hidden */
    protected static boolean _callbackInProgress = false;

    /** @hidden */
    protected static boolean _destroyed = false;

    /** @hidden */
    protected static boolean _interrupted = false;

    /** @hidden */
    protected static SignalPolicy _signalPolicy = SignalPolicy.HandleSignals;
}
