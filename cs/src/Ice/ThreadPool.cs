// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

//
// Uncomment these definitions as needed. (We use #define here instead
// of constants to stop the compiler from complaining about unreachable code.)
//
//#define TRACE_REGISTRATION
//#define TRACE_SHUTDOWN
//#define TRACE_MESSAGE
//#define TRACE_EXCEPTION
//#define TRACE_THREAD
//#define TRACE_STACK_TRACE

namespace IceInternal
{

    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Threading;

    public delegate void ThreadPoolWorkItem(bool safeThread);

    public sealed class ThreadPool
    {
        public ThreadPool(Instance instance, string prefix, int timeout)
        {
            _instance = instance;
            _destroyed = false;
            _prefix = prefix;
            _timeout = timeout;
            _size = 0;
            _sizeMax = 0;
            _sizeWarn = 0;
            _threadIndex = 0;
            _running = 0;
            _inUse = 0;
            _load = 1.0;
            _serialize = _instance.initializationData().properties.getPropertyAsInt(_prefix + ".Serialize") > 0;

            string programName = _instance.initializationData().properties.getProperty("Ice.ProgramName");
            if(programName.Length > 0)
            {
                _programNamePrefix = programName + "-";
            }
            else
            {
                _programNamePrefix = "";
            }

            //
            // We use just one thread as the default. This is the fastest
            // possible setting, still allows one level of nesting, and
            // doesn't require to make the servants thread safe.
            //
            int size = _instance.initializationData().properties.getPropertyAsIntWithDefault(_prefix + ".Size", 1);
            if(size < 1)
            {
                string s = _prefix + ".Size < 1; Size adjusted to 1";
                _instance.initializationData().logger.warning(s);
                size = 1;
            }

            int sizeMax =
                _instance.initializationData().properties.getPropertyAsIntWithDefault(_prefix + ".SizeMax", size);
            if(sizeMax < size)
            {
                string s = _prefix + ".SizeMax < " + _prefix + ".Size; SizeMax adjusted to Size (" + size + ")";
                _instance.initializationData().logger.warning(s);
                sizeMax = size;
            }

            int sizeWarn = _instance.initializationData().properties.getPropertyAsIntWithDefault(_prefix + ".SizeWarn",
                                                                              sizeMax * 80 / 100);
            if(sizeWarn > sizeMax)
            {
                string s = _prefix + ".SizeWarn > " + _prefix + ".SizeMax; adjusted SizeWarn to SizeMax ("
                    + sizeMax + ")";
                _instance.initializationData().logger.warning(s);
                sizeWarn = sizeMax;
            }

            _size = size;
            _sizeMax = sizeMax;
            _sizeWarn = sizeWarn;

            try
            {
                _threads = new List<WorkerThread>();
                for(int i = 0; i < _size; ++i)
                {
                    WorkerThread thread = new WorkerThread(this, _programNamePrefix + _prefix + "-" + _threadIndex++);
                    _threads.Add(thread);
                    thread.Start();
                    ++_running;
                }
            }
            catch(System.Exception ex)
            {
                string s = "cannot create thread for `" + _prefix + "':\n" + ex;
                _instance.initializationData().logger.error(s);

                destroy();
                joinWithAllThreads();
                throw;
            }
        }

        public void destroy()
        {
            lock(this)
            {
                #if TRACE_SHUTDOWN
                    trace("destroy");
                #endif

                Debug.Assert(!_destroyed);
                _destroyed = true;
                Monitor.PulseAll(this);
            }
        }

        public void execute(ThreadPoolWorkItem workItem)
        {
            lock(this)
            {
                #if TRACE_REGISTRATION
                    trace("adding work item");
                    #if TRACE_STACK_TRACE
                        Console.Error.WriteLine(Environment.StackTrace);
                    #endif
                #endif

                if(_destroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }
                _workItems.AddLast(workItem);
                Monitor.Pulse(this);
            }
        }

        public void finish(Ice.ConnectionI connection)
        {
            lock(this)
            {
                if(_destroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }
                _finished.AddLast(connection);
                Monitor.Pulse(this);
            }
        }

