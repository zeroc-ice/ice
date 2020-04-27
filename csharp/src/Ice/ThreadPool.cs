//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;

namespace IceInternal
{
    public delegate void ThreadPoolWorkItem();
    public delegate void AsyncCallback(object state);

    //
    // Thread pool threads set a custom synchronization context to ensure that
    // continuations from awaited methods continue executing on the thread pool
    // and not on the thread that notifies the awaited task.
    //
    internal sealed class ThreadPoolSynchronizationContext : SynchronizationContext
    {
        public ThreadPoolSynchronizationContext(ThreadPool threadPool) => _threadPool = threadPool;

        public override void Post(SendOrPostCallback d, object? state)
        {
            // Dispatch the continuation on the thread pool if this isn't called
            // already from a thread pool thread.
            var ctx = Current as ThreadPoolSynchronizationContext;
            if (ctx != this)
            {
                _threadPool.Dispatch(() => d(state));
            }
            else
            {
                d(state);
            }
        }

        public override void Send(SendOrPostCallback d, object? state) =>
            throw new System.NotSupportedException("the thread pool doesn't support synchronous calls");

        private readonly ThreadPool _threadPool;
    }

    internal struct ThreadPoolMessage
    {
        public ThreadPoolMessage(object mutex)
        {
            _mutex = mutex;
            _finish = false;
            _finishWithIO = false;
        }

        public bool StartIOScope(ref ThreadPoolCurrent current)
        {
            // This must be called with the handler locked.
            _finishWithIO = current.StartMessage();
            return _finishWithIO;
        }

        public void FinishIOScope(ref ThreadPoolCurrent current)
        {
            if (_finishWithIO)
            {
                lock (_mutex)
                {
                    current.FinishMessage(true);
                }
            }
        }

        public void Completed(ref ThreadPoolCurrent current)
        {
            //
            // Call FinishMessage once IO is completed only if serialization is not enabled.
            // Otherwise, FinishMessage will be called when the event handler is done with
            // the message (it will be called from destroy below).
            //
            Debug.Assert(_finishWithIO);
            if (current.IoCompleted())
            {
                _finishWithIO = false;
                _finish = true;
            }
        }

        public void Destroy(ref ThreadPoolCurrent current)
        {
            if (_finish)
            {
                //
                // A ThreadPoolMessage instance must be created outside the synchronization
                // of the event handler. We need to lock the event handler here to call
                // FinishMessage.
                //
                lock (_mutex)
                {
                    current.FinishMessage(false);
                    Debug.Assert(!current.CompletedSynchronously);
                }
            }
        }

        private readonly object _mutex;
        private bool _finish;
        private bool _finishWithIO;
    }

    public struct ThreadPoolCurrent
    {
        public ThreadPoolCurrent(ThreadPool threadPool, EventHandler handler, int op)
        {
            ThreadPool = threadPool;
            Handler = handler;
            Operation = op;
            CompletedSynchronously = false;
        }

        public readonly int Operation;
        public bool CompletedSynchronously;

        public bool IoCompleted() => ThreadPool.Serialize();

        public bool StartMessage() => ThreadPool.StartMessage(ref this);

        public void FinishMessage(bool fromIOThread) => ThreadPool.FinishMessage(ref this, fromIOThread);

        internal readonly ThreadPool ThreadPool;
        internal readonly EventHandler Handler;
    }

