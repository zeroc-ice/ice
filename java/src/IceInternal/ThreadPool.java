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
        RemoveInfo info = new RemoveInfo(fd, callFinished);
        info.next = _removes;
        _removes = info;
        setInterrupt();
    }

    public void
    promoteFollower()
    {
//System.out.println("ThreadPool - promote follower - lock count = " + _threadMutex.count());
        _threadMutex.unlock();
    }

    public synchronized void
    initiateServerShutdown()
    {
//System.out.println("ThreadPool - initiate server shutdown");
        java.nio.ByteBuffer buf = java.nio.ByteBuffer.allocate(1);
        buf.put(0, (byte)1);
        try
        {
            int n = _fdIntrWrite.write(buf);
            assert(n == 1);
        }
        catch (java.io.IOException ex)
        {
        }
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
            _instance.logger().error("can't wait for graceful server termination in thread pool\n" +
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
            _instance.logger().error("can't wait for graceful application termination in thread pool\n" +
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
        _adds = null;
        _removes = null;
        _handlers = 0;
        _servers = 0;
        _timeout = 0;

        Network.SocketPair pair = Network.createPipe();
        _fdIntrRead = (java.nio.channels.ReadableByteChannel)pair.source;
        _fdIntrWrite = pair.sink;

        try
        {
            _selector = java.nio.channels.Selector.open();
            pair.source.configureBlocking(false);
            _fdIntrReadKey = pair.source.register(_selector, java.nio.channels.SelectionKey.OP_READ);
        }
        catch (java.io.IOException ex)
        {
            Ice.SystemException sys = new Ice.SystemException();
            sys.initCause(ex);
            throw sys;
        }

        //
        // The Selector holds a Set representing the selected keys. The
        // Set reference doesn't change, so we obtain it once here.
        //
        _keys = _selector.selectedKeys();

        //
        // On Win32 platforms, once a key has been selected, it will not
        // be reported again until the channel associated with that key
        // has been processed. This means that we must process all of
        // the keys in the selected-key set before calling select again.
        // If the iterator _keysIter is null, it indicates that select needs
        // to be called. Otherwise, we need to remove the next active key
        // (if any) from the iterator.
        //
        _keysIter = null;

        String value = _instance.properties().getProperty("Ice.ServerIdleTime");
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
        value = _instance.properties().getProperty("Ice.ThreadPool.MaxConnections");
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

    //
    // Called by Instance
    //
    synchronized void
    destroy()
    {
//System.out.println("ThreadPool - destroy");
        assert(!_destroyed);
        _destroyed = true;
        setInterrupt();
    }

    private boolean
    clearInterrupt()
    {
//System.out.println("clearInterrupt");
/*
try
{
    throw new RuntimeException();
}
catch (RuntimeException ex)
{
    ex.printStackTrace();
}
*/
        boolean shutdown = false;
        java.nio.ByteBuffer buf = java.nio.ByteBuffer.allocate(1);
        try
        {
            while (true)
            {
                buf.rewind();
                if (_fdIntrRead.read(buf) != 1)
                {
                    break;
                }

//System.out.println("  clearInterrupt - got byte " + (int)buf.get(0));
                if (buf.get(0) == (byte)1) // Shutdown initiated?
                {
                    shutdown = true;
                }
            }
        }
        catch (java.io.IOException ex)
        {
        }

        return shutdown;
    }

    private void
    setInterrupt()
    {
//System.out.println("setInterrupt");
/*
try
{
    throw new RuntimeException();
}
catch (RuntimeException ex)
{
    ex.printStackTrace();
}
//*/
        java.nio.ByteBuffer buf = java.nio.ByteBuffer.allocate(1);
        buf.put(0, (byte)0);
        try
        {
            int n = _fdIntrWrite.write(buf);
            assert(n == 1);
        }
        catch (java.io.IOException ex)
        {
        }
    }

    private void
    run()
    {
        boolean shutdown = false;
        final int timeoutMillis = _timeout * 1000;

        //
        // On Win32 platforms, select() occasionally returns 0 when it
        // is supposed to block indefinitely. As a workaround, we only
        // treat this occurrence as a timeout if we have a timeout value,
        // and if the proper amount of time has elapsed. This can be a
        // local variable because a timeout would not be split over
        // multiple threads.
        //
        long nextTimeout = 0;

        while (true)
        {
            _threadMutex.lock();
//System.out.println("ThreadPool - thread " + Thread.currentThread() + " has the lock");

        repeatSelect:

            while (true)
            {
                if (shutdown) // Shutdown has been initiated.
                {
//System.out.println("ThreadPool - shutdown");
                    shutdown = false;
                    _instance.objectAdapterFactory().shutdown();
                }

                if (_keysIter == null) // Need to select.
                {
                    int ret = 0;
                    try
                    {
//System.out.println("ThreadPool - selecting on " + _selector.keys().size() + " keys, thread id = " + Thread.currentThread());
                        if (timeoutMillis > 0 && nextTimeout == 0)
                        {
                            nextTimeout = System.currentTimeMillis() + timeoutMillis;
                        }

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

//System.out.println("ThreadPool - select() returned " + ret + ", _keys.size() = " + _keys.size());
                    if (ret == 0) // Potential timeout.
                    {
                        if (_timeout > 0)
                        {
                            long now = System.currentTimeMillis();
                            if (now >= nextTimeout) // Timeout.
                            {
//System.out.println("ThreadPool - timeout");
                                _timeout = 0;
                                shutdown = true;
                                nextTimeout = 0;
                            }
                        }
//System.out.println("ThreadPool - timeout workaround");
                        continue repeatSelect;
                    }

                    nextTimeout = 0;
                }
                else
                {
//System.out.println("ThreadPool - still have keys");
                }

                if (_keysIter == null)
                {
//System.out.println("ThreadPool - initializing _keysIter");
                    _keysIter = _keys.iterator();
                }

                EventHandler handler = null;

                synchronized(this)
                {
                    if (_destroyed)
                    {
//System.out.println("ThreadPool - destroyed, thread id = " + Thread.currentThread());
                        //
                        // Don't clear the interrupt fd if destroyed, so that
                        // the other threads exit as well.
                        //
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
                            if ((info.fd.validOps() & java.nio.channels.SelectionKey.OP_READ) > 0)
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
                                info.key = info.fd.register(_selector, op, info);
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
                        RemoveInfo info = _removes;
                        while (info != null)
                        {
                            java.nio.channels.SelectionKey key = info.fd.keyFor(_selector);
                            assert(key != null);
                            HandlerInfo hinfo = (HandlerInfo)key.attachment();
                            key.cancel();
                            if (info.callFinished) // Call finished() on the handler?
                            {
                                hinfo.handler.finished();
                            }
                            if (hinfo.handler.server())
                            {
                                --_servers;
                            }
                            _handlers--;
                            info = info.next;
//System.out.println("ThreadPool - _handlers = " + _handlers + ", _servers = " + _servers);
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

                    java.nio.channels.SelectionKey key = null;
                    while (_keysIter.hasNext())
                    {
                        //
                        // Ignore selection keys that have been cancelled
                        //
                        java.nio.channels.SelectionKey k = (java.nio.channels.SelectionKey)_keysIter.next();
                        _keysIter.remove();
                        if (k.isValid())
                        {
//System.out.println("ThreadPool - found a key");
                            key = k;
                            break;
                        }
                    }

                    if (!_keysIter.hasNext())
                    {
                        _keysIter = null;
//System.out.println("ThreadPool - reset iterator");
                    }

                    if (key == null)
                    {
//System.out.println("ThreadPool - didn't find a valid key");
                        continue repeatSelect;
                    }

                    if (key.channel() == _fdIntrRead)
                    {
//System.out.println("ThreadPool - input ready on the interrupt pipe");
                        shutdown = clearInterrupt();
                        continue repeatSelect;
                    }

                    HandlerInfo info = (HandlerInfo)key.attachment();
                    assert(info != null);
                    handler = info.handler;
                }

                //
                // If the handler is "readable", try to read a message.
                //
                // NOTE: On Win32 platforms, select may report a channel
                // as readable although nothing can be read.  We want to
                // ignore the event in this case.
                //
                BasicStream stream = new BasicStream(_instance);
                try
                {
                    if (handler.readable())
                    {
                        try
                        {
                            if (!read(handler)) // No data available.
                            {
//System.out.println("ThreadPool - no input");
                                continue repeatSelect;
                            }
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
                finally
                {
                    stream.destroy();
                }

                break;
            }
        }
    }

    private boolean
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
            //
            // On Win32 platforms, the selector may select a channel for
            // reading even though no data is available. Therefore, we
            // first try to read non-blocking; if we don't get any
            // data, we ignore the read event.
            //
            boolean doRead = handler.tryRead(stream);
            if (stream.pos() == 0)
            {
                return false;
            }
            if (doRead)
            {
                handler.read(stream);
            }
            assert(stream.pos() == stream.size());
        }

        int pos = stream.pos();
        assert(pos >= Protocol.headerSize);
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
            assert(stream.pos() == stream.size());
        }

        return true;
    }

    private static final class HandlerInfo
    {
        java.nio.channels.SelectableChannel fd;
        EventHandler handler;
        java.nio.channels.SelectionKey key;
        HandlerInfo next;

        HandlerInfo(java.nio.channels.SelectableChannel fd, EventHandler handler)
        {
            this.fd = fd;
            this.handler = handler;
        }
    }

    private static final class RemoveInfo
    {
        java.nio.channels.SelectableChannel fd;
        boolean callFinished;
        RemoveInfo next;

        RemoveInfo(java.nio.channels.SelectableChannel fd, boolean callFinished)
        {
            this.fd = fd;
            this.callFinished = callFinished;
        }
    }

    private Instance _instance;
    private boolean _destroyed;
    private java.nio.channels.ReadableByteChannel _fdIntrRead;
    private java.nio.channels.SelectionKey _fdIntrReadKey;
    private java.nio.channels.WritableByteChannel _fdIntrWrite;
    private java.nio.channels.Selector _selector;
    private java.util.Set _keys;
    private java.util.Iterator _keysIter;
    private HandlerInfo _adds;
    private RemoveInfo _removes;
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
                String s = "unknown exception in thread pool:\n" + sw.toString();
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

//System.out.println("ThreadPool - run() terminated - promoting follower");
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