        public void joinWithAllThreads()
        {
            //
            // _threads is immutable after destroy() has been called,
            // therefore no synchronization is needed. (Synchronization
            // wouldn't be possible here anyway, because otherwise the
            // other threads would never terminate.)
            //
            Debug.Assert(_destroyed);
            foreach(WorkerThread thread in _threads)
            {
                thread.Join();
            }
        }

        public string prefix()
        {
            return _prefix;
        }

        public bool
        serialize()
        {
            return _serialize;
        }

        private void run()
        {
            while(true)
            {
                ThreadPoolWorkItem workItem = null;
                Ice.ConnectionI handler = null;
                lock(this)
                {
                    while(_finished.Count == 0 && _workItems.Count == 0 && !_destroyed)
                    {
                        if(!Monitor.Wait(this, _timeout > 0 ? _timeout * 1000 : Timeout.Infinite))
                        {
                            //
                            // Initiate server shutdown.
                            //
                            try
                            {
                                _instance.objectAdapterFactory().shutdown();
                            }
                            catch(Ice.CommunicatorDestroyedException)
                            {
                            }
                            continue;
                        }
                    }

                    Debug.Assert(_finished.Count > 0 || _workItems.Count > 0 || _destroyed);

                    //
                    // There are two possibilities for a notification:
                    //
                    // 1. The thread pool has been destroyed.
                    //
                    // 2. A work item has been scheduled.
                    //

                    if(_finished.Count > 0)
                    {
                        handler = _finished.First.Value;
                        _finished.RemoveFirst();
                    }
                    else if(_workItems.Count > 0)
                    {
                        //
                        // Work items must be executed first even if the thread pool is destroyed.
                        //
                        workItem = _workItems.First.Value;
                        _workItems.RemoveFirst();
                    }
                    else
                    {
                        Debug.Assert(_destroyed);
                        return;
                    }

                    if(_sizeMax > 1)
                    {
                        Debug.Assert(_inUse >= 0);
                        ++_inUse;

                        if(_inUse == _sizeWarn)
                        {
                            string s = "thread pool `" + _prefix + "' is running low on threads\n"
                                       + "Size=" + _size + ", " + "SizeMax=" + _sizeMax + ", "
                                       + "SizeWarn=" + _sizeWarn;
                            _instance.initializationData().logger.warning(s);
                        }

                        Debug.Assert(_inUse <= _running);
                        if(_inUse < _sizeMax && _inUse == _running)
                        {
                            try
                            {
                                WorkerThread thread = new WorkerThread(this, _programNamePrefix + _prefix + "-" +
                                                                       _threadIndex++);
                                thread.Start();
                                _threads.Add(thread);
                                ++_running;
                            }
                            catch(System.Exception ex)
                            {
                                string s = "cannot create thread for `" + _prefix + "':\n" + ex;
                                _instance.initializationData().logger.error(s);
                            }
                        }
                    }
                }

                //
                // Now we are outside the thread synchronization.
                //
                if(workItem != null)
                {
                    try
                    {
                        //
                        // Execute the work item and indicate whether this thread is safe (i.e., will not be reaped).
                        //
                        workItem(_size == _sizeMax);
                    }
                    catch(Ice.LocalException ex)
                    {
                        string s = "exception in `" + _prefix + "' while calling execute():\n" + ex;
                        _instance.initializationData().logger.error(s);
                    }
                }
                else 
                {
                    Debug.Assert(handler != null);
                    try
                    {
                        //
                        // Execute the work item and indicate whether this thread is safe (i.e., will not be reaped).
                        //
                        handler.finished(this);
                    }
                    catch(Ice.LocalException ex)
                    {
                        string s = "exception in `" + _prefix + "' while calling finished():\n" + ex;
                        _instance.initializationData().logger.error(s);
                    }
                }
                
                if(_sizeMax > 1)
                {
                    lock(this)
                    {
                        if(!_destroyed)
                        {
                            if(_size < _sizeMax) // Dynamic thread pool
                            {
                                //
                                // First we reap threads that have been destroyed before.
                                //
                                int sz = _threads.Count;
                                Debug.Assert(_running <= sz);
                                if(_running < sz)
                                {
                                    List<WorkerThread> liveThreads = new List<WorkerThread>();
                                    foreach(WorkerThread thread in _threads)
                                    {
                                        if(!thread.IsAlive())
                                        {
                                            thread.Join();
                                        }
                                        else
                                        {
                                            liveThreads.Add(thread);
                                        }
                                    }
                                    _threads = liveThreads;
                                }

                                //
                                // Now we check if this thread can be destroyed, based
                                // on a load factor.
                                //

                                //
                                // The load factor jumps immediately to the number of
                                // threads that are currently in use, but decays
                                // exponentially if the number of threads in use is
                                // smaller than the load factor. This reflects that we
                                // create threads immediately when they are needed,
                                // but want the number of threads to slowly decline to
                                // the configured minimum.
                                //
                                double inUse = (double)_inUse;
                                if(_load < inUse)
                                {
                                    _load = inUse;
                                }
                                else
                                {
                                    double loadFactor = 0.05; // TODO: Configurable?
                                    double oneMinusLoadFactor = 1 - loadFactor;
                                    _load = _load * oneMinusLoadFactor + inUse * loadFactor;
                                }

                                if(_running > _size)
                                {
                                    int load = (int)(_load + 0.5);
                                    if(load + 1 < _running)
                                    {
                                        Debug.Assert(_inUse > 0);
                                        --_inUse;

                                        Debug.Assert(_running > 0);
                                        --_running;
                                        return;
                                    }
                                }
                            }

                            Debug.Assert(_inUse > 0);
                            --_inUse;
                        }
                    }

                    #if TRACE_THREAD
                        trace("thread " + Thread.CurrentThread.Name + " is active");
                    #endif
                }
            }
        }

/*
 * Commented out because it is unused.
 *
        private void trace(string msg)
        {
            System.Console.Error.WriteLine(_prefix + "(" + Thread.CurrentThread.Name + "): " + msg);
        }
*/

