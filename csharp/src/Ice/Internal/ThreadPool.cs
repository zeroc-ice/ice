// Copyright (c) ZeroC, Inc.

using System.Diagnostics;

namespace Ice.Internal;

public delegate void ThreadPoolWorkItem(ThreadPoolCurrent current);

public delegate void AsyncCallback(object state);

internal class ThreadPoolMessage : IDisposable
{
    public ThreadPoolMessage(ThreadPoolCurrent current, object mutex)
    {
        _current = current;
        _mutex = mutex;
        _finish = false;
        _finishWithIO = false;
    }

    public bool startIOScope()
    {
        // This must be called with the handler locked.
        _finishWithIO = _current.startMessage();
        return _finishWithIO;
    }

    public void finishIOScope()
    {
        if (_finishWithIO)
        {
            // This must be called with the handler locked.
            _current.finishMessage();
        }
    }

    public void ioCompleted()
    {
        //
        // Call finishMessage once IO is completed only if serialization is not enabled.
        // Otherwise, finishMessage will be called when the event handler is done with
        // the message (it will be called from Dispose below).
        //
        Debug.Assert(_finishWithIO);
        if (_current.ioCompleted())
        {
            _finishWithIO = false;
            _finish = true;
        }
    }

    public void Dispose()
    {
        if (_finish)
        {
            //
            // A ThreadPoolMessage instance must be created outside the synchronization of the event handler. We
            // need to lock the event handler here to call finishMessage.
            //
            lock (_mutex)
            {
                _current.finishMessage();
            }
        }
    }

    private ThreadPoolCurrent _current;
    private object _mutex;
    private bool _finish;
    private bool _finishWithIO;
}

public class ThreadPoolCurrent
{
    internal ThreadPoolCurrent(ThreadPool threadPool, ThreadPool.WorkerThread thread)
    {
        _threadPool = threadPool;
        _thread = thread;
    }

    public int operation;

    public bool ioCompleted()
    {
        return _threadPool.ioCompleted(this);
    }

    public bool startMessage()
    {
        return _threadPool.startMessage(this);
    }

    public void finishMessage()
    {
        _threadPool.finishMessage(this);
    }

    internal readonly ThreadPool _threadPool;
    internal readonly ThreadPool.WorkerThread _thread;
    internal bool _ioCompleted;
    internal EventHandler _handler;
}

public sealed class ThreadPool : System.Threading.Tasks.TaskScheduler
{
    public ThreadPool(Instance instance, string prefix, int timeout)
    {
        Ice.Properties properties = instance.initializationData().properties;

        _instance = instance;
        _executor = instance.initializationData().executor;
        _destroyed = false;
        _prefix = prefix;
        _threadIndex = 0;
        _inUse = 0;
        _serialize = properties.getPropertyAsInt(_prefix + ".Serialize") > 0;
        _serverIdleTime = timeout <= 0 ? Timeout.InfiniteTimeSpan : TimeSpan.FromSeconds(timeout);

        string programName = properties.getIceProperty("Ice.ProgramName");
        if (programName.Length > 0)
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
        if (size < 1)
        {
            string s = _prefix + ".Size < 1; Size adjusted to 1";
            _instance.initializationData().logger.warning(s);
            size = 1;
        }

        int sizeMax = properties.getPropertyAsIntWithDefault(_prefix + ".SizeMax", size);
        if (sizeMax < size)
        {
            string s = _prefix + ".SizeMax < " + _prefix + ".Size; SizeMax adjusted to Size (" + size + ")";
            _instance.initializationData().logger.warning(s);
            sizeMax = size;
        }

        int sizeWarn = properties.getPropertyAsInt(_prefix + ".SizeWarn");
        if (sizeWarn != 0 && sizeWarn < size)
        {
            string s = _prefix + ".SizeWarn < " + _prefix + ".Size; adjusted SizeWarn to Size (" + size + ")";
            _instance.initializationData().logger.warning(s);
            sizeWarn = size;
        }
        else if (sizeWarn > sizeMax)
        {
            string s = _prefix + ".SizeWarn > " + _prefix + ".SizeMax; adjusted SizeWarn to SizeMax ("
                + sizeMax + ")";
            _instance.initializationData().logger.warning(s);
            sizeWarn = sizeMax;
        }

        int threadIdleTime = properties.getPropertyAsIntWithDefault(_prefix + ".ThreadIdleTime", 60);
        if (threadIdleTime < 0)
        {
            string s = _prefix + ".ThreadIdleTime < 0; ThreadIdleTime adjusted to 0";
            _instance.initializationData().logger.warning(s);
            threadIdleTime = 0;
        }

        _size = size;
        _sizeMax = sizeMax;
        _sizeWarn = sizeWarn;
        _threadIdleTime = threadIdleTime <= 0 ? Timeout.InfiniteTimeSpan : TimeSpan.FromSeconds(threadIdleTime);

        int stackSize = properties.getPropertyAsInt(_prefix + ".StackSize");
        if (stackSize < 0)
        {
            string s = _prefix + ".StackSize < 0; Size adjusted to OS default";
            _instance.initializationData().logger.warning(s);
            stackSize = 0;
        }
        _stackSize = stackSize;

        _priority = properties.getProperty(_prefix + ".ThreadPriority").Length > 0 ?
            Util.stringToThreadPriority(properties.getProperty(_prefix + ".ThreadPriority")) :
            Util.stringToThreadPriority(properties.getIceProperty("Ice.ThreadPriority"));

        if (_instance.traceLevels().threadPool >= 1)
        {
            string s = "creating " + _prefix + ": Size = " + _size + ", SizeMax = " + _sizeMax + ", SizeWarn = " +
                       _sizeWarn;
            _instance.initializationData().logger.trace(_instance.traceLevels().threadPoolCat, s);
        }

        _workItems = new Queue<ThreadPoolWorkItem>();

        try
        {
            _threads = new List<WorkerThread>();
            for (int i = 0; i < _size; ++i)
            {
                var thread = new WorkerThread(this, _threadPrefix + "-" + _threadIndex++);
                thread.start(_priority);
                _threads.Add(thread);
            }
        }
        catch (System.Exception ex)
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
        lock (_mutex)
        {
            if (_destroyed)
            {
                return;
            }
            _destroyed = true;
            Monitor.PulseAll(_mutex);
        }
    }