    public sealed class ThreadPool : TaskScheduler
    {
        public ThreadPool(Ice.Communicator communicator, string prefix, int timeout)
        {
            _communicator = communicator;
            _destroyed = false;
            _prefix = prefix;
            _threadIndex = 0;
            _inUse = 0;
            _serialize = _communicator.GetPropertyAsBool($"{_prefix}.Serialize") ?? false;
            _serverIdleTime = timeout;

            string? programName = _communicator.GetProperty("Ice.ProgramName");
            if (programName != null)
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
            int size = _communicator.GetPropertyAsInt($"{_prefix}.Size") ?? 1;
            if (size < 1)
            {
                string s = _prefix + ".Size < 1; Size adjusted to 1";
                _communicator.Logger.Warning(s);
                size = 1;
            }

            int sizeMax = _communicator.GetPropertyAsInt($"{_prefix}.SizeMax") ?? size;
            if (sizeMax < size)
            {
                _communicator.Logger.Warning($"{_prefix}.SizeMax < {_prefix}.Size; SizeMax adjusted to Size ({size})");
                sizeMax = size;
            }

            int sizeWarn = _communicator.GetPropertyAsInt($"{_prefix}.SizeWarn") ?? 0;
            if (sizeWarn != 0 && sizeWarn < size)
            {
                _communicator.Logger.Warning(
                    $"{_prefix}.SizeWarn < {_prefix}.Size; adjusted SizeWarn to Size ({size})");
                sizeWarn = size;
            }
            else if (sizeWarn > sizeMax)
            {
                _communicator.Logger.Warning(
                    "${_prefix}.SizeWarn > {_prefix}.SizeMax; adjusted SizeWarn to SizeMax ({sizeMax})");
                sizeWarn = sizeMax;
            }

            int threadIdleTime = _communicator.GetPropertyAsInt($"{_prefix}.ThreadIdleTime") ?? 60;
            if (threadIdleTime < 0)
            {
                _communicator.Logger.Warning($"{_prefix}.ThreadIdleTime < 0; ThreadIdleTime adjusted to 0");
                threadIdleTime = 0;
            }

            _size = size;
            _sizeMax = sizeMax;
            _sizeWarn = sizeWarn;
            _threadIdleTime = threadIdleTime;

            int stackSize = communicator.GetPropertyAsInt($"{_prefix }.StackSize") ?? 0;
            if (stackSize < 0)
            {
                _communicator.Logger.Warning($"{_prefix}.StackSize < 0; Size adjusted to OS default");
                stackSize = 0;
            }
            _stackSize = stackSize;

            _priority = Util.StringToThreadPriority(_communicator.GetProperty($"{_prefix}.ThreadPriority") ??
                                                    _communicator.GetProperty("Ice.ThreadPriority"));

            if (_communicator.TraceLevels.ThreadPool >= 1)
            {
                _communicator.Logger.Trace(_communicator.TraceLevels.ThreadPoolCat,
                    $"creating {_prefix}: Size = {_size}, SizeMax = {_sizeMax}, SizeWarn = {_sizeWarn}");
            }

            _workItems = new Queue<ThreadPoolWorkItem>();

            try
            {
                _threads = new List<WorkerThread>();
                for (int i = 0; i < _size; ++i)
                {
                    var thread = new WorkerThread(this, _threadPrefix + "-" + _threadIndex++);
                    thread.Start(_priority);
                    _threads.Add(thread);
                }
            }
            catch (System.Exception ex)
            {
                string s = "cannot create thread for `" + _prefix + "':\n" + ex;
                _communicator.Logger.Error(s);

                Destroy();
                JoinWithAllThreads();
                throw;
            }
        }

        public void Destroy()
        {
            lock (this)
            {
                if (_destroyed)
                {
                    return;
                }
                _destroyed = true;
                Monitor.PulseAll(this);
            }
        }

        public void UpdateObservers()
        {
            lock (this)
            {
                foreach (WorkerThread t in _threads)
                {
                    t.UpdateObserver();
                }
            }
        }

        public void Initialize(EventHandler handler)
        {
            handler.Ready = 0;
            handler.Pending = 0;
            handler.Started = 0;
            handler.Finish = false;
            handler.Registered = 0;
        }

        public void Register(EventHandler handler, int op) => Update(handler, SocketOperation.None, op);

        public void Update(EventHandler handler, int remove, int add)
        {
            lock (this)
            {
                Debug.Assert(!_destroyed);

                // Don't remove what needs to be added
                remove &= ~add;

                // Don't remove/add if already un-registered or registered
                remove &= handler.Registered;
                add &= ~handler.Registered;
                if (remove == add)
                {
                    return;
                }

                handler.Registered &= ~remove;
                handler.Registered |= add;

                if ((add & SocketOperation.Read) != 0 && (handler.Pending & SocketOperation.Read) == 0)
                {
                    handler.Pending |= SocketOperation.Read;
                    ExecuteNonBlocking(() => MessageCallback(new ThreadPoolCurrent(this, handler, SocketOperation.Read)));
                }
                else if ((add & SocketOperation.Write) != 0 && (handler.Pending & SocketOperation.Write) == 0)
                {
                    handler.Pending |= SocketOperation.Write;
                    ExecuteNonBlocking(() => MessageCallback(new ThreadPoolCurrent(this, handler, SocketOperation.Write)));
                }
            }
        }

        public void Unregister(EventHandler handler, int op) => Update(handler, op, SocketOperation.None);

