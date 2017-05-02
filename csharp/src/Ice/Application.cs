// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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

    internal static class NativeMethods
    {
        [DllImport("kernel32.dll")]
        [return: MarshalAsAttribute(UnmanagedType.Bool)]
        internal static extern bool
        SetConsoleCtrlHandler(CtrlCEventHandler eh, [MarshalAsAttribute(UnmanagedType.Bool)]bool add);
    }

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
    /// Utility base class that makes it easy to correctly initialize and finalize
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
        /// Initializes an instance that handles signals according to the signal policy.
        /// If not signal policy is provided the default SinalPolicy.HandleSignals
        /// will be used, which calls Communicator.shutdown if a signal is received.
        /// </summary>
        /// <param name="signalPolicy">Determines how to respond to signals.</param>
        public Application(SignalPolicy signalPolicy = SignalPolicy.HandleSignals)
        {
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
            return main(args, new InitializationData());
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
            if(Util.getProcessLogger() is ConsoleLoggerI)
            {
                Util.setProcessLogger(new ConsoleLoggerI(iceAppName));
            }

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
                    Util.getProcessLogger().error(ex.ToString());
                    return 1;
                }
                catch(System.Exception ex)
                {
                    Util.getProcessLogger().error("unknown exception:\n" + ex);
                    return 1;
                }
            }
            return main(args, initData);
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
        /// <param name="initializationData">Additional data used to initialize the communicator.</param>
        /// <returns>The value returned by run. If run terminates with an exception,
        /// the return value is non-zero.</returns>
        public int main(string[] args, InitializationData initializationData)
        {
            if(Util.getProcessLogger() is ConsoleLoggerI)
            {
                Util.setProcessLogger(new ConsoleLoggerI(iceAppName));
            }

            if(iceCommunicator != null)
            {
                Util.getProcessLogger().error("only one instance of the Application class can be used");
                return 1;
            }

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

            try
            {
                initData.properties = Util.createProperties(ref args, initData.properties);
            }
            catch(Ice.Exception ex)
            {
                Util.getProcessLogger().error(ex.ToString());
                return 1;
            }
            catch(System.Exception ex)
            {
                Util.getProcessLogger().error("unknown exception:\n" + ex);
                return 1;
            }
            iceAppName = initData.properties.getPropertyWithDefault("Ice.ProgramName", iceAppName);

            iceNohup = initData.properties.getPropertyAsInt("Ice.Nohup") > 0;
            _application = this;

            int status;
            if(iceSignalPolicy == SignalPolicy.HandleSignals)
            {
                _signals = new WindowsSignals();
                _signals.register(_handler);

                status = doMain(args, initData);

                _signals.destroy();
                _signals = null;
            }
            else
            {
                status = doMain(args, initData);
            }

            return status;
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
            return iceAppName;
        }

        /// <summary>
        /// Returns the communicator for the application. Because communicator is a static method,
        /// it permits access to the communicator from anywhere in the program. Note that, as a consequence,
        /// you cannot have more than one instance of Application in a program.
        /// </summary>
        /// <returns>The communicator for the application.</returns>
        public static Communicator communicator()
        {
            return iceCommunicator;
        }

        /// <summary>
        /// Instructs Application to call Communicator.destroy on receipt of a signal.
        /// This is default signal handling policy established by the default constructor.
        /// </summary>
        public static void destroyOnInterrupt()
        {
            if(iceSignalPolicy == SignalPolicy.HandleSignals)
            {
                lock(iceMutex)
                {
                    if(_callback == _holdCallback)
                    {
                        iceReleased = true;
                        System.Threading.Monitor.Pulse(iceMutex);
                    }
                    _callback = _destroyCallback;
                }
            }
            else
            {
                Util.getProcessLogger().warning(
                    "interrupt method called on Application configured to not handle interrupts.");
            }
        }

        /// <summary>
        /// Instructs Application to call Communicator.shutdown on receipt of a signal.
        /// </summary>
        public static void shutdownOnInterrupt()
        {
            if(iceSignalPolicy == SignalPolicy.HandleSignals)
            {
                lock(iceMutex)
                {
                    if(_callback == _holdCallback)
                    {
                        iceReleased = true;
                        System.Threading.Monitor.Pulse(iceMutex);
                    }
                    _callback = _shutdownCallback;
                }
            }
            else
            {
                Util.getProcessLogger().warning(
                            "interrupt method called on Application configured to not handle interrupts.");
            }
        }

        /// <summary>
        /// Instructs Application to ignore signals.
        /// </summary>
        public static void ignoreInterrupt()
        {
            if(iceSignalPolicy == SignalPolicy.HandleSignals)
            {
                lock(iceMutex)
                {
                    if(_callback == _holdCallback)
                    {
                        iceReleased = true;
                        System.Threading.Monitor.Pulse(iceMutex);
                    }
                    _callback = null;
                }
            }
            else
            {
                Util.getProcessLogger().warning(
                            "interrupt method called on Application configured to not handle interrupts.");
            }
        }

        /// <summary>
        /// Instructs Application to call interruptCallback on receipt of a signal.
        /// The derived class can intercept signals by overriding interruptCallback.
        /// </summary>
        public static void callbackOnInterrupt()
        {
            if(iceSignalPolicy == SignalPolicy.HandleSignals)
            {
                lock(iceMutex)
                {
                    if(_callback == _holdCallback)
                    {
                        iceReleased = true;
                        System.Threading.Monitor.Pulse(iceMutex);
                    }
                    _callback = _userCallback;
                }
            }
            else
            {
                Util.getProcessLogger().warning(
                            "interrupt method called on Application configured to not handle interrupts.");
            }
        }

        /// <summary>
        /// Instructs Application to call to hold signals.
        /// </summary>
        public static void holdInterrupt()
        {
            if(iceSignalPolicy == SignalPolicy.HandleSignals)
            {
                lock(iceMutex)
                {
                    if(_callback != _holdCallback)
                    {
                        _previousCallback = _callback;
                        iceReleased = false;
                        _callback = _holdCallback;
                    }
                    // else, we were already holding signals
                }
            }
            else
            {
                Util.getProcessLogger().warning(
                            "interrupt method called on Application configured to not handle interrupts.");
            }
        }

        /// <summary>
        /// Instructs Application respond to signals. If a signal arrived since the last call
        /// to holdInterrupt, it is delivered once you call releaseInterrupt.
        /// </summary>
        public static void releaseInterrupt()
        {
            if(iceSignalPolicy == SignalPolicy.HandleSignals)
            {
                lock(iceMutex)
                {
                    if(_callback == _holdCallback)
                    {
                        //
                        // Note that it's very possible no signal is held;
                        // in this case the callback is just replaced and
                        // setting iceReleased to true and signalling this
                        // will do no harm.
                        //

                        iceReleased = true;
                        _callback = _previousCallback;
                        System.Threading.Monitor.Pulse(iceMutex);
                    }
                    // Else nothing to release.
                }
            }
            else
            {
                Util.getProcessLogger().warning(
                            "interrupt method called on Application configured to not handle interrupts.");
            }
        }

        /// <summary>
        /// Determines whether the application shut down intentionally or was forced to shut down due to a signal.
        /// This is useful for logging purposes.
        /// </summary>
        /// <returns>True if a signal caused the communicator to shut down; false otherwise.</returns>
        public static bool interrupted()
        {
            lock(iceMutex)
            {
                return iceInterrupted;
            }
        }

        protected virtual int doMain(string[] args, InitializationData initData)
        {
            int status = 0;

            try
            {
                //
                // If the process logger is the default logger, we replace it with a
                // a logger which is using the program name for the prefix.
                //
                if(initData.properties.getProperty("Ice.ProgramName").Length > 0 &&
                   Util.getProcessLogger() is ConsoleLoggerI)
                {
                    Util.setProcessLogger(new ConsoleLoggerI(initData.properties.getProperty("Ice.ProgramName")));
                }

                iceCommunicator = Util.initialize(ref args, initData);
                iceDestroyed = false;

                //
                // The default is to destroy when a signal is received.
                //
                if(iceSignalPolicy == SignalPolicy.HandleSignals)
                {
                    destroyOnInterrupt();
                }

                status = run(args);
            }
            catch(Ice.Exception ex)
            {
                Util.getProcessLogger().error(ex.ToString());
                status = 1;
            }
            catch(System.Exception ex)
            {
                Util.getProcessLogger().error("unknown exception:\n" + ex);
                status = 1;
            }

            //
            // Don't want any new interrupt. And at this point
            // (post-run), it would not make sense to release a held
            // signal to run shutdown or destroy.
            //
            if(iceSignalPolicy == SignalPolicy.HandleSignals)
            {
                ignoreInterrupt();
            }

            lock(iceMutex)
            {
                while(iceCallbackInProgress)
                {
                    System.Threading.Monitor.Wait(iceMutex);
                }
                if(iceDestroyed)
                {
                    iceCommunicator = null;
                }
                else
                {
                    iceDestroyed = true;
                    //
                    // iceCommunicator != null means that it will be destroyed
                    // next; iceDestroyed == true ensures that any
                    // remaining callback won't do anything
                    //
                }
                _application = null;
            }

            if(iceCommunicator != null)
            {
                try
                {
                    iceCommunicator.destroy();
                }
                catch(Ice.Exception ex)
                {
                    Util.getProcessLogger().error(ex.ToString());
                    status = 1;
                }
                catch(System.Exception ex)
                {
                    Util.getProcessLogger().error("unknown exception:\n" + ex);
                    status = 1;
                }
                iceCommunicator = null;
            }

            return status;
        }

        //
        // First-level handler.
        //
        private static void signalHandler(int sig)
        {
            Callback callback;

            lock(iceMutex)
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
            lock(iceMutex)
            {
                while(!iceReleased)
                {
                    System.Threading.Monitor.Wait(iceMutex);
                }

                if(iceDestroyed)
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
            lock(iceMutex)
            {
                if(iceDestroyed)
                {
                    //
                    // Being destroyed by main thread
                    //
                    return;
                }
                if(iceNohup && sig == SIGHUP)
                {
                    return;
                }

                Debug.Assert(!iceCallbackInProgress);
                iceCallbackInProgress = true;
                iceInterrupted = true;
                iceDestroyed = true;
            }

            try
            {
                Debug.Assert(iceCommunicator != null);
                iceCommunicator.destroy();
            }
            catch(System.Exception ex)
            {
                Util.getProcessLogger().error("(while destroying in response to signal " + sig + "):\n" + ex);
            }

            lock(iceMutex)
            {
                iceCallbackInProgress = false;
                System.Threading.Monitor.Pulse(iceMutex);
            }
        }

        private static void shutdownOnInterruptCallback(int sig)
        {
            lock(iceMutex)
            {
                if(iceDestroyed)
                {
                    //
                    // Being destroyed by main thread
                    //
                    return;
                }
                if(iceNohup && sig == SIGHUP)
                {
                    return;
                }

                Debug.Assert(!iceCallbackInProgress);
                iceCallbackInProgress = true;
                iceInterrupted = true;
            }

            try
            {
                Debug.Assert(iceCommunicator != null);
                iceCommunicator.shutdown();
            }
            catch(System.Exception ex)
            {
                Util.getProcessLogger().error("(while shutting down in response to signal " + sig + "):\n" + ex);
            }

            lock(iceMutex)
            {
                iceCallbackInProgress = false;
                System.Threading.Monitor.Pulse(iceMutex);
            }
        }

        private static void userCallbackOnInterruptCallback(int sig)
        {
            lock(iceMutex)
            {
                if(iceDestroyed)
                {
                    //
                    // Being destroyed by main thread
                    //
                    return;
                }
                // For SIGHUP the user callback is always called. It can
                // decide what to do.
                Debug.Assert(!iceCallbackInProgress);
                iceCallbackInProgress = true;
                iceInterrupted = true;
            }

            try
            {
                Debug.Assert(_application != null);
                _application.interruptCallback(sig);
            }
            catch(System.Exception ex)
            {
                Util.getProcessLogger().error("(while interrupting in response to signal " + sig + "):\n" + ex);
            }

            lock(iceMutex)
            {
                iceCallbackInProgress = false;
                System.Threading.Monitor.Pulse(iceMutex);
            }
        }

        protected static object iceMutex = new object();
        protected static bool iceCallbackInProgress = false;
        protected static bool iceDestroyed = false;
        protected static bool iceInterrupted = false;
        protected static bool iceReleased = false;
        protected static bool iceNohup = false;
        protected static SignalPolicy iceSignalPolicy = SignalPolicy.HandleSignals;

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
        protected static string iceAppName = AppDomain.CurrentDomain.FriendlyName;
        protected static Communicator iceCommunicator;
        private static Application _application;

        private static int SIGHUP;
        static Application()
        {
            SIGHUP = 5; // CTRL_LOGOFF_EVENT, from wincon.h
        }

        private delegate void SignalHandler(int sig);
        private static readonly SignalHandler _handler = new SignalHandler(signalHandler);
        private Signals _signals;

        private interface Signals
        {
            void register(SignalHandler handler);
            void destroy();
        }

        private class WindowsSignals : Signals
        {
#if MANAGED
            public void register(SignalHandler handler)
            {
                _handler = handler;
                Console.CancelKeyPress += delegate(object sender, ConsoleCancelEventArgs args)
                {
                    args.Cancel = true;
                    _handler(0);
                };
            }

            public void destroy()
            {
            }
#else
            public void register(SignalHandler handler)
            {
                _handler = handler;
                _callback = new CtrlCEventHandler(callback);

                bool rc = NativeMethods.SetConsoleCtrlHandler(_callback, true);
                Debug.Assert(rc);
            }

            public void destroy()
            {
            }

            private CtrlCEventHandler _callback;

            private bool callback(int sig)
            {
                _handler(sig);
                return true;
            }
#endif
            private SignalHandler _handler;
        }
    }

    delegate bool CtrlCEventHandler(int sig);
}