        private Instance _instance;
        private bool _destroyed;
        private readonly string _prefix;
        private readonly string _programNamePrefix;

        private LinkedList<ThreadPoolWorkItem> _workItems = new LinkedList<ThreadPoolWorkItem>();
        private LinkedList<Ice.ConnectionI> _finished = new LinkedList<Ice.ConnectionI>();

        private int _timeout;

        private sealed class WorkerThread
        {
            private ThreadPool _threadPool;

            internal WorkerThread(ThreadPool threadPool, string name)
                : base()
            {
                _threadPool = threadPool;
                _name = name;
            }

            public bool IsAlive()
            {
                return _thread.IsAlive;
            }

            public void Join()
            {
                _thread.Join();
            }

            public void Start()
            {
                _thread = new Thread(new ThreadStart(Run));
                _thread.IsBackground = true;
                _thread.Name = _name;
                _thread.Start();
            }

            public void Run()
            {
                if(_threadPool._instance.initializationData().threadHook != null)
                {
                    _threadPool._instance.initializationData().threadHook.start();
                }

                try
                {
                    _threadPool.run();
                }
                catch(Ice.LocalException ex)
                {
                    string s = "exception in `" + _threadPool._prefix + "' thread " + _thread.Name + ":\n" + ex;
                    _threadPool._instance.initializationData().logger.error(s);
                }
                catch(System.Exception ex)
                {
                    string s = "unknown exception in `" + _threadPool._prefix + "' thread " + _thread.Name + ":\n" + ex;
                    _threadPool._instance.initializationData().logger.error(s);
                }

                if(_threadPool._instance.initializationData().threadHook != null)
                {
                    _threadPool._instance.initializationData().threadHook.stop();
                }

                #if TRACE_THREAD
                    _threadPool.trace("run() terminated");
                #endif
            }

            private string _name;
            private Thread _thread;
        }

        private readonly int _size; // Number of threads that are pre-created.
        private readonly int _sizeMax; // Maximum number of threads.
        private readonly int _sizeWarn; // If _inUse reaches _sizeWarn, a "low on threads" warning will be printed.
        private readonly bool _serialize; // True if requests need to be serialized over the connection.

        private List<WorkerThread> _threads; // All threads, running or not.
        private int _threadIndex; // For assigning thread names.
        private int _running; // Number of running threads.
        private int _inUse; // Number of threads that are currently in use.
        private double _load; // Current load in number of threads.
    }

}
