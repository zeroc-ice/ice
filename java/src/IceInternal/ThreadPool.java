// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package IceInternal;

public final class ThreadPool
{
    public synchronized void
    _register(java.nio.channels.SelectableChannel fd, EventHandler handler)
    {
        if (handler.server())
        {
            ++_servers;
        }
        _adds.add(new HandlerInfo(fd, handler));
        setInterrupt();
    }

    public synchronized void
    unregister(java.nio.channels.SelectableChannel fd)
    {
        java.nio.channels.SelectionKey key = fd.keyFor(_selector);
        _removes.add(key);
        setInterrupt();
    }

    public void
    promoteFollower()
    {
        _threadMutex.unlock();
    }

    public synchronized void
    initiateServerShutdown()
    {
        assert(!_shutdown);
        _shutdown = true;
        setInterrupt();
    }

    public synchronized void
    waitUntilServerFinished()
    {
        while (_servers > 0 && _threadNum > 0)
        {
            try
            {
                wait();
            }
            catch (InterruptedException ex)
            {
            }
        }

        if (_servers > 0)
        {
            _instance.logger().error("can't wait for graceful server " +
                                     "termination in thread pool\n" +
                                     "since all threads have vanished");
        }
    }

    public synchronized void
    waitUntilFinished()
    {
        while (_handlers > 0 && _threadNum > 0)
        {
            try
            {
                wait();
            }
            catch (InterruptedException ex)
            {
            }
        }

        if (_handlers > 0)
        {
            _instance.logger().error("can't wait for graceful application " +
                                     "termination in thread pool\n" +
                                     "since all threads have vanished");
        }
    }

    public void
    joinWithAllThreads()
    {
        //
        // _threads is immutable after the initial creation in the
        // constructor, therefore no synchronization is
        // needed. (Synchronization wouldn't be possible here anyway,
        // because otherwise the other threads would never terminate.)
        //
        for (int i = 0; i < _threads.length; i++)
        {
            while (true)
            {
                try
                {
                    _threads[i].join();
                    break;
                }
                catch (InterruptedException ex)
                {
                }
            }
        }
    }

    public synchronized void
    setMaxConnections(int maxConnections)
    {
        if (maxConnections < _threadNum + 1 && maxConnections != 0)
        {
            _maxConnections = _threadNum + 1;
        }
        else
        {
            _maxConnections = maxConnections;
        }
    }

    public synchronized int
    getMaxConnections()
    {
        return _maxConnections;
    }

    //
    // Only for use by Instance
    //
    ThreadPool(Instance instance)
    {
        _instance = instance;
        _destroyed = false;
        _interrupted = false;
        _shutdown = false;
        _handlers = 0;
        _servers = 0;
        _timeout = 0;

        try
        {
            _selector = java.nio.channels.Selector.open();
        }
        catch (java.io.IOException ex)
        {
            Ice.SystemException sys = new Ice.SystemException();
            sys.initCause(ex);
            throw sys;
        }

        String value =
            _instance.properties().getProperty("Ice.ServerIdleTime");
        if (value != null)
        {
            try
            {
                _timeout = Integer.parseInt(value);
            }
            catch (NumberFormatException ex)
            {
                // TODO: Error?
            }
        }

        try
        {
            _threadNum = 10;
            value = _instance.properties().getProperty("Ice.ThreadPool.Size");
            if (value != null)
            {
                try
                {
                    _threadNum = Integer.parseInt(value);
                    if (_threadNum < 1)
                    {
                        _threadNum = 1;
                    }
                }
                catch (NumberFormatException ex)
                {
                    // TODO: Error?
                }
            }

            _threads = new EventHandlerThread[_threadNum];
            for (int i = 0; i < _threadNum; i++)
            {
                _threads[i] = new EventHandlerThread(this);
                _threads[i].start();
            }
        }
        catch (RuntimeException ex)
        {
            destroy();
            throw ex;
        }

        // Must be called after _threadNum is set
        int maxConnections = 0;
        value = _instance.properties().getProperty(
            "Ice.ThreadPool.MaxConnections");
        if (value != null)
        {
            try
            {
                maxConnections = Integer.parseInt(value);
            }
            catch (NumberFormatException ex)
            {
                // TODO: Error?
            }
        }
        setMaxConnections(maxConnections);
    }

    protected void
    finalize()
        throws Throwable
    {
        assert(_destroyed);
        if (_selector != null)
        {
            try
            {
                _selector.close();
            }
            catch (java.io.IOException ex)
            {
            }
        }

        super.finalize();
    }

    //
    // Called by Instance
    //
    synchronized void
    destroy()
    {
        assert(!_destroyed);
        _destroyed = true;
        setInterrupt();
    }

