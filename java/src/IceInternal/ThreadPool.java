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
        _adds.add(new FdHandlerPair(fd, handler));
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

    public void
    initiateServerShutdown()
    {
        // Can't use _fdIntrWriteBuf because it's not thread-safe
        final byte[] arr = { 1 };
        java.nio.ByteBuffer buf = java.nio.ByteBuffer.wrap(arr);
        try
        {
            _fdIntrWrite.write(buf);
        }
        catch (java.io.IOException ex)
        {
            Ice.SystemException sys = new Ice.SystemException();
            sys.initCause(ex);
            throw sys;
        }
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
        while (_handlerMap.size() > 0 && _threadNum > 0)
        {
            try
            {
                wait();
            }
            catcH (InterruptedException ex)
            {
            }
        }

        if (_handlerMap.size() > 0)
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
    void
    ThreadPool(Instance instance)
    {
        _instance = instance;
        _destroyed = false;
        _servers = 0;
        _timeout = 0;

        try
        {
            _selector = java.nio.channels.Selector.open();
            _pipe = java.nio.channels.Pipe.open();
            _fdIntrRead = _pipe.source();
            _fdIntrRead.configureBlocking(false);
            _fdIntrWrite = _pipe.sink();
            _fdIntrReadBuf = java.nio.ByteBuffer.allocateDirect(1);
            _fdIntrWriteBuf = java.nio.ByteBuffer.allocateDirect(1);
            _fdIntrRead.register(_selector,
                                 java.nio.channels.SelectionKey.OP_READ);
            _fdIntrReadKey = _fdIntrRead.keyFor(_selector);
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
        if (_fdIntrWrite != null)
        {
            try
            {
                _fdIntrWrite.close();
            }
            catch (java.io.IOException ex)
            {
            }
        }
        if (_fdIntrRead != null)
        {
            try
            {
                _fdIntrRead.close();
            }
            catch (java.io.IOException ex)
            {
            }
        }

        super.finalize();
    }

    private synchronized void
    destroy()
    {
        assert(!_destroyed);
        _destroyed = true;
        setInterrupt();
    }

    private boolean
    clearInterrupt()
    {
        try
        {
            boolean shutdown = false;
            _fdIntrReadBuf.rewind();
            while (_fdIntrRead.read(_fdIntrReadBuf) == 1)
            {
                if (_fdIntrReadBuf.get(0) == 1) // Shutdown initiated?
                {
                    shutdown = true;
                }
                _fdIntrReadBuf.rewind();
            }

            return shutdown;
        }
        catch (java.io.IOException ex)
        {
            Ice.SystemException sys = new Ice.SystemException();
            sys.initCause(ex);
            throw sys;
        }
    }

    private void
    setInterrupt()
    {
        _fdIntrWriteBuf.rewind();
        _fdIntrWriteBuf.put(0, (byte)0);
        try
        {
            _fdIntrWrite.write(_fdIntrReadBuf);
        }
        catch (java.io.IOException ex)
        {
            Ice.SystemException sys = new Ice.SystemException();
            sys.initCause(ex);
            throw sys;
        }
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

                    boolean interrupt = false;
                    if (_fdIntrReadKey.isReadable())
                    {
                        shutdown = clearInterrupt();
                        interrupt = true;
                    }

                    if (!_adds.isEmpty())
                    {
                        //
                        // New handlers have been added.
                        //
                        java.util.ListIterator p = _adds.listIterator();
                        while (p.hasNext())
                        {
                            FdHandlerPair pair = (FdHandlerPair)p.next();
                            _reapList.addFirst(pair.fd);
                            try
                            {
                                pair.fd.register(
                                    _selector,
                                    java.nio.channels.SelectionKey.OP_READ,
                                    pair.handler);
                            }
                            catch (java.io.ClosedChannelException ex)
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
                            handler = (EventHandler)key.attachment();
                            assert(handler != null);
                            handler.finished();
                            if (handler.server())
                            {
                                --_servers;
                            }
                            _reapList.remove(key.channel());
                        }
                        _removes.clear();

                        // TODO
                        if (_handlerMap.empty() || _servers == 0)
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
                    // TODO
                    if (_maxConnections > 0 &&
                        _handlerMap.size() > _maxConnections)
                    {
                        // TODO
                    }

                    if (!reap)
                    {
                        java.util.Set keys = _selector.selectedKeys();
                        java.util.Iterator i = keys.iterator();
                        while (handler == null && i.hasNext())
                        {
                            java.nio.channels.SelectionKey key =
                                (java.nio.channels.SelectionKey)i.next();
                            if (key != _fdIntrReadKey)
                            {
                                handler = (EventHandler)key.attachment();
                            }
                            i.remove();
                        }
                        assert(handler != null);

                        // TODO: Update _reapList
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
                    Ice.Stream stream = ...; // TODO
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

                        stream.swap(handler._stream); // TODO
                        //assert(stream.i == stream.b.end());
                    }

                    handler.message(stream);
                }

                break;
            }
        }
    }

    private EventHandler
    read()
    {
        // TODO
    }

    private static final class FdHandlerPair
    {
        java.nio.channels.SelectableChannel fd;
        EventHandler handler;

        FdHandlerPair(java.nio.channels.SelectableChannel fd,
                      EventHandler handler)
        {
            this.fd = fd;
            this.handler = handler;
        }
    }

    private Instance _instance;
    private boolean _destroyed;
    private java.nio.channels.Selector _selector;
    private java.nio.channels.Pipe _pipe;
    private java.nio.channels.Pipe.SourceChannel _fdIntrRead;
    private java.nio.channels.Pipe.SinkChannel _fdIntrWrite;
    private java.nio.ByteBuffer _fdIntrReadBuf;
    private java.nio.ByteBuffer _fdIntrWriteBuf;
    private java.nio.channels.SelectionKey _fdIntrReadKey;
    private java.util.LinkedList _adds = new java.util.LinkedList();
    private java.util.LinkedList _removes = new java.util.LinkedList();
    private java.util.LinkedList _reapList = new java.util.LinkedList();
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
