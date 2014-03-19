// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public final class ThreadPool
{
    final class ShutdownWorkItem implements ThreadPoolWorkItem
    {
        public void execute(ThreadPoolCurrent current)
        {
            current.ioCompleted();
            try
            {
                _instance.objectAdapterFactory().shutdown();
            }
            catch(Ice.CommunicatorDestroyedException ex)
            {
            }
        }
    }
    
    static final class FinishedWorkItem implements ThreadPoolWorkItem
    {
        public
        FinishedWorkItem(EventHandler handler)
        {
            _handler = handler;
        }

        public void execute(ThreadPoolCurrent current)
        {
            _handler.finished(current);
        }

        private final EventHandler _handler;
    }
    
    static final class JoinThreadWorkItem implements ThreadPoolWorkItem
    {
        public
        JoinThreadWorkItem(EventHandlerThread thread)
        {
            _thread = thread;
        }
        
        public void execute(ThreadPoolCurrent current)
        {
            // No call to ioCompleted, this shouldn't block (and we don't want to cause
            // a new thread to be started).
            _thread.join();
        }
        
        private final EventHandlerThread _thread;
    }
 
    //
    // Exception raised by the thread pool work queue when the thread pool
    // is destroyed.
    //
    static final class DestroyedException extends RuntimeException
    {
    }

    public
    ThreadPool(Instance instance, String prefix, int timeout)
    {
        _instance = instance;
        _destroyed = false;
        _prefix = prefix;
        _selector = new Selector(instance);
        _threadIndex = 0;
        _inUse = 0;
        _inUseIO = 0;
        _promote = true;
        _serialize = _instance.initializationData().properties.getPropertyAsInt(_prefix + ".Serialize") > 0;
        _serverIdleTime = timeout;
        
        Ice.Properties properties = _instance.initializationData().properties;

        String programName = properties.getProperty("Ice.ProgramName");
        if(programName.length() > 0)
        {
            _threadPrefix = programName + "-" + _prefix;
        }
        else
        {
            _threadPrefix = _prefix;
        }

        int nProcessors = Runtime.getRuntime().availableProcessors();

        //
        // We use just one thread as the default. This is the fastest
        // possible setting, still allows one level of nesting, and
        // doesn't require to make the servants thread safe.
        //
        int size = properties.getPropertyAsIntWithDefault(_prefix + ".Size", 1);
        if(size < 1)
        {
            String s = _prefix + ".Size < 1; Size adjusted to 1";
            _instance.initializationData().logger.warning(s);
            size = 1;
        }               

        int sizeMax = properties.getPropertyAsIntWithDefault(_prefix + ".SizeMax", size);
        if(sizeMax == -1)
        {
            sizeMax = nProcessors;
        }
        if(sizeMax < size)
        {
            String s = _prefix + ".SizeMax < " + _prefix + ".Size; SizeMax adjusted to Size (" + size + ")";
            _instance.initializationData().logger.warning(s);
            sizeMax = size;
        }
                
        int sizeWarn = properties.getPropertyAsInt(_prefix + ".SizeWarn");
        if(sizeWarn != 0 && sizeWarn < size)
        {
            String s = _prefix + ".SizeWarn < " + _prefix + ".Size; adjusted SizeWarn to Size (" + size + ")";
            _instance.initializationData().logger.warning(s);
            sizeWarn = size;
        }
        else if(sizeWarn > sizeMax)
        {
            String s = _prefix + ".SizeWarn > " + _prefix + ".SizeMax; adjusted SizeWarn to SizeMax (" + sizeMax + ")";
            _instance.initializationData().logger.warning(s);
            sizeWarn = sizeMax;
        }

        int threadIdleTime = properties.getPropertyAsIntWithDefault(_prefix + ".ThreadIdleTime", 60);
        if(threadIdleTime < 0)
        {
            String s = _prefix + ".ThreadIdleTime < 0; ThreadIdleTime adjusted to 0";
            _instance.initializationData().logger.warning(s);
            threadIdleTime = 0;
        }

        _size = size;
        _sizeMax = sizeMax;
        _sizeWarn = sizeWarn;
        _sizeIO = Math.min(sizeMax, nProcessors);
        _threadIdleTime = threadIdleTime;

        int stackSize = properties.getPropertyAsInt( _prefix + ".StackSize");
        if(stackSize < 0)
        {
            String s = _prefix + ".StackSize < 0; Size adjusted to JRE default";
            _instance.initializationData().logger.warning(s);
            stackSize = 0;
        }
        _stackSize = stackSize;

        boolean hasPriority = properties.getProperty(_prefix + ".ThreadPriority").length() > 0;
        int priority = properties.getPropertyAsInt(_prefix + ".ThreadPriority");
        if(!hasPriority)
        {
            hasPriority = properties.getProperty("Ice.ThreadPriority").length() > 0;
            priority = properties.getPropertyAsInt("Ice.ThreadPriority");
        }
        _hasPriority = hasPriority;
        _priority = priority;

        _workQueue = new ThreadPoolWorkQueue(this, _instance, _selector);

        _nextHandler = _handlers.iterator();
        
        if(_instance.traceLevels().threadPool >= 1)
        {
            String s = "creating " + _prefix + ": Size = " + _size + ", SizeMax = " + _sizeMax + ", SizeWarn = " +
                       _sizeWarn;
            _instance.initializationData().logger.trace(_instance.traceLevels().threadPoolCat, s);
        }
        
        try
        {
            for(int i = 0; i < _size; i++)
            {
                EventHandlerThread thread = new EventHandlerThread(_threadPrefix + "-" + _threadIndex++);
                _threads.add(thread);
                if(_hasPriority)
                {
                    thread.start(_priority);
                }
                else
                {
                    thread.start(java.lang.Thread.NORM_PRIORITY);
                }
            }
        }
        catch(RuntimeException ex)
        {
            String s = "cannot create thread for `" + _prefix + "':\n" + Ex.toString(ex);
            _instance.initializationData().logger.error(s);

            destroy();
            joinWithAllThreads();
            throw ex;
        }
    }

    protected synchronized void
    finalize()
        throws Throwable
    {
        try
        {
            IceUtilInternal.Assert.FinalizerAssert(_destroyed);
        }
        catch(java.lang.Exception ex)
        {
        }
        finally
        {
            super.finalize();
        }
    }

    public synchronized void
    destroy()
    {
        assert(!_destroyed);
        _destroyed = true;
        _workQueue.destroy();
    }

    public synchronized void
    updateObservers()
    {
        for(EventHandlerThread thread : _threads)
        {
            thread.updateObserver();
        }
    }

    public synchronized void
    initialize(EventHandler handler)
    {
        assert(!_destroyed);
        _selector.initialize(handler);
    }

    public void
    register(EventHandler handler, int op)
    {
        update(handler, SocketOperation.None, op);
    }

    public synchronized void
    update(EventHandler handler, int remove, int add)
    {
        assert(!_destroyed);
        _selector.update(handler, remove, add);
    }

    public void
    unregister(EventHandler handler, int op)
    {
        update(handler, op, SocketOperation.None);
    }

    public synchronized void
    finish(EventHandler handler)
    {
        assert(!_destroyed);
        _selector.finish(handler);
        _workQueue.queue(new FinishedWorkItem(handler));
    }

    public void
    execute(ThreadPoolWorkItem workItem)
    {
        _workQueue.queue(workItem);
    }

    public void
    joinWithAllThreads()
    {
        //
        // _threads is immutable after destroy() has been called,
        // therefore no synchronization is needed. (Synchronization
        // wouldn't be possible here anyway, because otherwise the
        // other threads would never terminate.)
        //
        for(EventHandlerThread thread : _threads)
        {
            thread.join();
        }

        //
        // Destroy the selector
        //
        _workQueue.close();
        _selector.destroy();
    }

    public String
    prefix()
    {
        return _prefix;
    }

    private void
    run(EventHandlerThread thread)
    {
        ThreadPoolCurrent current = new ThreadPoolCurrent(_instance, this, thread);
        boolean select = false;
        while(true)
        {
            if(current._handler != null)
            {
                try
                {
                    current._handler.message(current);
                }
                catch(DestroyedException ex)
                {
                    return;
                }
                catch(java.lang.Exception ex)
                {
                    String s = "exception in `" + _prefix + "':\n" + Ex.toString(ex);
                    s += "\nevent handler: " + current._handler.toString();
                    _instance.initializationData().logger.error(s);
                }
            }
            else if(select)
            {
                try
                {
                    _selector.select(_serverIdleTime);
                }
                catch(Selector.TimeoutException ex)
                {
                    synchronized(this)
                    {
                        if(!_destroyed && _inUse == 0)
                        {
                            _workQueue.queue(new ShutdownWorkItem()); // Select timed-out.
                        }
                        continue;
                    }
                }
            }

            synchronized(this)
            {
                if(current._handler == null)
                {
                    if(select)
                    {
                        _selector.finishSelect(_handlers, _serverIdleTime);
                        _nextHandler = _handlers.iterator();
                        select = false;
                    }
                    else if(!current._leader && followerWait(current))
                    {
                        return; // Wait timed-out.
                    }
                }
                else if(_sizeMax > 1)
                {
                    if(!current._ioCompleted)
                    {
                        //
                        // The handler didn't call ioCompleted() so we take care of decreasing 
                        // the IO thread count now.
                        //
                        --_inUseIO;

                        if((current.operation & SocketOperation.Read) != 0 && current._handler.hasMoreData())
                        {
                            _selector.hasMoreData(current._handler);
                        }
                    }
                    else
                    {
                        //
                        // If the handler called ioCompleted(), we re-enable the handler in
                        // case it was disabled and we decrease the number of thread in use.
                        //
                        _selector.enable(current._handler, current.operation);
                        assert(_inUse > 0);
                        --_inUse;
                    }

                    if(!current._leader && followerWait(current))
                    {
                        return; // Wait timed-out.
                    }
                }
                else if(!current._ioCompleted && 
                        (current.operation & SocketOperation.Read) != 0 && current._handler.hasMoreData())
                {
                    _selector.hasMoreData(current._handler);
                }

                //
                // Get the next ready handler.
                //
                if(_nextHandler.hasNext())
                {
                    current._ioCompleted = false;
                    current._handler = _nextHandler.next();
                    current.operation = current._handler._ready;
                    thread.setState(Ice.Instrumentation.ThreadState.ThreadStateInUseForIO);
                }
                else
                {
                    current._handler = null;
                }

                if(current._handler == null)
                {
                    //
                    // If there are no more ready handlers and there are still threads busy performing 
                    // IO, we give up leadership and promote another follower (which will perform the
                    // select() only once all the IOs are completed). Otherwise, if there's no more 
                    // threads peforming IOs, it's time to do another select().
                    //
                    if(_inUseIO > 0)
                    {
                        promoteFollower(current);
                    }
                    else
                    {
                        _selector.startSelect();
                        select = true;
                        thread.setState(Ice.Instrumentation.ThreadState.ThreadStateIdle);
                    }
                }
                else if(_sizeMax > 1)
                {
                    //
                    // Increment the IO thread count and if there's still threads available
                    // to perform IO and more handlers ready, we promote a follower.
                    //
                    ++_inUseIO;
                    if(_nextHandler.hasNext() && _inUseIO < _sizeIO)
                    {
                        promoteFollower(current);
                    }
                }
            }
        }
    }

    synchronized void
    ioCompleted(ThreadPoolCurrent current)
    {
        current._ioCompleted = true; // Set the IO completed flag to specify that ioCompleted() has been called.

        current._thread.setState(Ice.Instrumentation.ThreadState.ThreadStateInUseForUser);

        if(_sizeMax > 1)
        {
            --_inUseIO;
            
            if((current.operation & SocketOperation.Read) != 0 && current._handler.hasMoreData())
            {
                _selector.hasMoreData(current._handler);
            }
            
            if(_serialize && !_destroyed)
            {
                _selector.disable(current._handler, current.operation);
            }    
                
            if(current._leader)
            {
                //
                // If this thread is still the leader, it's time to promote a new leader.
                //
                promoteFollower(current);
            }
            else if(_promote && (_nextHandler.hasNext() || _inUseIO == 0))
            {
                notify();
            }

            assert(_inUse >= 0);
            ++_inUse;
                
            if(_inUse == _sizeWarn)
            {
                String s = "thread pool `" + _prefix + "' is running low on threads\n"
                    + "Size=" + _size + ", " + "SizeMax=" + _sizeMax + ", " + "SizeWarn=" + _sizeWarn;
                _instance.initializationData().logger.warning(s);
            }
                
            if(!_destroyed)
            {            
                assert(_inUse <= _threads.size());
                if(_inUse < _sizeMax && _inUse == _threads.size())
                {
                    if(_instance.traceLevels().threadPool >= 1)
                    {
                        String s = "growing " + _prefix + ": Size=" + (_threads.size() + 1);
                        _instance.initializationData().logger.trace(_instance.traceLevels().threadPoolCat, s);
                    }
                        
                    try
                    {
                        EventHandlerThread thread = new EventHandlerThread(_threadPrefix + "-" + _threadIndex++);
                        _threads.add(thread);
                        if(_hasPriority)
                        {
                            thread.start(_priority);
                        }
                        else
                        {
                            thread.start(java.lang.Thread.NORM_PRIORITY);
                        }
                    }
                    catch(RuntimeException ex)
                    {
                        String s = "cannot create thread for `" + _prefix + "':\n" + Ex.toString(ex);
                        _instance.initializationData().logger.error(s);
                    }
                }
            }
        }
        else if((current.operation & SocketOperation.Read) != 0 && current._handler.hasMoreData())
        {
            _selector.hasMoreData(current._handler);
        }
    }
    
    private synchronized void
    promoteFollower(ThreadPoolCurrent current)
    {
        assert(!_promote && current._leader);
        _promote = true;
        if(_inUseIO < _sizeIO && (_nextHandler.hasNext() || _inUseIO == 0))
        {
            notify();
        }
        current._leader = false;
    }

    private synchronized boolean
    followerWait(ThreadPoolCurrent current)
    {
        assert(!current._leader);

        current._thread.setState(Ice.Instrumentation.ThreadState.ThreadStateIdle);

        //
        // It's important to clear the handler before waiting to make sure that
        // resources for the handler are released now if it's finished. We also
        // clear the per-thread stream.
        //
        current._handler = null;
        current.stream.reset();
        
        //
        // Wait to be promoted and for all the IO threads to be done.
        //
        while(!_promote || _inUseIO == _sizeIO || !_nextHandler.hasNext() && _inUseIO > 0)
        {
            try
            {
                if(_threadIdleTime > 0)
                {
                    long before = IceInternal.Time.currentMonotonicTimeMillis();
                    wait(_threadIdleTime * 1000);
                    if(IceInternal.Time.currentMonotonicTimeMillis() - before >= _threadIdleTime * 1000)
                    {
                        if(!_destroyed && (!_promote || _inUseIO == _sizeIO ||
                                           (!_nextHandler.hasNext() && _inUseIO > 0)))
                        {
                            if(_instance.traceLevels().threadPool >= 1)
                            {
                                String s = "shrinking " + _prefix + ": Size=" + (_threads.size() - 1);
                                _instance.initializationData().logger.trace(_instance.traceLevels().threadPoolCat, s);
                            }
                            assert(_threads.size() > 1); // Can only be called by a waiting follower thread.
                            _threads.remove(current._thread);
                            _workQueue.queue(new JoinThreadWorkItem(current._thread));
                            return true;
                        }
                    }
                }
                else
                {
                    wait();
                }
            }
            catch(InterruptedException ex)
            {
            }
        }
        current._leader = true; // The current thread has become the leader.
        _promote = false;
        return false;
    }

    private final Instance _instance;
    private final ThreadPoolWorkQueue _workQueue;
    private boolean _destroyed;
    private final String _prefix;
    private final String _threadPrefix;
    private final Selector _selector;

    final class EventHandlerThread implements Runnable
    {
        EventHandlerThread(String name)
        {
            _name = name;
            _state = Ice.Instrumentation.ThreadState.ThreadStateIdle;
            updateObserver();
        }

        public void
        updateObserver()
        {
            // Must be called with the thread pool mutex locked
            Ice.Instrumentation.CommunicatorObserver obsv = _instance.getObserver();
            if(obsv != null)
            {
                _observer = obsv.getThreadObserver(_prefix, _name, _state, _observer);
                if(_observer != null)
                {
                    _observer.attach();
                }
            }
        }

        public void
        setState(Ice.Instrumentation.ThreadState s)
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

        public void
        join()
        {
            while(true)
            {
                try
                {
                    _thread.join();
                    break;
                }
                catch(InterruptedException ex)
                {
                }
            }
        }

        public void
        start(int priority)
        {
            _thread = new Thread(null, this, _name, _stackSize);
            _thread.setPriority(priority);
            _thread.start();
        }

        public void
        run()
        {
            if(_instance.initializationData().threadHook != null)
            {
                try
                {
                    _instance.initializationData().threadHook.start();
                }
                catch(java.lang.Exception ex)
                {
                    String s = "thread hook start() method raised an unexpected exception in `";
                    s += _prefix + "' thread " + _name + ":\n" + Ex.toString(ex);
                    _instance.initializationData().logger.error(s);
                }
            }

            try
            {
                ThreadPool.this.run(this);
            }
            catch(java.lang.Exception ex)
            {
                String s = "exception in `" + _prefix + "' thread " + _name + ":\n" + Ex.toString(ex);
                _instance.initializationData().logger.error(s);
            }

            if(_observer != null)
            {
                _observer.detach();
            }

            if(_instance.initializationData().threadHook != null)
            {
                try
                {
                    _instance.initializationData().threadHook.stop();
                }
                catch(java.lang.Exception ex)
                {
                    String s = "thread hook stop() method raised an unexpected exception in `";
                    s += _prefix + "' thread " + _name + ":\n" + Ex.toString(ex);
                    _instance.initializationData().logger.error(s);
                }
            }
        }

        final private String _name;
        private Thread _thread;
        private Ice.Instrumentation.ThreadState _state;
        private Ice.Instrumentation.ThreadObserver _observer;
    }

    private final int _size; // Number of threads that are pre-created.
    private final int _sizeIO; // Number of threads that can concurrently perform IO.
    private final int _sizeMax; // Maximum number of threads.
    private final int _sizeWarn; // If _inUse reaches _sizeWarn, a "low on threads" warning will be printed.
    private final boolean _serialize; // True if requests need to be serialized over the connection.
    private final int _priority;
    private final boolean _hasPriority;
    private final long _serverIdleTime;
    private final long _threadIdleTime;
    private final int _stackSize;

    private java.util.List<EventHandlerThread> _threads = new java.util.ArrayList<EventHandlerThread>();
    private int _threadIndex; // For assigning thread names.
    private int _inUse; // Number of threads that are currently in use.
    private int _inUseIO; // Number of threads that are currently performing IO.

    private java.util.List<EventHandler> _handlers = new java.util.ArrayList<EventHandler>();
    private java.util.Iterator<EventHandler> _nextHandler;

    private boolean _promote;
}
