// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Ice
{

    using System;
    using System.Diagnostics;
    using System.Reflection;
    using System.Runtime.InteropServices;
    using System.Threading;

    public enum SignalPolicy { HandleSignals, NoSignalHandling }

    public abstract class Application
    {
        public abstract int run(string[] args);

        //
        // Override this method to provide a custom application interrupt
        // hook. You must call callbackOnInterrupt for this method
        // to be called. Note that the interruptCallback can be called
        // concurrently with any other thread (including main) in your
        // application--take appropriate concurrency precautions.
        //
        public virtual void interruptCallback(int sig)
        {
        }

        public Application()
        {
        }

        public Application(SignalPolicy signalPolicy)
        {
            _signalPolicy = signalPolicy;
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
            return main(args, new InitializationData());
        }

        public int main(string[] args, string configFile)
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
                    Console.Error.WriteLine(_appName + ":\n" + ex);
                    return 1;
                }
                catch(System.Exception ex)
                {
                    Console.Error.WriteLine(_appName + ": unknown exception:\n" + ex);
                    return 1;
                }
            }
            return main(args, initData);
        }

        public int main(string[] args, InitializationData initData)
        {
            if(_communicator != null)
            {
                Console.Error.WriteLine(_appName + ": only one instance of the Application class can be used");
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

                status = mainInternal(args, initData);

                _signals = null;
            }
            else
            {
                status = mainInternal(args, initData);
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
                Console.Error.WriteLine(_appName +
                            ": warning: interrupt method called on Application configured to not handle interrupts.");
            }
        }

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
                Console.Error.WriteLine(_appName +
                            ": warning: interrupt method called on Application configured to not handle interrupts.");
            }
        }

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
                Console.Error.WriteLine(_appName +
                            ": warning: interrupt method called on Application configured to not handle interrupts.");
            }
        }

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
                Console.Error.WriteLine(_appName +
                            ": warning: interrupt method called on Application configured to not handle interrupts.");
            }
        }

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
                Console.Error.WriteLine(_appName +
                            ": warning: interrupt method called on Application configured to not handle interrupts.");
            }
        }

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
                Console.Error.WriteLine(_appName +
                            ": warning: interrupt method called on Application configured to not handle interrupts.");
            }
        }

        public static bool interrupted()
        {
            lock(_mutex)
            {
                return _interrupted;
            }
        }

        private int mainInternal(string[] args, InitializationData initializationData)
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

                //
                // If the process logger is the default logger, we replace it with a
                // a logger which is using the program name for the prefix.
                //
                if(Util.getProcessLogger() is LoggerI)
                {
                    Util.setProcessLogger(new LoggerI(initData.properties.getProperty("Ice.ProgramName")));
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
                Console.Error.WriteLine(_appName + ":\n" + ex);
                status = 1;
            }
            catch(System.Exception ex)
            {
                Console.Error.WriteLine(_appName + ": unknown exception:\n" + ex);
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
                    Console.Error.WriteLine(_appName + ":\n" + ex);
                    status = 1;
                }
                catch(System.Exception ex)
                {
                    Console.Error.WriteLine(_appName + ": unknown exception:\n" + ex);
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
                Console.Error.WriteLine(_appName + " (while destroying in response to signal " + sig + "):\n" + ex);
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
                Console.Error.WriteLine(_appName + " (while shutting down in response to signal " + sig + "):\n" + ex);
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
                Console.Error.WriteLine(_appName + " (while interrupting in response to signal " + sig + "):\n" + ex);
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
                    Console.Error.WriteLine("Ice.Application: warning: unable to initialize signals");
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
