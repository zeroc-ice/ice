// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Ice
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Reflection;
    using System.Runtime.InteropServices;
    using System.Threading;

    /// <summary>
    /// The signal policy for Ice.Application signal handling.
    /// </summary>
    public enum SignalPolicy
    {
        /// <summary>
        /// If a signal is received, Ice.Application reacts to the signal
        /// by calling Communicator.destroy or Communicator.shutdown,
        /// or by calling a custom shutdown hook installed by the application.
        /// </summary>
        HandleSignals,

        /// <summary>
        /// Any signal that is received is not intercepted and takes the default action.
        /// </summary>
        NoSignalHandling
    }

    /// <summary>
    /// Utility base class that makes it easy to to correctly initialize and finalize
    /// the Ice run time, as well as handle signals. Unless the application specifies
    /// a logger, Application installs a per-process logger that logs to the standard
    /// error output.
    /// Applications must create a derived class that implements the run method.
    /// A program can contain only one instance of this class.
    /// </summary>
    public abstract class Application
    {
        /// <summary>
        /// Called once the communicator has been initialized. The derived class must
        /// implement run, which is the application's starting method.
        /// </summary>
        /// <param name="args">The argument vector for the application. Application
        /// scans the argument vector passed to main for options that are
        /// specific to the Ice run time and removes them; therefore, the vector passed
        /// to run is free from Ice-related options and contains only options
        /// and arguments that are application-specific.</param>
        /// <returns>The run method should return zero for successful termination, and
        /// non-zero otherwise. Application.main returns the value returned by run.</returns>
        public abstract int run(string[] args);

        /// <summary>
        /// Override this method to provide a custom application interrupt
        /// hook. You must call callbackOnInterrupt for this method
        /// to be called. Note that the interruptCallback can be called
        /// concurrently with any other thread (including main) in your
        /// application--take appropriate concurrency precautions.
        /// </summary>
        /// <param name="sig">The cause of the interrupt.</param>
        public virtual void interruptCallback(int sig)
        {
        }

        /// <summary>
        /// Initializes an instance that calls Communicator.shutdown if a signal is received.
        /// </summary>
        public Application()
        {
        }

        /// <summary>
        /// Initializes an instance that handles signals according to the signal policy.
        /// </summary>
        /// <param name="signalPolicy">Determines how to respond to signals.</param>
        public Application(SignalPolicy signalPolicy)
        {
            _signalPolicy = signalPolicy;
        }

        /// <summary>
        /// The application must call main after it has
        /// instantiated the derived class. main creates
        /// a communicator, establishes the specified signal policy, and,
        /// once run returns, destroys the communicator.
        /// The method prints an error message for any exception that propagates
        /// out of run and ensures that the communicator is
        /// destroyed correctly even if run completes abnormally.
        /// </summary>
        /// <param name="args">The arguments for the application (as passed to Main(string[])
        /// by the operating system.</param>
        /// <returns>The value returned by run. If run terminates with an exception,
        /// the return value is non-zero.</returns>
        public int main(string[] args)
        {
            return mainInternal(args, new InitializationData(), null);
        }

        /// <summary>
        /// The application must call main after it has
        /// instantiated the derived class. main creates
        /// a communicator, establishes the specified signal policy, and,
        /// once run returns, destroys the communicator.
        /// The method prints an error message for any exception that propagates
        /// out of run and ensures that the communicator is
        /// destroyed correctly even if run completes abnormally.
        /// </summary>
        /// <param name="args">The arguments for the application (as passed to Main(string[])
        /// by the operating system.</param>
        /// <param name="configFile">The configuration file with which to initialize
        /// Ice properties.</param>
        /// <returns>The value returned by run. If run terminates with an exception,
        /// the return value is non-zero.</returns>
        public int main(string[] args, string configFile)
        {
            return main(args, configFile, null);
        }

        /// <summary>
        /// The application must call main after it has
        /// instantiated the derived class. main creates
        /// a communicator, establishes the specified signal policy, and,
        /// once run returns, destroys the communicator.
        /// The method prints an error message for any exception that propagates
        /// out of run and ensures that the communicator is
        /// destroyed correctly even if run completes abnormally.
        /// </summary>
        /// <param name="args">The arguments for the application (as passed to Main(string[])
        /// by the operating system.</param>
        /// <param name="configFile">The configuration file with which to initialize
        /// Ice properties.</param>
        /// <param name="overrideProps">Property values that override any settings in configFile.</param>
        /// <returns>The value returned by run. If run terminates with an exception,
        /// the return value is non-zero.</returns>
        public int main(string[] args, string configFile, Properties overrideProps)
        {
            InitializationData initData = new InitializationData();
            if(configFile != null)
            {
                try
                {
                    initData.properties = Util.createProperties();
                    initData.properties.load(configFile);
                }
                catch(Ice.Exception ex)
                {
                    Util.getProcessLogger().error(_appName + ":\n" + ex);
                    return 1;
                }
                catch(System.Exception ex)
                {
                    Util.getProcessLogger().error(_appName + ": unknown exception:\n" + ex);
                    return 1;
                }
            }
            return mainInternal(args, initData, overrideProps);
        }

        /// <summary>
        /// The application must call main after it has
        /// instantiated the derived class. main creates
        /// a communicator, establishes the specified signal policy, and,
        /// once run returns, destroys the communicator.
        /// The method prints an error message for any exception that propagates
        /// out of run and ensures that the communicator is
        /// destroyed correctly even if run completes abnormally.
        /// </summary>
        /// <param name="args">The arguments for the application (as passed to Main(string[])
        /// by the operating system.</param>
        /// <param name="initData">Additional data used to initialize the communicator.</param>
        /// <returns>The value returned by run. If run terminates with an exception,
        /// the return value is non-zero.</returns>
        public int main(string[] args, InitializationData initData)
        {
            return mainInternal(args, initData, null);
        }

        /// <summary>
        /// Returns the application name (which is also the value of Ice.ProgramName.
        /// This method is useful mainly for error messages that
        /// include the application name. Because appName is a static method, it is available from anywhere
        /// in the program.
        /// </summary>
        /// <returns>The name of the application.</returns>
        public static string appName()
        {
            return _appName;
        }

        /// <summary>
        /// Returns the communicator for the application. Because communicator is a static method,
        /// it permits access to the communicator from anywhere in the program. Note that, as a consequence,
        /// you cannot have more than one instance of Application in a program.
        /// </summary>
        /// <returns>The communicator for the application.</returns>
        public static Communicator communicator()
        {
            return _communicator;
        }

        /// <summary>
        /// Instructs Application to call Communicator.destroy on receipt of a signal.
        /// This is default signal handling policy established by the default constructor.
        /// </summary>
        public static void destroyOnInterrupt()
        {
            if(_signalPolicy == SignalPolicy.HandleSignals)
            {
                lock(_mutex)
                {
                    if(_callback == _holdCallback)
                    {
                        _released = true;
                        Monitor.Pulse(_mutex);
                    }
                    _callback = _destroyCallback;
                }
            }
            else
            {
                Util.getProcessLogger().error(_appName +
                            ": warning: interrupt method called on Application configured to not handle interrupts.");
            }
        }

        /// <summary>
        /// Instructs Application to call Communicator.shutdown on receipt of a signal.
        /// </summary>
        public static void shutdownOnInterrupt()
        {
            if(_signalPolicy == SignalPolicy.HandleSignals)
            {
                lock(_mutex)
                {
                    if(_callback == _holdCallback)
                    {
                        _released = true;
                        Monitor.Pulse(_mutex);
                    }
                    _callback = _shutdownCallback;
                }
            }
            else
            {
                Util.getProcessLogger().error(_appName +
                            ": warning: interrupt method called on Application configured to not handle interrupts.");
            }
        }

        /// <summary>
        /// Instructs Application to ignore signals.
        /// </summary>
        public static void ignoreInterrupt()
        {
            if(_signalPolicy == SignalPolicy.HandleSignals)
            {
                lock(_mutex)
                {
                    if(_callback == _holdCallback)
                    {
                        _released = true;
                        Monitor.Pulse(_mutex);
                    }
                    _callback = null;
                }
            }
            else
            {
                Util.getProcessLogger().error(_appName +
                            ": warning: interrupt method called on Application configured to not handle interrupts.");
            }
        }

        /// <summary>
        /// Instructs Application to call interruptCallback on receipt of a signal.
        /// The derived class can intercept signals by overriding interruptCallback.
        /// </summary>
        public static void callbackOnInterrupt()
        {
            if(_signalPolicy == SignalPolicy.HandleSignals)
            {
                lock(_mutex)
                {
                    if(_callback == _holdCallback)
                    {
                        _released = true;
                        Monitor.Pulse(_mutex);
                    }
                    _callback = _userCallback;
                }
            }
            else
            {
                Util.getProcessLogger().error(_appName +
                            ": warning: interrupt method called on Application configured to not handle interrupts.");
            }
        }

        /// <summary>
        /// Instructs Application to call to hold signals.
        /// </summary>
        public static void holdInterrupt()
        {
            if(_signalPolicy == SignalPolicy.HandleSignals)
            {
                lock(_mutex)
                {
                    if(_callback != _holdCallback)
                    {
                        _previousCallback = _callback;
                        _released = false;
                        _callback = _holdCallback;
                    }
                    // else, we were already holding signals
                }
            }
            else
            {
                Util.getProcessLogger().error(_appName +
                            ": warning: interrupt method called on Application configured to not handle interrupts.");
            }
        }

        /// <summary>
        /// Instructs Application respond to signals. If a signal arrived since the last call
        /// to holdInterrupt, it is delivered once you call releaseInterrupt.
        /// </summary>
        public static void releaseInterrupt()
        {
            if(_signalPolicy == SignalPolicy.HandleSignals)
            {
                lock(_mutex)
                {
                    if(_callback == _holdCallback)
                    {
                        //
                        // Note that it's very possible no signal is held;
                        // in this case the callback is just replaced and
                        // setting _released to true and signalling _mutex
                        // do no harm.
                        //

                        _released = true;
                        _callback = _previousCallback;
                        Monitor.Pulse(_mutex);
                    }
                    // Else nothing to release.
                }
            }
            else
            {
                Util.getProcessLogger().error(_appName +
                            ": warning: interrupt method called on Application configured to not handle interrupts.");
            }
        }

        /// <summary>
        /// Determines whether the application shut down intentionally or was forced to shut down due to a signal.
        /// This is useful for logging purposes.
        /// </summary>
        /// <returns>True if a signal caused the communicator to shut down; false otherwise.</returns>
        public static bool interrupted()
        {
            lock(_mutex)
            {
                return _interrupted;
            }
        }

        private int mainInternal(string[] args, InitializationData initData, Properties overrideProps)
        {
            if(_communicator != null)
            {
                Util.getProcessLogger().error(_appName + ": only one instance of the Application class can be used");
                return 1;
            }

            int status;

            if(_signalPolicy == SignalPolicy.HandleSignals)
            {
                if(IceInternal.AssemblyUtil.platform_ == IceInternal.AssemblyUtil.Platform.Windows)
                {
                    _signals = new WindowsSignals();
                }
                else
                {
                    _signals = new MonoSignals();
                }
                _signals.register(_handler);

                status = executeRun(args, initData, overrideProps);

                _signals = null;
            }
            else
            {
                status = executeRun(args, initData, overrideProps);
            }

            return status;
        }

        private int executeRun(string[] args, InitializationData initializationData, Properties overrideProps)
        {
            int status = 0;

            try
            {
                //
                // We parse the properties here to extract Ice.ProgramName.
                //
                InitializationData initData;
                if(initializationData != null)
                {
                    initData = (InitializationData)initializationData.Clone();
                }
                else
                {
                    initData = new InitializationData();
                }
                initData.properties = Util.createProperties(ref args, initData.properties);
                if(overrideProps != null)
                {
                    Dictionary<string, string> oprops = overrideProps.getPropertiesForPrefix("");
                    foreach(KeyValuePair<string,string> entry in oprops)
                    {
                        initData.properties.setProperty(entry.Key, entry.Value);
                    }
                }

                //
                // If the process logger is the default logger, we replace it with a
                // a logger which is using the program name for the prefix.
                //
                if(Util.getProcessLogger() is LoggerI)
                {
                    Util.setProcessLogger(new LoggerI(initData.properties.getProperty("Ice.ProgramName"), ""));
                }

                _application = this;
                _communicator = Util.initialize(ref args, initData);
                _destroyed = false;

                Properties props = _communicator.getProperties();
                _nohup = props.getPropertyAsInt("Ice.Nohup") > 0;
                _appName = props.getPropertyWithDefault("Ice.ProgramName", _appName);

                //
                // The default is to destroy when a signal is received.
                //
                if(_signalPolicy == SignalPolicy.HandleSignals)
                {
                    destroyOnInterrupt();
                }

                status = run(args);
            }
            catch(Ice.Exception ex)
            {
                Util.getProcessLogger().error(_appName + ":\n" + ex);
                status = 1;
            }
            catch(System.Exception ex)
            {
                Util.getProcessLogger().error(_appName + ": unknown exception:\n" + ex);
                status = 1;
            }

            //
            // Don't want any new interrupt. And at this point
            // (post-run), it would not make sense to release a held
            // signal to run shutdown or destroy.
            //
            if(_signalPolicy == SignalPolicy.HandleSignals)
            {
                ignoreInterrupt();
            }

            lock(_mutex)
            {
                while(_callbackInProgress)
                {
                    Monitor.Wait(_mutex);
                }
                if(_destroyed)
                {
                    _communicator = null;
                }
                else
                {
                    _destroyed = true;
                    //
                    // _communicator != null means that it will be destroyed
                    // next; _destroyed == true ensures that any
                    // remaining callback won't do anything
                    //
                }
                _application = null;
            }

            if(_communicator != null)
            {
                try
                {
                    _communicator.destroy();
                }
                catch(Ice.Exception ex)
                {
                    Util.getProcessLogger().error(_appName + ":\n" + ex);
                    status = 1;
                }
                catch(System.Exception ex)
                {
                    Util.getProcessLogger().error(_appName + ": unknown exception:\n" + ex);
                    status = 1;
                }
                _communicator = null;
            }

            return status;
        }

        //
        // First-level handler.
        //
        private static void signalHandler(int sig)
        {
            Callback callback;
            lock(_mutex)
            {
                callback = _callback;
            }
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
        }

        //
        // The callbacks to be invoked from the handler.
        //
        private static void holdInterruptCallback(int sig)
        {
            Callback callback = null;
            lock(_mutex)
            {
                while(!_released)
                {
                    Monitor.Wait(_mutex);
                }

                if(_destroyed)
                {
                    //
                    // Being destroyed by main thread
                    //
                    return;
                }

                callback = _callback;
            }

            if(callback != null)
            {
                callback(sig);
            }
        }

        //
        // The callbacks to be invoked from the handler.
        //
        private static void destroyOnInterruptCallback(int sig)
        {
            lock(_mutex)
            {
                if(_destroyed)
                {
                    //
                    // Being destroyed by main thread
                    //
                    return;
                }
                if(_nohup && sig == SIGHUP)
                {
                    return;
                }

                Debug.Assert(!_callbackInProgress);
                _callbackInProgress = true;
                _interrupted = true;
                _destroyed = true;
            }

            try
            {
                Debug.Assert(_communicator != null);
                _communicator.destroy();
            }
            catch(System.Exception ex)
            {
                Util.getProcessLogger().error(
                    _appName + " (while destroying in response to signal " + sig + "):\n" + ex);
            }

            lock(_mutex)
            {
                _callbackInProgress = false;
                Monitor.Pulse(_mutex);
            }
        }

        private static void shutdownOnInterruptCallback(int sig)
        {
            lock(_mutex)
            {
                if(_destroyed)
                {
                    //
                    // Being destroyed by main thread
                    //
                    return;
                }
                if(_nohup && sig == SIGHUP)
                {
                    return;
                }

                Debug.Assert(!_callbackInProgress);
                _callbackInProgress = true;
                _interrupted = true;
            }

            try
            {
                Debug.Assert(_communicator != null);
                _communicator.shutdown();
            }
            catch(System.Exception ex)
            {
                Util.getProcessLogger().error(
                    _appName + " (while shutting down in response to signal " + sig + "):\n" + ex);
            }

            lock(_mutex)
            {
                _callbackInProgress = false;
                Monitor.Pulse(_mutex);
            }
        }

        private static void userCallbackOnInterruptCallback(int sig)
        {
            lock(_mutex)
            {
                if(_destroyed)
                {
                    //
                    // Being destroyed by main thread
                    //
                    return;
                }
                // For SIGHUP the user callback is always called. It can
                // decide what to do.
                Debug.Assert(!_callbackInProgress);
                _callbackInProgress = true;
                _interrupted = true;
            }

            try
            {
                Debug.Assert(_application != null);
                _application.interruptCallback(sig);
            }
            catch(System.Exception ex)
            {
                Util.getProcessLogger().error(
                    _appName + " (while interrupting in response to signal " + sig + "):\n" + ex);
            }

            lock(_mutex)
            {
                _callbackInProgress = false;
                Monitor.Pulse(_mutex);
            }
        }

        private static readonly object _mutex = new object();

        private static bool _callbackInProgress = false;
        private static bool _destroyed = false;
        private static bool _interrupted = false;
        private static bool _released = false;
        private static bool _nohup = false;
        private static SignalPolicy _signalPolicy = SignalPolicy.HandleSignals;

        private delegate void Callback(int sig);
        private static readonly Callback _destroyCallback = new Callback(destroyOnInterruptCallback);
        private static readonly Callback _shutdownCallback = new Callback(shutdownOnInterruptCallback);
        private static readonly Callback _holdCallback = new Callback(holdInterruptCallback);
        private static readonly Callback _userCallback = new Callback(userCallbackOnInterruptCallback);

        private static Callback _callback = null; // Current callback
        private static Callback _previousCallback; // Remembers prev. callback when signals are held

        //
        // We use FriendlyName instead of Process.GetCurrentProcess().ProcessName because the latter
        // is terribly slow. (It takes around 1 second!)
        //
        private static string _appName = AppDomain.CurrentDomain.FriendlyName;
        private static Communicator _communicator;
        private static Application _application;

        private static int SIGHUP;
        static Application()
        {
            if(IceInternal.AssemblyUtil.platform_ == IceInternal.AssemblyUtil.Platform.Windows)
            {
                SIGHUP = 5; // CTRL_LOGOFF_EVENT, from wincon.h
            }
            else
            {
                SIGHUP = 1;
            }
        }

        private delegate void SignalHandler(int sig);
        private static readonly SignalHandler _handler = new SignalHandler(signalHandler);
        private Signals _signals;

        private interface Signals
        {
            void register(SignalHandler handler);
        }

        private class MonoSignals : Signals
        {
            public void register(SignalHandler handler)
            {
                try
                {
                    //
                    // Signal handling in Mono is provided in the Mono.Unix.Native namespace.
                    // We use reflection to do the equivalent of the following:
                    //
                    // Stdlib.signal(Signum.SIGHUP, delegate);
                    // Stdlib.signal(Signum.SIGINT, delegate);
                    // Stdlib.signal(Signum.SIGTERM, delegate);
                    //
                    // We don't use conditional compilation so that the Ice assembly can be
                    // used without change on Windows and Mono.
                    //
                    Assembly a = Assembly.Load(
                        "Mono.Posix, Version=1.0.5000.0, Culture=neutral, PublicKeyToken=0738eb9f132ed756");
                    Type sigs = a.GetType("Mono.Unix.Native.Signum");
                    object SIGHUP = Enum.Parse(sigs, "SIGHUP");
                    object SIGINT = Enum.Parse(sigs, "SIGINT");
                    object SIGTERM = Enum.Parse(sigs, "SIGTERM");
                    Type stdlib = a.GetType("Mono.Unix.Native.Stdlib");
                    MethodInfo method = stdlib.GetMethod("signal", BindingFlags.Static | BindingFlags.Public);
                    Type del = a.GetType("Mono.Unix.Native.SignalHandler");
                    _delegate = Delegate.CreateDelegate(del, handler.Target, handler.Method);
                    object[] args = new object[2];
                    args[0] = SIGHUP;
                    args[1] = _delegate;
                    method.Invoke(null, args);
                    args[0] = SIGINT;
                    args[1] = _delegate;
                    method.Invoke(null, args);
                    args[0] = SIGTERM;
                    args[1] = _delegate;
                    method.Invoke(null, args);
                }
                catch(System.DllNotFoundException)
                {
                    //
                    // The class Mono.Unix.Native.Stdlib requires libMonoPosixHelper.so. Mono raises
                    // DllNotFoundException if it cannot be found in the shared library search path.
                    //
                    Util.getProcessLogger().error("Ice.Application: warning: unable to initialize signals");
                }
                catch(System.Exception)
                {
                    Debug.Assert(false);
                }
            }

            private Delegate _delegate;
        }

        private class WindowsSignals : Signals
        {
#if MANAGED
            public void register(SignalHandler handler)
            {
                //
                // Signals aren't supported in managed code on Windows.
                //
            }
#else
            public void register(SignalHandler handler)
            {
                _handler = handler;
                _callback = new EventHandler(callback);

                bool rc = SetConsoleCtrlHandler(_callback, true);
                Debug.Assert(rc);
            }

            private delegate bool EventHandler(int sig);
            private EventHandler _callback;
            private SignalHandler _handler;

            private bool callback(int sig)
            {
                _handler(sig);
                return true;
            }

            //
            // It's not necessary to wrap DllImport in conditional compilation. The binding occurs
            // at run time, and it will never be executed on Mono.
            //
            [DllImport("kernel32.dll")]
            private static extern bool SetConsoleCtrlHandler(EventHandler eh, bool add);
#endif
        }
    }
}
