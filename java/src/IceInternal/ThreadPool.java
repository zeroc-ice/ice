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
        HandlerInfo info = new HandlerInfo(fd, handler);
        info.next = _adds;
        _adds = info;
        setInterrupt();
    }

    public synchronized void
    unregister(java.nio.channels.SelectableChannel fd, boolean callFinished)
    {
        java.nio.channels.SelectionKey key = fd.keyFor(_selector);
        HandlerInfo info = (HandlerInfo)key.attachment();
        assert(info != null);
        info.callFinished = callFinished;
        info.next = _removes;
        _removes = info;
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
        setInterrupt(); // TODO: just use wakeup?
    }

    public synchronized void
    waitUntilServerFinished()
    {
        while (_servers != 0 && _threadNum != 0)
        {
            try
            {
                wait();
            }
            catch (InterruptedException ex)
            {
            }
        }

        if (_servers != 0)
        {
            _instance.logger().error("can't wait for graceful server " +
                                     "termination in thread pool\n" +
                                     "since all threads have vanished");
        }
    }

    public synchronized void
    waitUntilFinished()
    {
        while (_handlers != 0 && _threadNum != 0)
        {
            try
            {
                wait();
            }
            catch (InterruptedException ex)
            {
            }
        }

        if (_handlers != 0)
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
        _adds = null;
        _removes = null;
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

        try
        {
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

        repeatSelect:

            while (true)
            {
                if (shutdown) // Shutdown has been initiated.
                {
                    shutdown = false;
                    _instance.objectAdapterFactory().shutdown();
                }

                _selector.selectedKeys().clear();
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

                if (ret == 0 && !_interrupted) // Timeout.
                {
                    assert(_timeout > 0);
                    _timeout = 0;
                    shutdown = true;
                    continue repeatSelect;
                }

                EventHandler handler = null;

                synchronized(this)
                {
                    if (_destroyed)
                    {
                        //
                        // Don't clear the interrupt fd if destroyed, so that
                        // the other threads exit as well.
                        //
                        _selector.wakeup();
                        return;
                    }

                    if (_adds != null)
                    {
                        //
                        // New handlers have been added.
                        //
                        HandlerInfo info = _adds;
                        while (info != null)
                        {
                            int op;
                            if ((info.fd.validOps() &
                                java.nio.channels.SelectionKey.OP_READ) > 0)
                            {
                                op = java.nio.channels.SelectionKey.OP_READ;
                            }
                            else
                            {
                                op = java.nio.channels.SelectionKey.OP_ACCEPT;
                            }

                            _handlers++;
                            try
                            {
                                info.key =
                                    info.fd.register(_selector, op, info);
                            }
                            catch (java.nio.channels.ClosedChannelException ex)
                            {
                                assert(false);
                            }
                            HandlerInfo next = info.next;
                            info.next = null;
                            info = next;
                        }
                        _adds = null;
                    }

                    if (_removes != null)
                    {
                        //
                        // Handlers are permanently removed.
                        //
                        HandlerInfo info = _removes;
                        while (info != null)
                        {
                            info.key.cancel();
                            if (info.callFinished)
                            {
                                info.handler.finished();
                            }
                            if (info.handler.server())
                            {
                                --_servers;
                            }
                            _handlers--;
                            info = info.next;
                        }
                        _removes = null;

                        if (_handlers == 0 || _servers == 0)
                        {
                            notifyAll(); // For waitUntil...Finished() methods.
                        }

                        //
                        // Selected filedescriptors may have changed, I
                        // therefore need to repeat the select().
                        //
                        shutdown = clearInterrupt();
                        continue repeatSelect;
                    }

                    java.util.Set keys = _selector.selectedKeys();
                    if (keys.size() == 0)
                    {
                        shutdown = clearInterrupt();
                        continue repeatSelect;
                    }

                    java.util.Iterator i = keys.iterator();
                    while (i.hasNext())
                    {
                        java.nio.channels.SelectionKey key =
                            (java.nio.channels.SelectionKey)i.next();
                        //
                        // Ignore selection keys that have been
                        // cancelled
                        //
                        if (key.isValid())
                        {
                            HandlerInfo info =
                                (HandlerInfo)key.attachment();
                            assert(info != null);
                            handler = info.handler;
                            break;
                        }
                    }

                    if (handler == null)
                    {
                        continue repeatSelect;
                    }
                }

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

                break;
            }
        }
    }

    private void
    read(EventHandler handler)
    {
        BasicStream stream = handler._stream;

        if (stream.size() == 0)
        {
            stream.resize(Protocol.headerSize, true);
            stream.pos(0);
        }

        if (stream.pos() != stream.size())
        {
            handler.read(stream);
            assert(stream.pos() != stream.size());
        }

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
        if (size < Protocol.headerSize)
        {
            throw new Ice.IllegalMessageSizeException();
        }
        if (size > 1024 * 1024) // TODO: Configurable
        {
            throw new Ice.MemoryLimitException();
        }
        if (size > stream.size())
        {
            stream.resize(size, true);
        }
        stream.pos(pos);

        if (stream.pos() != stream.size())
        {
            handler.read(stream);
            assert(stream.pos() != stream.size());
        }
    }

    private static final class HandlerInfo
    {
        java.nio.channels.SelectableChannel fd;
        EventHandler handler;
        java.nio.channels.SelectionKey key;
        HandlerInfo next;
        boolean callFinished;

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
    private HandlerInfo _adds;
    private HandlerInfo _removes;
    private int _handlers;
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
                java.io.StringWriter sw = new java.io.StringWriter();
                java.io.PrintWriter pw = new java.io.PrintWriter(sw);
                ex.printStackTrace(pw);
                pw.flush();
                String s = "exception in thread pool:\n" + sw.toString();
                _pool._instance.logger().error(s);
            }
            catch (RuntimeException ex)
            {
                java.io.StringWriter sw = new java.io.StringWriter();
                java.io.PrintWriter pw = new java.io.PrintWriter(sw);
                ex.printStackTrace(pw);
                pw.flush();
                String s = "unknown exception in thread pool:\n" +
                    sw.toString();
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