        public void Finish(EventHandler handler)
        {
            lock (this)
            {
                Debug.Assert(!_destroyed);

                handler.Registered = SocketOperation.None;

                //
                // If there are no pending asynchronous operations, we can call finish on the handler now.
                //
                if (handler.Pending == 0)
                {
                    ExecuteNonBlocking(() =>
                       {
                           var current = new ThreadPoolCurrent(this, handler, SocketOperation.None);
                           handler.Finished(ref current);
                       });
                }
                else
                {
                    handler.Finish = true;
                }
            }
        }

        public void Dispatch(System.Action call)
        {
            lock (this)
            {
                if (_destroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }

                _workItems.Enqueue(() => call());

                Monitor.Pulse(this);

                //
                // If this is a dynamic thread pool which can still grow and if all threads are
                // currently busy dispatching or about to dispatch, we spawn a new thread to
                // execute this new work item right away.
                //
                if (_threads.Count < _sizeMax &&
                   (_inUse + _workItems.Count) > _threads.Count &&
                   !_destroyed)
                {
                    if (_communicator.TraceLevels.ThreadPool >= 1)
                    {
                        _communicator.Logger.Trace(_communicator.TraceLevels.ThreadPoolCat,
                            $"growing {_prefix}: Size = {_threads.Count + 1}");
                    }

                    try
                    {
                        var t = new WorkerThread(this, _threadPrefix + "-" + _threadIndex++);
                        t.Start(_priority);
                        _threads.Add(t);
                    }
                    catch (System.Exception ex)
                    {
                        _communicator.Logger.Error($"cannot create thread for `{_prefix}':\n{ex}");
                    }
                }
            }
        }

        public void ExecuteNonBlocking(ThreadPoolWorkItem workItem)
        {
            lock (this)
            {
                Debug.Assert(!_destroyed);
                _communicator.AsyncIOThread().Queue(workItem);
            }
        }

        public void JoinWithAllThreads()
        {
            //
            // _threads is immutable after destroy() has been called,
            // therefore no synchronization is needed. (Synchronization
            // wouldn't be possible here anyway, because otherwise the
            // other threads would never terminate.)
            //
            Debug.Assert(_destroyed);
            foreach (WorkerThread thread in _threads)
            {
                thread.Join();
            }
        }

        public string Prefix() => _prefix;

        public bool Serialize() => _serialize;

        protected sealed override void QueueTask(Task task) =>
            Dispatch(() => TryExecuteTask(task));

        protected sealed override bool TryExecuteTaskInline(Task task, bool taskWasPreviouslyQueued)
        {
            if (!taskWasPreviouslyQueued)
            {
                TryExecuteTask(task);
                return true;
            }
            return false;
        }

        protected sealed override bool TryDequeue(Task task) => false;

        protected sealed override IEnumerable<Task> GetScheduledTasks() => System.Array.Empty<Task>();