    private boolean
    clearInterrupt()
    {
        _interrupted = false;
        boolean shutdown = _shutdown;
        _shutdown = false;
        return shutdown;
    }

    private void
    setInterrupt()
    {
        _interrupted = true;
        _selector.wakeup(); // Causes select() to return immediately
    }

    private void
    run()
    {
        boolean shutdown = false;
        final int timeoutMillis = _timeout * 1000;

        while (true)
        {
            _threadMutex.lock();

            EventHandler handler = null;
            boolean reap = false;

        repeatSelect:

            while (true)
            {
                if (shutdown) // Shutdown has been initiated.
                {
                    shutdown = false;
                    _instance.objectAdapterFactory().shutdown();
                }

                int ret = 0;
                try
                {
                    ret = _selector.select(timeoutMillis);
                }
                catch (java.io.InterruptedIOException ex)
                {
                    continue repeatSelect;
                }
                catch (java.io.IOException ex)
                {
                    Ice.SocketException se = new Ice.SocketException();
                    se.initCause(ex);
                    throw se;
                }

                if (ret == 0) // Timeout.
                {
                    assert(_timeout > 0);
                    _timeout = 0;
                    shutdown = true;
                    continue repeatSelect;
                }

                synchronized(this)
                {
                    if (_destroyed)
                    {
                        //
                        // Don't clear the interrupt fd if destroyed, so that
                        // the other threads exit as well.
                        //
                        return;
                    }

                    boolean interrupt = _interrupted;
                    if (interrupt)
                    {
                        shutdown = clearInterrupt();
                    }

                    if (!_adds.isEmpty())
                    {
                        //
                        // New handlers have been added.
                        //
                        java.util.ListIterator p = _adds.listIterator();
                        while (p.hasNext())
                        {
                            HandlerInfo info = (HandlerInfo)p.next();
                            addHandler(info);
                            _handlers++;
                            try
                            {
                                info.fd.register(
                                    _selector,
                                    java.nio.channels.SelectionKey.OP_READ,
                                    info);
                            }
                            catch (java.nio.channels.ClosedChannelException ex)
                            {
                                assert(false);
                            }
                        }
                        _adds.clear();
                    }

                    if (!_removes.isEmpty())
                    {
                        //
                        // Handlers are permanently removed.
                        //
                        java.util.ListIterator p = _adds.listIterator();
                        while (p.hasNext())
                        {
                            java.nio.channels.SelectionKey key =
                                (java.nio.channels.SelectionKey)p.next();
                            key.cancel();
                            HandlerInfo info = (HandlerInfo)key.attachment();
                            assert(info != null);
                            info.handler.finished();
                            if (info.handler.server())
                            {
                                --_servers;
                            }

                            _handlers--;
                            removeHandler(info);
                        }
                        _removes.clear();

                        if (_handlers == 0 || _servers == 0)
                        {
                            notifyAll(); // For waitUntil...Finished() methods.
                        }
                    }

                    if (interrupt)
                    {
                        continue repeatSelect;
                    }

                    //
                    // Check if there are connections to reap.
                    //
                    reap = false;
                    if (_maxConnections > 0 && _handlers > _maxConnections)
                    {
                        HandlerInfo info = _reapListEnd;
                        while (info != null)
                        {
                            if (!info.reaped)
                            {
                                info.reaped = true;
                                handler = info.handler;
                                reap = true;
                                break;
                            }
                            info = info.prev;
                        }
                    }

                    if (!reap)
                    {
                        java.util.Set keys = _selector.selectedKeys();
                        java.util.Iterator i = keys.iterator();
                        assert(i.hasNext());
                        java.nio.channels.SelectionKey key =
                            (java.nio.channels.SelectionKey)i.next();
                        HandlerInfo info = (HandlerInfo)key.attachment();
                        i.remove();
                        assert(info != null);

                        //
                        // Make the fd for the handler the most recently used
                        // one by moving it to the beginning of the the reap
                        // list.
                        //
                        if (info != _reapList)
                        {
                            removeHandler(info);
                            addHandler(info);
                        }

                        handler = info.handler;
                    }
                }

                if (reap)
                {
                    //
                    // Reap the handler.
                    //
                    try
                    {
                        if (!handler.tryDestroy())
                        {
                            continue repeatSelect;
                        }
                    }
                    catch (Ice.LocalException ex)
                    {
                        // Ignore exceptions.
                    }
                }
                else
                {
                    //
                    // If the handler is "readable", try to read a message.
                    //
                    BasicStream stream = new BasicStream(_instance);
                    if (handler.readable())
                    {
                        try
                        {
                            read(handler);
                        }
                        catch (Ice.TimeoutException ex) // Expected
                        {
                            continue repeatSelect;
                        }
                        catch (Ice.LocalException ex)
                        {
                            handler.exception(ex);
                            continue repeatSelect;
                        }

                        stream.swap(handler._stream);
                        assert(stream.pos() == stream.size());
                    }

                    handler.message(stream);
                }

                break;
            }
        }
    }

