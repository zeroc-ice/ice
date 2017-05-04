// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Threading;

    public delegate void ThreadPoolWorkItem();
    public delegate void AsyncCallback(object state);

    //
    // Thread pool threads set a custom synchronization context to ensure that
    // continuations from awaited methods continue executing on the thread pool
    // and not on the thread that notifies the awaited task.
    //
    sealed class ThreadPoolSynchronizationContext : SynchronizationContext
    {
        public ThreadPoolSynchronizationContext(ThreadPool threadPool)
        {
            _threadPool = threadPool;
        }

        public override void Post(SendOrPostCallback d, object state)
        {
            //
            // Dispatch the continuation on the thread pool if this isn't called
            // already from a thread pool thread. We don't use the dispatcher
            // for the continuations, the dispatcher is only used when the
            // call is initialy invoked (e.g.: a servant dispatch after being
            // received is dispatched using the dispatcher which might dispatch
            // the call on the UI thread which will then use its own synchronization
            // context to execute continuations).
            //
            var ctx = Current as ThreadPoolSynchronizationContext;
            if(ctx != this)
            {
                _threadPool.dispatch(() => { d(state); }, null, false);
            }
            else
            {
                d(state);
            }
        }

        public override void Send(SendOrPostCallback d, object state)
        {
            throw new System.NotSupportedException("the thread pool doesn't support synchronous calls");
        }

        private ThreadPool _threadPool;
    }

    internal struct ThreadPoolMessage
    {
        public ThreadPoolMessage(object mutex)
        {
            _mutex = mutex;
            _finish = false;
            _finishWithIO = false;
        }

        public bool startIOScope(ref ThreadPoolCurrent current)
        {
            // This must be called with the handler locked.
            _finishWithIO = current.startMessage();
            return _finishWithIO;
        }

        public void finishIOScope(ref ThreadPoolCurrent current)
        {
            if(_finishWithIO)
            {
                lock(_mutex)
                {
                    current.finishMessage(true);
                }
            }
        }

        public void completed(ref ThreadPoolCurrent current)
        {
            //
            // Call finishMessage once IO is completed only if serialization is not enabled.
            // Otherwise, finishMessage will be called when the event handler is done with
            // the message (it will be called from destroy below).
            //
            Debug.Assert(_finishWithIO);
            if(current.ioCompleted())
            {
                _finishWithIO = false;
                _finish = true;
            }
        }

        public void destroy(ref ThreadPoolCurrent current)
        {
            if(_finish)
            {
                //
                // A ThreadPoolMessage instance must be created outside the synchronization
                // of the event handler. We need to lock the event handler here to call
                // finishMessage.
                //
                lock(_mutex)
                {
                    current.finishMessage(false);
                    Debug.Assert(!current.completedSynchronously);
                }
            }
        }

        private object _mutex;
        private bool _finish;
        private bool _finishWithIO;
    }

    public struct ThreadPoolCurrent
    {
        public ThreadPoolCurrent(ThreadPool threadPool, EventHandler handler, int op)
        {
            _threadPool = threadPool;
            _handler = handler;
            operation = op;
            completedSynchronously = false;
        }

        public readonly int operation;
        public bool completedSynchronously;

        public bool ioCompleted()
        {
            return _threadPool.serialize();
        }

        public bool startMessage()
        {
            return _threadPool.startMessage(ref this);
        }

        public void finishMessage(bool fromIOThread)
        {
            _threadPool.finishMessage(ref this, fromIOThread);
        }

        internal readonly ThreadPool _threadPool;
        internal readonly EventHandler _handler;
    }

    public sealed class ThreadPool : System.Threading.Tasks.TaskScheduler
    {
        public ThreadPool(Instance instance, string prefix, int timeout)
        {
            Ice.Properties properties = instance.initializationData().properties;

            _instance = instance;
            _dispatcher = instance.initializationData().dispatcher;
            _destroyed = false;
            _prefix = prefix;
            _threadIndex = 0;
            _inUse = 0;
            _serialize = properties.getPropertyAsInt(_prefix + ".Serialize") > 0;
            _serverIdleTime = timeout;

            string programName = properties.getProperty("Ice.ProgramName");
            if(programName.Length > 0)
            {
                _threadPrefix = programName + "-" + _prefix;
            }
            else
            {
                _threadPrefix = _prefix;
            }

            //
            // We use just one thread as the default. This is the fastest
            // possible setting, still allows one level of nesting, and
            // doesn't require to make the servants thread safe.
            //
            int size = properties.getPropertyAsIntWithDefault(_prefix + ".Size", 1);
            if(size < 1)
            {
                string s = _prefix + ".Size < 1; Size adjusted to 1";
                _instance.initializationData().logger.warning(s);
                size = 1;
            }

            int sizeMax = properties.getPropertyAsIntWithDefault(_prefix + ".SizeMax", size);
            if(sizeMax < size)
            {
                string s = _prefix + ".SizeMax < " + _prefix + ".Size; SizeMax adjusted to Size (" + size + ")";
                _instance.initializationData().logger.warning(s);
                sizeMax = size;
            }

            int sizeWarn = properties.getPropertyAsInt(_prefix + ".SizeWarn");
            if(sizeWarn != 0 && sizeWarn < size)
            {
                string s = _prefix + ".SizeWarn < " + _prefix + ".Size; adjusted SizeWarn to Size (" + size + ")";
                _instance.initializationData().logger.warning(s);
                sizeWarn = size;
            }
            else if(sizeWarn > sizeMax)
            {
                string s = _prefix + ".SizeWarn > " + _prefix + ".SizeMax; adjusted SizeWarn to SizeMax ("
                    + sizeMax + ")";
                _instance.initializationData().logger.warning(s);
                sizeWarn = sizeMax;
            }

            int threadIdleTime = properties.getPropertyAsIntWithDefault(_prefix + ".ThreadIdleTime", 60);
            if(threadIdleTime < 0)
            {
                string s = _prefix + ".ThreadIdleTime < 0; ThreadIdleTime adjusted to 0";
                _instance.initializationData().logger.warning(s);
                threadIdleTime = 0;
            }

            _size = size;
            _sizeMax = sizeMax;
            _sizeWarn = sizeWarn;
            _threadIdleTime = threadIdleTime;

            int stackSize = properties.getPropertyAsInt(_prefix + ".StackSize");
            if(stackSize < 0)
            {
                string s = _prefix + ".StackSize < 0; Size adjusted to OS default";
                _instance.initializationData().logger.warning(s);
                stackSize = 0;
            }
            _stackSize = stackSize;

            _priority = properties.getProperty(_prefix + ".ThreadPriority").Length > 0 ?
                Util.stringToThreadPriority(properties.getProperty(_prefix + ".ThreadPriority")) :
                Util.stringToThreadPriority(properties.getProperty("Ice.ThreadPriority"));

            if(_instance.traceLevels().threadPool >= 1)
            {
                string s = "creating " + _prefix + ": Size = " + _size + ", SizeMax = " + _sizeMax + ", SizeWarn = " +
                           _sizeWarn;
                _instance.initializationData().logger.trace(_instance.traceLevels().threadPoolCat, s);
            }

            _workItems = new Queue<ThreadPoolWorkItem>();

            try
            {
                _threads = new List<WorkerThread>();
                for(int i = 0; i < _size; ++i)
                {
                    WorkerThread thread = new WorkerThread(this, _threadPrefix + "-" + _threadIndex++);
                    thread.start(_priority);
                    _threads.Add(thread);
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
                if(_destroyed)
                {
                    return;
                }
                _destroyed = true;
                Monitor.PulseAll(this);
            }
        }

        public void updateObservers()
        {
            lock(this)
            {
                foreach(WorkerThread t in _threads)
                {
                    t.updateObserver();
                }
            }
        }

        public void initialize(EventHandler handler)
        {
            handler._ready = 0;
            handler._pending = 0;
            handler._started = 0;
            handler._finish = false;
            handler._hasMoreData = false;
            handler._registered = 0;
        }

        public void register(EventHandler handler, int op)
        {
            update(handler, SocketOperation.None, op);
        }

        public void update(EventHandler handler, int remove, int add)
        {
            lock(this)
            {
                Debug.Assert(!_destroyed);

                // Don't remove what needs to be added
                remove &= ~add;

                // Don't remove/add if already un-registered or registered
                remove &= handler._registered;
                add &= ~handler._registered;
                if(remove == add)
                {
                    return;
                }

                handler._registered &= ~remove;
                handler._registered |= add;

                if((add & SocketOperation.Read) != 0 && (handler._pending & SocketOperation.Read) == 0)
                {
                    handler._pending |= SocketOperation.Read;
                    executeNonBlocking(() =>
                        {
                            messageCallback(new ThreadPoolCurrent(this, handler, SocketOperation.Read));
                        });
                }
                else if((add & SocketOperation.Write) != 0 && (handler._pending & SocketOperation.Write) == 0)
                {
                    handler._pending |= SocketOperation.Write;
                    executeNonBlocking(() =>
                        {
                            messageCallback(new ThreadPoolCurrent(this, handler, SocketOperation.Write));
                        });
                }
            }
        }

        public void unregister(EventHandler handler, int op)
        {
            update(handler, op, SocketOperation.None);
        }

        public void finish(EventHandler handler)
        {
            lock(this)
            {
                Debug.Assert(!_destroyed);

                handler._registered = SocketOperation.None;

                //
                // If there are no pending asynchronous operations, we can call finish on the handler now.
                //
                if(handler._pending == 0)
                {
                    executeNonBlocking(() =>
                       {
                           ThreadPoolCurrent current = new ThreadPoolCurrent(this, handler, SocketOperation.None);
                           handler.finished(ref current);
                       });
                }
                else
                {
                    handler._finish = true;
                }
            }
        }

        public void dispatchFromThisThread(System.Action call, Ice.Connection con)
        {
            if(_dispatcher != null)
            {
                try
                {
                    _dispatcher(call, con);
                }
                catch(System.Exception ex)
                {
                    if(_instance.initializationData().properties.getPropertyAsIntWithDefault(
                           "Ice.Warn.Dispatch", 1) > 1)
                    {
                        _instance.initializationData().logger.warning("dispatch exception:\n" + ex);
                    }
                }
            }
            else
            {
                call();
            }
        }

        public void dispatch(System.Action call, Ice.Connection con, bool useDispatcher = true)
        {
            lock(this)
            {
                if(_destroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }

                if(useDispatcher)
                {
                    _workItems.Enqueue(() => { dispatchFromThisThread(call, con); });
                }
                else
                {
                    _workItems.Enqueue(() => { call(); });
                }
                Monitor.Pulse(this);

                //
                // If this is a dynamic thread pool which can still grow and if all threads are
                // currently busy dispatching or about to dispatch, we spawn a new thread to
                // execute this new work item right away.
                //
                if(_threads.Count < _sizeMax &&
                   (_inUse + _workItems.Count) > _threads.Count &&
                   !_destroyed)
                {
                    if(_instance.traceLevels().threadPool >= 1)
                    {
                        string s = "growing " + _prefix + ": Size = " + (_threads.Count + 1);
                        _instance.initializationData().logger.trace(_instance.traceLevels().threadPoolCat, s);
                    }

                    try
                    {
                        WorkerThread t = new WorkerThread(this, _threadPrefix + "-" + _threadIndex++);
                        t.start(_priority);
                        _threads.Add(t);
                    }
                    catch(System.Exception ex)
                    {
                        string s = "cannot create thread for `" + _prefix + "':\n" + ex;
                        _instance.initializationData().logger.error(s);
                    }
                }
            }
        }

        public void executeNonBlocking(ThreadPoolWorkItem workItem)
        {
            lock(this)
            {
                Debug.Assert(!_destroyed);
                _instance.asyncIOThread().queue(workItem);
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
                thread.join();
            }
        }

        public string prefix()
        {
            return _prefix;
        }

        public bool serialize()
        {
            return _serialize;
        }

        protected sealed override void QueueTask(System.Threading.Tasks.Task task)
        {
            dispatch(() => { TryExecuteTask(task); }, null, _dispatcher != null);
        }

        protected sealed override bool TryExecuteTaskInline(System.Threading.Tasks.Task task, bool taskWasPreviouslyQueued)
        {
            if(_dispatcher == null && !taskWasPreviouslyQueued)
            {
                if(_threads.Find(t => t.getThread().ManagedThreadId.Equals(Thread.CurrentThread.ManagedThreadId)) != null)
                {
                    dispatchFromThisThread(() => { TryExecuteTask(task); }, null);
                    return true;
                }
            }
            return false;
        }

        protected sealed override bool TryDequeue(System.Threading.Tasks.Task task)
        {
            return false;
        }

        protected sealed override IEnumerable<System.Threading.Tasks.Task> GetScheduledTasks()
        {
            return new System.Threading.Tasks.Task[0];
        }

        private void run(WorkerThread thread)
        {
            ThreadPoolWorkItem workItem = null;
            while(true)
            {
                lock(this)
                {
                    if(workItem != null)
                    {
                        Debug.Assert(_inUse > 0);
                        --_inUse;
                        if(_workItems.Count == 0)
                        {
                            thread.setState(Ice.Instrumentation.ThreadState.ThreadStateIdle);
                        }
                    }

                    workItem = null;

                    while(_workItems.Count == 0)
                    {
                        if(_destroyed)
                        {
                            return;
                        }

                        if(_threadIdleTime > 0)
                        {
                            if(!Monitor.Wait(this, _threadIdleTime * 1000) && _workItems.Count == 0) // If timeout
                            {
                                if(_destroyed)
                                {
                                    return;
                                }
                                else if(_serverIdleTime == 0 || _threads.Count > 1)
                                {
                                    //
                                    // If not the last thread or if server idle time isn't configured,
                                    // we can exit. Unlike C++/Java, there's no need to have a thread
                                    // always spawned in the thread pool because all the IO is done
                                    // by the .NET thread pool threads. Instead, we'll just spawn a
                                    // new thread when needed (i.e.: when a new work item is queued).
                                    //
                                    if(_instance.traceLevels().threadPool >= 1)
                                    {
                                        string s = "shrinking " + _prefix + ": Size=" + (_threads.Count - 1);
                                        _instance.initializationData().logger.trace(
                                            _instance.traceLevels().threadPoolCat, s);
                                    }

                                    _threads.Remove(thread);
                                    _instance.asyncIOThread().queue(() =>
                                        {
                                            thread.join();
                                        });
                                    return;
                                }
                                else
                                {
                                    Debug.Assert(_serverIdleTime > 0 && _inUse == 0 && _threads.Count == 1);
                                    if(!Monitor.Wait(this, _serverIdleTime * 1000)  &&
                                       _workItems.Count == 0)
                                    {
                                        if(!_destroyed)
                                        {
                                            _workItems.Enqueue(() =>
                                               {
                                                   try
                                                   {
                                                       _instance.objectAdapterFactory().shutdown();
                                                   }
                                                   catch(Ice.CommunicatorDestroyedException)
                                                   {
                                                   }
                                               });
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                            Monitor.Wait(this);
                        }
                    }

                    Debug.Assert(_workItems.Count > 0);
                    workItem = _workItems.Dequeue();

                    Debug.Assert(_inUse >= 0);
                    ++_inUse;

                    thread.setState(Ice.Instrumentation.ThreadState.ThreadStateInUseForUser);

                    if(_sizeMax > 1 && _inUse == _sizeWarn)
                    {
                        string s = "thread pool `" + _prefix + "' is running low on threads\n"
                            + "Size=" + _size + ", " + "SizeMax=" + _sizeMax + ", " + "SizeWarn=" + _sizeWarn;
                        _instance.initializationData().logger.warning(s);
                    }
                }

                try
                {
                    workItem();
                }
                catch(System.Exception ex)
                {
                    string s = "exception in `" + _prefix + "' while calling on work item:\n" + ex + '\n';
                    _instance.initializationData().logger.error(s);
                }
            }
        }

        public bool startMessage(ref ThreadPoolCurrent current)
        {
            Debug.Assert((current._handler._pending & current.operation) != 0);

            if((current._handler._started & current.operation) != 0)
            {
                Debug.Assert((current._handler._ready & current.operation) == 0);
                current._handler._ready |= current.operation;
                current._handler._started &= ~current.operation;
                if(!current._handler.finishAsync(current.operation)) // Returns false if the handler is finished.
                {
                    current._handler._pending &= ~current.operation;
                    if(current._handler._pending == 0 && current._handler._finish)
                    {
                        finish(current._handler);
                    }
                    return false;
                }
            }
            else if((current._handler._ready & current.operation) == 0 &&
                    (current._handler._registered & current.operation) != 0)
            {
                Debug.Assert((current._handler._started & current.operation) == 0);
                bool completed = false;
                if(!current._handler.startAsync(current.operation, getCallback(current.operation), ref completed))
                {
                    current._handler._pending &= ~current.operation;
                    if(current._handler._pending == 0 && current._handler._finish)
                    {
                        finish(current._handler);
                    }
                    return false;
                }
                else
                {
                    current.completedSynchronously = completed;
                    current._handler._started |= current.operation;
                    return false;
                }
            }

            if((current._handler._registered & current.operation) != 0)
            {
                Debug.Assert((current._handler._ready & current.operation) != 0);
                current._handler._ready &= ~current.operation;
                return true;
            }
            else
            {
                current._handler._pending &= ~current.operation;
                if(current._handler._pending == 0 && current._handler._finish)
                {
                    finish(current._handler);
                }
                return false;
            }
        }

        public void finishMessage(ref ThreadPoolCurrent current, bool fromIOThread)
        {
            if((current._handler._registered & current.operation) != 0)
            {
                if(fromIOThread)
                {
                    Debug.Assert((current._handler._ready & current.operation) == 0);
                    bool completed = false;
                    if(!current._handler.startAsync(current.operation, getCallback(current.operation), ref completed))
                    {
                        current._handler._pending &= ~current.operation;
                    }
                    else
                    {
                        Debug.Assert((current._handler._pending & current.operation) != 0);
                        current.completedSynchronously = completed;
                        current._handler._started |= current.operation;
                    }
                }
                else
                {
                    ThreadPoolCurrent c = current;
                    executeNonBlocking(() =>
                       {
                           messageCallback(c);
                       });
                }
            }
            else
            {
                current._handler._pending &= ~current.operation;
            }

            if(current._handler._pending == 0 && current._handler._finish)
            {
                // There are no more pending async operations, it's time to call finish.
                finish(current._handler);
            }
        }

        public void asyncReadCallback(object state)
        {
            messageCallback(new ThreadPoolCurrent(this, (EventHandler)state, SocketOperation.Read));
        }

        public void asyncWriteCallback(object state)
        {
            messageCallback(new ThreadPoolCurrent(this, (EventHandler)state, SocketOperation.Write));
        }

        public void messageCallback(ThreadPoolCurrent current)
        {
            try
            {
                do
                {
                    current.completedSynchronously = false;
                    current._handler.message(ref current);
                }
                while(current.completedSynchronously);
            }
            catch(System.Exception ex)
            {
                string s = "exception in `" + _prefix + "':\n" + ex + "\nevent handler: " + current._handler.ToString();
                _instance.initializationData().logger.error(s);
            }
        }

        private AsyncCallback getCallback(int operation)
        {
            switch(operation)
            {
            case SocketOperation.Read:
                return asyncReadCallback;
            case SocketOperation.Write:
                return asyncWriteCallback;
            default:
                Debug.Assert(false);
                return null;
            }
        }

        private Instance _instance;
        private System.Action<System.Action, Ice.Connection> _dispatcher;
        private bool _destroyed;
        private readonly string _prefix;
        private readonly string _threadPrefix;

        private sealed class WorkerThread
        {
            private ThreadPool _threadPool;
            private Ice.Instrumentation.ThreadObserver _observer;
            private Ice.Instrumentation.ThreadState _state;

            internal WorkerThread(ThreadPool threadPool, string name) : base()
            {
                _threadPool = threadPool;
                _name = name;
                _state = Ice.Instrumentation.ThreadState.ThreadStateIdle;
                updateObserver();
            }

            public void updateObserver()
            {
                // Must be called with the thread pool mutex locked
                Ice.Instrumentation.CommunicatorObserver obsv = _threadPool._instance.initializationData().observer;
                if(obsv != null)
                {
                    _observer = obsv.getThreadObserver(_threadPool._prefix, _name, _state, _observer);
                    if(_observer != null)
                    {
                        _observer.attach();
                    }
                }
            }

            public void setState(Ice.Instrumentation.ThreadState s)
            {
                // Must be called with the thread pool mutex locked
                if(_observer != null)
                {
                    if(_state != s)
                    {
                        _observer.stateChanged(_state, s);
                    }
                }
                _state = s;
            }

            public Thread getThread()
            {
                return _thread;
            }

            public void join()
            {
                _thread.Join();
            }

            public void start(ThreadPriority priority)
            {
                if(_threadPool._stackSize == 0)
                {
                    _thread = new Thread(new ThreadStart(Run));
                }
                else
                {
                    _thread = new Thread(new ThreadStart(Run), _threadPool._stackSize);
                }
                _thread.IsBackground = true;
                _thread.Name = _name;
                _thread.Priority = priority;
                _thread.Start();
            }

            public void Run()
            {
                //
                // Set the default synchronization context to allow async/await to run
                // continuations on the thread pool.
                //
                SynchronizationContext.SetSynchronizationContext(new ThreadPoolSynchronizationContext(_threadPool));

                if(_threadPool._instance.initializationData().threadStart != null)
                {
                    try
                    {
                        _threadPool._instance.initializationData().threadStart();
                    }
                    catch(System.Exception ex)
                    {
                        string s = "thread hook start() method raised an unexpected exception in `";
                        s += _threadPool._prefix + "' thread " + _thread.Name + ":\n" + ex;
                        _threadPool._instance.initializationData().logger.error(s);
                    }
                }

                try
                {
                    _threadPool.run(this);
                }
                catch(System.Exception ex)
                {
                    string s = "exception in `" + _threadPool._prefix + "' thread " + _thread.Name + ":\n" + ex;
                    _threadPool._instance.initializationData().logger.error(s);
                }

                if(_observer != null)
                {
                    _observer.detach();
                }

                if(_threadPool._instance.initializationData().threadStop != null)
                {
                    try
                    {
                        _threadPool._instance.initializationData().threadStop();
                    }
                    catch(System.Exception ex)
                    {
                        string s = "thread hook stop() method raised an unexpected exception in `";
                        s += _threadPool._prefix + "' thread " + _thread.Name + ":\n" + ex;
                        _threadPool._instance.initializationData().logger.error(s);
                    }
                }
            }

            private readonly string _name;
            private Thread _thread;
        }

        private readonly int _size; // Number of threads that are pre-created.
        private readonly int _sizeMax; // Maximum number of threads.
        private readonly int _sizeWarn; // If _inUse reaches _sizeWarn, a "low on threads" warning will be printed.
        private readonly bool _serialize; // True if requests need to be serialized over the connection.
        private readonly ThreadPriority _priority;
        private readonly int _serverIdleTime;
        private readonly int _threadIdleTime;
        private readonly int _stackSize;

        private List<WorkerThread> _threads; // All threads, running or not.
        private int _threadIndex; // For assigning thread names.
        private int _inUse; // Number of threads that are currently in use.

        private Queue<ThreadPoolWorkItem> _workItems;
    }

}