        private void Run(WorkerThread thread)
        {
            ThreadPoolWorkItem? workItem = null;
            while (true)
            {
                lock (this)
                {
                    if (workItem != null)
                    {
                        Debug.Assert(_inUse > 0);
                        --_inUse;
                        if (_workItems.Count == 0)
                        {
                            thread.SetState(Ice.Instrumentation.ThreadState.ThreadStateIdle);
                        }
                    }

                    workItem = null;

                    while (_workItems.Count == 0)
                    {
                        if (_destroyed)
                        {
                            return;
                        }

                        if (_threadIdleTime > 0)
                        {
                            if (!Monitor.Wait(this, _threadIdleTime * 1000) && _workItems.Count == 0) // If timeout
                            {
                                if (_destroyed)
                                {
                                    return;
                                }
                                else if (_serverIdleTime == 0 || _threads.Count > 1)
                                {
                                    //
                                    // If not the last thread or if server idle time isn't configured,
                                    // we can exit. Unlike C++/Java, there's no need to have a thread
                                    // always spawned in the thread pool because all the IO is done
                                    // by the .NET thread pool threads. Instead, we'll just spawn a
                                    // new thread when needed (i.e.: when a new work item is queued).
                                    //
                                    if (_communicator.TraceLevels.ThreadPool >= 1)
                                    {
                                        _communicator.Logger.Trace(_communicator.TraceLevels.ThreadPoolCat,
                                            $"shrinking {_prefix}: Size={_threads.Count - 1}");
                                    }

                                    _threads.Remove(thread);
                                    _communicator.AsyncIOThread().Queue(() => thread.Join());
                                    return;
                                }
                                else
                                {
                                    Debug.Assert(_serverIdleTime > 0 && _inUse == 0 && _threads.Count == 1);
                                    if (!Monitor.Wait(this, _serverIdleTime * 1000) &&
                                       _workItems.Count == 0)
                                    {
                                        if (!_destroyed)
                                        {
                                            _workItems.Enqueue(() =>
                                               {
                                                   try
                                                   {
                                                       _communicator.Shutdown();
                                                   }
                                                   catch (Ice.CommunicatorDestroyedException)
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

                    thread.SetState(Ice.Instrumentation.ThreadState.ThreadStateInUseForUser);

                    if (_sizeMax > 1 && _inUse == _sizeWarn)
                    {
                        string s = "thread pool `" + _prefix + "' is running low on threads\n"
                            + "Size=" + _size + ", " + "SizeMax=" + _sizeMax + ", " + "SizeWarn=" + _sizeWarn;
                        _communicator.Logger.Warning(s);
                    }
                }

                try
                {
                    workItem();
                }
                catch (System.Exception ex)
                {
                    string s = "exception in `" + _prefix + "' while calling on work item:\n" + ex + '\n';
                    _communicator.Logger.Error(s);
                }
            }
        }

        public bool StartMessage(ref ThreadPoolCurrent current)
        {
            Debug.Assert((current.Handler.Pending & current.Operation) != 0);

            if ((current.Handler.Started & current.Operation) != 0)
            {
                Debug.Assert((current.Handler.Ready & current.Operation) == 0);
                current.Handler.Ready |= current.Operation;
                current.Handler.Started &= ~current.Operation;
                if (!current.Handler.FinishAsync(current.Operation)) // Returns false if the handler is finished.
                {
                    current.Handler.Pending &= ~current.Operation;
                    if (current.Handler.Pending == 0 && current.Handler.Finish)
                    {
                        Finish(current.Handler);
                    }
                    return false;
                }
            }
            else if ((current.Handler.Ready & current.Operation) == 0 &&
                    (current.Handler.Registered & current.Operation) != 0)
            {
                Debug.Assert((current.Handler.Started & current.Operation) == 0);
                bool completed = false;
                if (!current.Handler.StartAsync(current.Operation, GetCallback(current.Operation), ref completed))
                {
                    current.Handler.Pending &= ~current.Operation;
                    if (current.Handler.Pending == 0 && current.Handler.Finish)
                    {
                        Finish(current.Handler);
                    }
                    return false;
                }
                else
                {
                    current.CompletedSynchronously = completed;
                    current.Handler.Started |= current.Operation;
                    return false;
                }
            }

            if ((current.Handler.Registered & current.Operation) != 0)
            {
                Debug.Assert((current.Handler.Ready & current.Operation) != 0);
                current.Handler.Ready &= ~current.Operation;
                return true;
            }
            else
            {
                current.Handler.Pending &= ~current.Operation;
                if (current.Handler.Pending == 0 && current.Handler.Finish)
                {
                    Finish(current.Handler);
                }
                return false;
            }
        }

        public void FinishMessage(ref ThreadPoolCurrent current, bool fromIOThread)
        {
            if ((current.Handler.Registered & current.Operation) != 0)
            {
                if (fromIOThread)
                {
                    Debug.Assert((current.Handler.Ready & current.Operation) == 0);
                    bool completed = false;
                    if (!current.Handler.StartAsync(current.Operation, GetCallback(current.Operation), ref completed))
                    {
                        current.Handler.Pending &= ~current.Operation;
                    }
                    else
                    {
                        Debug.Assert((current.Handler.Pending & current.Operation) != 0);
                        current.CompletedSynchronously = completed;
                        current.Handler.Started |= current.Operation;
                    }
                }
                else
                {
                    ThreadPoolCurrent c = current;
                    ExecuteNonBlocking(() => MessageCallback(c));
                }
            }
            else
            {
                current.Handler.Pending &= ~current.Operation;
            }

            if (current.Handler.Pending == 0 && current.Handler.Finish)
            {
                // There are no more pending async operations, it's time to call finish.
                Finish(current.Handler);
            }
        }

        public void AsyncReadCallback(object state) =>
            MessageCallback(new ThreadPoolCurrent(this, (EventHandler)state, SocketOperation.Read));

        public void AsyncWriteCallback(object state) =>
            MessageCallback(new ThreadPoolCurrent(this, (EventHandler)state, SocketOperation.Write));

        public void MessageCallback(ThreadPoolCurrent current)
        {
            try
            {
                do
                {
                    current.CompletedSynchronously = false;
                    current.Handler.Message(ref current);
                }
                while (current.CompletedSynchronously);
            }
            catch (System.Exception ex)
            {
                _communicator.Logger.Error($"exception in `{_prefix}':\n{ex}\nevent handler:{current.Handler}");
            }
        }

        private AsyncCallback GetCallback(int operation)
        {
            switch (operation)
            {
                case SocketOperation.Read:
                    return AsyncReadCallback;
                case SocketOperation.Write:
                    return AsyncWriteCallback;
                default:
                    Debug.Assert(false);
                    return null;
            }
        }

        private readonly Ice.Communicator _communicator;

        private bool _destroyed;
        private readonly string _prefix;
        private readonly string _threadPrefix;

        private sealed class WorkerThread
        {
            private readonly ThreadPool _threadPool;
            private Ice.Instrumentation.IThreadObserver? _observer;
            private Ice.Instrumentation.ThreadState _state;

            internal WorkerThread(ThreadPool threadPool, string name) : base()
            {
                _threadPool = threadPool;
                _name = name;
                _state = Ice.Instrumentation.ThreadState.ThreadStateIdle;
                UpdateObserver();
            }

            public void UpdateObserver()
            {
                // Must be called with the thread pool mutex locked
                Ice.Instrumentation.ICommunicatorObserver? obsv = _threadPool._communicator.Observer;
                if (obsv != null)
                {
                    _observer = obsv.GetThreadObserver(_threadPool._prefix, _name, _state, _observer);
                    if (_observer != null)
                    {
                        _observer.Attach();
                    }
                }
            }

            public void SetState(Ice.Instrumentation.ThreadState s)
            {
                // Must be called with the thread pool mutex locked
                if (_observer != null)
                {
                    if (_state != s)
                    {
                        _observer.StateChanged(_state, s);
                    }
                }
                _state = s;
            }

            public Thread GetThread()
            {
                Debug.Assert(_thread != null);
                return _thread;
            }

            public void Join()
            {
                Debug.Assert(_thread != null);
                _thread.Join();
            }

            public void Start(ThreadPriority priority)
            {
                if (_threadPool._stackSize == 0)
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
                Debug.Assert(_threadPool != null);
                Debug.Assert(_thread != null);
                //
                // Set the default synchronization context to allow async/await to run
                // continuations on the thread pool.
                //
                SynchronizationContext.SetSynchronizationContext(new ThreadPoolSynchronizationContext(_threadPool));

                System.Action? threadStart = _threadPool._communicator.ThreadStart;
                if (threadStart != null)
                {
                    try
                    {
                        threadStart();
                    }
                    catch (System.Exception ex)
                    {
                        string s = "thread hook start() method raised an unexpected exception in `";
                        s += _threadPool._prefix + "' thread " + _thread.Name + ":\n" + ex;
                        _threadPool._communicator.Logger.Error(s);
                    }
                }

                try
                {
                    _threadPool.Run(this);
                }
                catch (System.Exception ex)
                {
                    string s = "exception in `" + _threadPool._prefix + "' thread " + _thread.Name + ":\n" + ex;
                    _threadPool._communicator.Logger.Error(s);
                }

                if (_observer != null)
                {
                    _observer.Detach();
                }

                System.Action? threadStop = _threadPool._communicator.ThreadStop;
                if (threadStop != null)
                {
                    try
                    {
                        threadStop();
                    }
                    catch (System.Exception ex)
                    {
                        string s = "thread hook stop() method raised an unexpected exception in `";
                        s += _threadPool._prefix + "' thread " + _thread.Name + ":\n" + ex;
                        _threadPool._communicator.Logger.Error(s);
                    }
                }
            }

            private readonly string _name;
            private Thread? _thread;
        }

        private readonly int _size; // Number of threads that are pre-created.
        private readonly int _sizeMax; // Maximum number of threads.
        private readonly int _sizeWarn; // If _inUse reaches _sizeWarn, a "low on threads" warning will be printed.
        private readonly bool _serialize; // True if requests need to be serialized over the connection.
        private readonly ThreadPriority _priority;
        private readonly int _serverIdleTime;
        private readonly int _threadIdleTime;
        private readonly int _stackSize;

        private readonly List<WorkerThread> _threads; // All threads, running or not.
        private int _threadIndex; // For assigning thread names.
        private int _inUse; // Number of threads that are currently in use.

        private readonly Queue<ThreadPoolWorkItem> _workItems;
    }
}