    private void
    read(EventHandler handler)
    {
        BasicStream stream = handler._stream;

        if (stream.size() < Protocol.headerSize) // Read header?
        {
            if (stream.size() == 0)
            {
                stream.resize(Protocol.headerSize);
                stream.pos(0);
            }

            handler.read(stream);
            if (stream.pos() != stream.size())
            {
                return;
            }
        }

        if (stream.size() >= Protocol.headerSize) // Interpret header?
        {
            int pos = stream.pos();
            stream.pos(0);
            byte protVer = stream.readByte();
            if (protVer != Protocol.protocolVersion)
            {
                throw new Ice.UnsupportedProtocolException();
            }
            byte encVer = stream.readByte();
            if (encVer != Protocol.encodingVersion)
            {
                throw new Ice.UnsupportedEncodingException();
            }
            byte messageType = stream.readByte();
            int size = stream.readInt();
            if (size > 1024 * 1024) // TODO: Configurable
            {
                throw new Ice.MemoryLimitException();
            }
            stream.resize(size);
            stream.pos(pos);
        }

        if (stream.size() > Protocol.headerSize &&
            stream.pos() != stream.size())
        {
            handler.read(stream);
        }
    }

    private void
    addHandler(HandlerInfo info)
    {
        info.next = _reapList;
        info.prev = null;
        if (_reapList != null)
        {
            _reapList.prev = info;
        }
        else
        {
            _reapListEnd = info;
        }
        _reapList = info;
    }

    private void
    removeHandler(HandlerInfo info)
    {
        //
        // Remove from _reapList
        //
        if (info.prev == null)
        {
            _reapList = info.next;
        }
        else
        {
            info.prev.next = info.next;
        }
        if (info.next == null)
        {
            _reapListEnd = info.prev;
        }
    }

    private static final class HandlerInfo
    {
        java.nio.channels.SelectableChannel fd;
        EventHandler handler;
        HandlerInfo prev;
        HandlerInfo next;
        boolean reaped;

        HandlerInfo(java.nio.channels.SelectableChannel fd,
                    EventHandler handler)
        {
            this.fd = fd;
            this.handler = handler;
        }
    }

    private Instance _instance;
    private boolean _destroyed;
    private java.nio.channels.Selector _selector;
    private boolean _interrupted;
    private boolean _shutdown;
    private java.util.LinkedList _adds = new java.util.LinkedList();
    private java.util.LinkedList _removes = new java.util.LinkedList();
    private int _handlers;
    private HandlerInfo _reapList = null;
    private HandlerInfo _reapListEnd = null;
    private int _servers;
    private int _timeout;
    private RecursiveMutex _threadMutex = new RecursiveMutex();

    private final static class EventHandlerThread extends Thread
    {
        EventHandlerThread(ThreadPool pool)
        {
            _pool = pool;
        }

        public void
        run()
        {
            try
            {
                _pool.run();
            }
            catch (Ice.LocalException ex)
            {
                String s = "exception in thread pool:\n" + ex;
                // TODO: Stack trace?
                _pool._instance.logger().error(s);
            }
            catch (RuntimeException ex)
            {
                String s = "unknown exception in thread pool:\n" + ex;
                // TODO: Stack trace?
                _pool._instance.logger().error(s);
            }

            synchronized(_pool)
            {
                --_pool._threadNum;
                assert(_pool._threadNum >= 0);

                //
                // The notifyAll() shouldn't be needed, *except* if one of the
                // threads exits because of an exception. (Which is an error
                // condition in Ice and if it happens needs to be debugged.)
                // However, I call notifyAll() anyway, in all cases, using a
                // "defensive" programming approach when it comes to
                // multithreading.
                //
                if (_pool._threadNum == 0)
                {
                    _pool.notifyAll(); // For waitUntil...Finished() methods.
                }
            }

            _pool.promoteFollower();
            _pool = null; // Break cyclic dependency.
        }

        private ThreadPool _pool;
    }
    private EventHandlerThread[] _threads;
    private int _threadNum; // Number of running threads
    private int _maxConnections; // Maximum number of connections. If set to
                                 // zero, the number of connections is not
                                 // limited.
}