    public void updateObservers()
    {
        lock (_mutex)
        {
            foreach (WorkerThread t in _threads)
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
        lock (_mutex)
        {
            Debug.Assert(!_destroyed);

            // Don't remove what needs to be added
            remove &= ~add;

            // Don't remove/add if already un-registered or registered
            remove &= handler._registered;
            add &= ~handler._registered;
            if (remove == add)
            {
                return;
            }

            handler._registered &= ~remove;
            handler._registered |= add;

            if ((add & SocketOperation.Read) != 0 && (handler._pending & SocketOperation.Read) == 0)
            {
                handler._pending |= SocketOperation.Read;
                queueReadyForIOHandler(handler, SocketOperation.Read);
            }
            else if ((add & SocketOperation.Write) != 0 && (handler._pending & SocketOperation.Write) == 0)
            {
                handler._pending |= SocketOperation.Write;
                queueReadyForIOHandler(handler, SocketOperation.Write);
            }
        }
    }

    public void unregister(EventHandler handler, int op)
    {
        update(handler, op, SocketOperation.None);
    }

    public void finish(EventHandler handler)
    {
        lock (_mutex)
        {
            Debug.Assert(!_destroyed);

            handler._registered = SocketOperation.None;

            //
            // If there are no pending asynchronous operations, we can call finish on the handler now.
            //
            if (handler._pending == 0)
            {
                _workItems.Enqueue(current =>
                    {
                        current.operation = SocketOperation.None;
                        current._handler = handler;
                        handler.finished(current);
                    });
                Monitor.Pulse(_mutex);
            }
            else
            {
                handler._finish = true;
            }
        }
    }

    public void executeFromThisThread(System.Action call, Ice.Connection connection)
    {
        if (_executor is not null)
        {
            try
            {
                _executor(call, connection);
            }
            catch (System.Exception ex)
            {
                if (_instance.initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 1)
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

    public void execute(Action workItem, Ice.Connection connection)
    {
        lock (_mutex)
        {
            if (_destroyed)
            {
                throw new Ice.CommunicatorDestroyedException();
            }
            _workItems.Enqueue(current =>
                {
                    current.ioCompleted();
                    executeFromThisThread(workItem, connection);
                });
            Monitor.Pulse(_mutex);
        }
    }

    public void joinWithAllThreads()
    {
        //
        // _threads is immutable after destroy() has been called, therefore no synchronization is needed.
        // (Synchronization wouldn't be possible here anyway, because otherwise the other threads would never
        // terminate.)
        //
        Debug.Assert(_destroyed);
        foreach (WorkerThread thread in _threads)
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
        execute(() => { TryExecuteTask(task); }, null);
    }

    protected sealed override bool TryExecuteTaskInline(System.Threading.Tasks.Task task, bool taskWasPreviouslyQueued)
    {
        if (!taskWasPreviouslyQueued)
        {
            executeFromThisThread(() => { TryExecuteTask(task); }, null);
            return true;
        }
        return false;
    }

    protected sealed override bool TryDequeue(System.Threading.Tasks.Task task)
    {
        return false;
    }

    protected sealed override IEnumerable<System.Threading.Tasks.Task> GetScheduledTasks()
    {
        return Array.Empty<System.Threading.Tasks.Task>();
    }

    private void queueReadyForIOHandler(EventHandler handler, int operation)
    {
        lock (_mutex)
        {
            Debug.Assert(!_destroyed);
            _workItems.Enqueue(current =>
                {
                    current._handler = handler;
                    current.operation = operation;
                    try
                    {
                        current._handler.message(current);
                    }
                    catch (System.Exception ex)
                    {
                        string s = "exception in `" + _prefix + "':\n" + ex + "\nevent handler: " +
                            current._handler.ToString();
                        _instance.initializationData().logger.error(s);
                    }
                });
            Monitor.Pulse(_mutex);
        }
    }

    private void run(WorkerThread thread)
    {
        var current = new ThreadPoolCurrent(this, thread);
        while (true)
        {
            ThreadPoolWorkItem workItem = null;
            lock (_mutex)
            {
                while (_workItems.Count == 0)
                {
                    if (_destroyed)
                    {
                        return;
                    }

                    if (_threadIdleTime != Timeout.InfiniteTimeSpan)
                    {
                        if (!Monitor.Wait(_mutex, _threadIdleTime) && _workItems.Count == 0) // If timeout
                        {
                            if (_destroyed)
                            {
                                return;
                            }
                            else if (_inUse < _threads.Count - 1)
                            {
                                if (_instance.traceLevels().threadPool >= 1)
                                {
                                    string s = "shrinking " + _prefix + ": Size=" + (_threads.Count - 1);
                                    _instance.initializationData().logger.trace(
                                        _instance.traceLevels().threadPoolCat, s);
                                }

                                _threads.Remove(thread);
                                _workItems.Enqueue(c =>
                                    {
                                        // No call to ioCompleted, this shouldn't block (and we don't want to cause
                                        // a new thread to be started).
                                        thread.join();
                                    });
                                Monitor.Pulse(_mutex);
                                return;
                            }
                            else if (_inUse > 0)
                            {
                                //
                                // If this is the last idle thread but there are still other threads
                                // busy dispatching, we go back waiting with _threadIdleTime. We only
                                // wait with _serverIdleTime when there's only one thread left.
                                //
                                continue;
                            }

                            Debug.Assert(_threads.Count == 1);
                            if (!Monitor.Wait(_mutex, _serverIdleTime) && !_destroyed)
                            {
                                _workItems.Enqueue(c =>
                                    {
                                        c.ioCompleted();
                                        try
                                        {
                                            _instance.objectAdapterFactory().shutdown();
                                        }
                                        catch (Ice.CommunicatorDestroyedException)
                                        {
                                        }
                                    });
                            }
                        }
                    }
                    else
                    {
                        Monitor.Wait(_mutex);
                    }
                }

                Debug.Assert(_workItems.Count > 0);
                workItem = _workItems.Dequeue();

                current._thread.setState(Ice.Instrumentation.ThreadState.ThreadStateInUseForIO);
                current._ioCompleted = false;
            }

            try
            {
                workItem(current);
            }
            catch (System.Exception ex)
            {
                string s = "exception in `" + _prefix + "' while calling on work item:\n" + ex + '\n';
                _instance.initializationData().logger.error(s);
            }

            lock (_mutex)
            {
                if (_sizeMax > 1 && current._ioCompleted)
                {
                    Debug.Assert(_inUse > 0);
                    --_inUse;
                }
                thread.setState(Ice.Instrumentation.ThreadState.ThreadStateIdle);
            }
        }
    }

    public bool ioCompleted(ThreadPoolCurrent current)
    {
        lock (_mutex)
        {
            current._ioCompleted = true; // Set the IO completed flag to specify that ioCompleted() has been called.

            current._thread.setState(Ice.Instrumentation.ThreadState.ThreadStateInUseForUser);

            if (_sizeMax > 1)
            {
                Debug.Assert(_inUse >= 0);
                ++_inUse;

                if (_sizeMax > 1 && _inUse == _sizeWarn)
                {
                    string s = "thread pool `" + _prefix + "' is running low on threads\n"
                        + "Size=" + _size + ", " + "SizeMax=" + _sizeMax + ", " + "SizeWarn=" + _sizeWarn;
                    _instance.initializationData().logger.warning(s);
                }

                if (!_destroyed && _inUse < _sizeMax && _inUse == _threads.Count)
                {
                    if (_instance.traceLevels().threadPool >= 1)
                    {
                        string s = "growing " + _prefix + ": Size = " + (_threads.Count + 1);
                        _instance.initializationData().logger.trace(_instance.traceLevels().threadPoolCat, s);
                    }

                    try
                    {
                        var t = new WorkerThread(this, _threadPrefix + "-" + _threadIndex++);
                        t.start(_priority);
                        _threads.Add(t);
                    }
                    catch (System.Exception ex)
                    {
                        string s = "cannot create thread for `" + _prefix + "':\n" + ex;
                        _instance.initializationData().logger.error(s);
                    }
                }
            }
        }
        return _serialize;
    }

    public bool startMessage(ThreadPoolCurrent current)
    {
        Debug.Assert((current._handler._pending & current.operation) != 0);

        if ((current._handler._started & current.operation) != 0)
        {
            Debug.Assert((current._handler._ready & current.operation) == 0);
            current._handler._ready |= current.operation;
            current._handler._started &= ~current.operation;
            if (!current._handler.finishAsync(current.operation)) // Returns false if the handler is finished.
            {
                current._handler._pending &= ~current.operation;
                if (current._handler._pending == 0 && current._handler._finish)
                {
                    finish(current._handler);
                }
                return false;
            }
        }
        else if ((current._handler._ready & current.operation) == 0 &&
                (current._handler._registered & current.operation) != 0)
        {
            Debug.Assert((current._handler._started & current.operation) == 0);
            if (!current._handler.startAsync(current.operation, getCallback(current.operation)))
            {
                current._handler._pending &= ~current.operation;
                if (current._handler._pending == 0 && current._handler._finish)
                {
                    finish(current._handler);
                }
                return false;
            }
            else
            {
                current._handler._started |= current.operation;
                return false;
            }
        }

        if ((current._handler._registered & current.operation) != 0)
        {
            Debug.Assert((current._handler._ready & current.operation) != 0);
            current._handler._ready &= ~current.operation;
            return true;
        }
        else
        {
            current._handler._pending &= ~current.operation;
            if (current._handler._pending == 0 && current._handler._finish)
            {
                finish(current._handler);
            }
            return false;
        }
    }

    public void finishMessage(ThreadPoolCurrent current)
    {
        if ((current._handler._registered & current.operation) != 0)
        {
            Debug.Assert((current._handler._ready & current.operation) == 0);
            if (!current._handler.startAsync(current.operation, getCallback(current.operation)))
            {
                current._handler._pending &= ~current.operation;
            }
            else
            {
                Debug.Assert((current._handler._pending & current.operation) != 0);
                current._handler._started |= current.operation;
            }
        }
        else
        {
            current._handler._pending &= ~current.operation;
        }

        if (current._handler._pending == 0 && current._handler._finish)
        {
            // There are no more pending async operations, it's time to call finish.
            finish(current._handler);
        }
    }

    private AsyncCallback getCallback(int operation)
    {
        Debug.Assert(operation == SocketOperation.Read || operation == SocketOperation.Write);
        return state => queueReadyForIOHandler((EventHandler)state, operation);
    }

    private Instance _instance;
    private System.Action<System.Action, Ice.Connection> _executor;
    private bool _destroyed;
    private readonly string _prefix;
    private readonly string _threadPrefix;

    private readonly object _mutex = new object();

    internal sealed class WorkerThread
    {
        private ThreadPool _threadPool;
        private Ice.Instrumentation.ThreadObserver _observer;
        private Ice.Instrumentation.ThreadState _state;

        internal WorkerThread(ThreadPool threadPool, string name)
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
            if (obsv is not null)
            {
                _observer = obsv.getThreadObserver(_threadPool._prefix, _name, _state, _observer);
                if (_observer is not null)
                {
                    _observer.attach();
                }
            }
        }

        public void setState(Ice.Instrumentation.ThreadState s)
        {
            // Must be called with the thread pool mutex locked
            if (_observer is not null)
            {
                if (_state != s)
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
            if (_threadPool._instance.initializationData().threadStart is not null)
            {
                try
                {
                    _threadPool._instance.initializationData().threadStart();
                }
                catch (System.Exception ex)
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
            catch (System.Exception ex)
            {
                string s = "exception in `" + _threadPool._prefix + "' thread " + _thread.Name + ":\n" + ex;
                _threadPool._instance.initializationData().logger.error(s);
            }

            if (_observer is not null)
            {
                _observer.detach();
            }

            if (_threadPool._instance.initializationData().threadStop is not null)
            {
                try
                {
                    _threadPool._instance.initializationData().threadStop();
                }
                catch (System.Exception ex)
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
    private readonly TimeSpan _serverIdleTime;
    private readonly TimeSpan _threadIdleTime;
    private readonly int _stackSize;

    private List<WorkerThread> _threads; // All threads, running or not.
    private int _threadIndex; // For assigning thread names.
    private int _inUse; // Number of threads that are currently in use.

    private Queue<ThreadPoolWorkItem> _workItems;
}
