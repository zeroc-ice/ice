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
    private final static boolean TRACE_REGISTRATION = false;
    private final static boolean TRACE_INTERRUPT = false;
    private final static boolean TRACE_SHUTDOWN = false;
    private final static boolean TRACE_SELECT = false;
    private final static boolean TRACE_EXCEPTION = false;
    private final static boolean TRACE_THREAD = false;
    private final static boolean TRACE_STACK_TRACE = false;

    public synchronized void
    _register(java.nio.channels.SelectableChannel fd, EventHandler handler)
    {
        if (TRACE_REGISTRATION)
        {
            trace("adding handler of type " + handler.getClass().getName() + " for channel " + fd +
                  ", handler count = " + (_handlers + 1));
        }

        ++_handlers;
        _changes.add(new FdHandlerPair(fd, handler));
        setInterrupt(0);
    }

    public synchronized void
    unregister(java.nio.channels.SelectableChannel fd)
    {
        if (TRACE_REGISTRATION)
        {
            if (TRACE_STACK_TRACE)
            {
                java.io.StringWriter sw = new java.io.StringWriter();
                try
                {
                    throw new RuntimeException();
                }
                catch (RuntimeException ex)
                {
                    java.io.PrintWriter pw = new java.io.PrintWriter(sw);
                    ex.printStackTrace(pw);
                    pw.flush();
                }
                trace("removing handler for channel " + fd + "\n" + sw.toString());
            }
            else
            {
                trace("removing handler for channel " + fd);
            }
        }

        _changes.add(new FdHandlerPair(fd, null));
        setInterrupt(0);
    }

    public void
    promoteFollower()
    {
        if (_multipleThreads)
        {
            _threadMutex.unlock();
        }
    }

    public void
    initiateShutdown()
    {
        if (TRACE_SHUTDOWN)
        {
            trace("initiate server shutdown");
        }

        setInterrupt(1);
    }

    public synchronized void
    waitUntilFinished()
    {
        if (TRACE_SHUTDOWN)
        {
            trace("waiting until finished...");
        }

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
        else
        {
            assert(_handlerMap.isEmpty());
        }

        if (TRACE_SHUTDOWN)
        {
            trace("finished.");
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

    //
    // Only for use by Instance
    //
    ThreadPool(Instance instance, boolean server, String name)
    {
        _instance = instance;
        _destroyed = false;
        _handlers = 0;
        _timeout = 0;
        _multipleThreads = false;
        _name = name;

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

        if (server)
        {
            _timeout = _instance.properties().getPropertyAsInt("Ice.ServerIdleTime");
            _timeoutMillis = _timeout * 1000;
            _threadNum = _instance.properties().getPropertyAsIntWithDefault("Ice.ServerThreadPool.Size", 10);
        }
        else
        {
            _threadNum = _instance.properties().getPropertyAsIntWithDefault("Ice.ClientThreadPool.Size", 1);
        }

	if (_threadNum < 1)
	{
	    _threadNum = 1;
	}

        if (_threadNum > 1)
        {
            _multipleThreads = true;
        }

        try
        {
            _threads = new EventHandlerThread[_threadNum];
            for (int i = 0; i < _threadNum; i++)
            {
                _threads[i] = new EventHandlerThread(_name + "-" + i);
                _threads[i].start();
            }
        }
        catch (RuntimeException ex)
        {
	    java.io.StringWriter sw = new java.io.StringWriter();
	    java.io.PrintWriter pw = new java.io.PrintWriter(sw);
	    ex.printStackTrace(pw);
	    pw.flush();
	    String s = "cannot create threads for thread pool:\n" + sw.toString();
	    _instance.logger().error(s);

            destroy();
	    joinWithAllThreads();
            throw ex;
        }
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
        if (TRACE_SHUTDOWN)
        {
            trace("destroy");
        }

        assert(!_destroyed);
        _destroyed = true;
        setInterrupt(0);
    }

    private boolean
    clearInterrupt()
    {
        if (TRACE_INTERRUPT)
        {
            trace("clearInterrupt");
            if (TRACE_STACK_TRACE)
            {
                try
                {
                    throw new RuntimeException();
                }
                catch (RuntimeException ex)
                {
                    ex.printStackTrace();
                }
            }
        }

        byte b = 0;

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

                if (TRACE_INTERRUPT)
                {
                    trace("clearInterrupt got byte " + (int)buf.get(0));
                }

                b = buf.get(0);
                break;
            }
        }
        catch (java.io.IOException ex)
        {
            Ice.SocketException se = new Ice.SocketException();
            se.initCause(ex);
            throw se;
        }

        return b == (byte)1; // Return true if shutdown has been initiated.
    }

    private void
    setInterrupt(int b)
    {
        if (TRACE_INTERRUPT)
        {
            trace("setInterrupt(" + b + ")");
            if (TRACE_STACK_TRACE)
            {
                try
                {
                    throw new RuntimeException();
                }
                catch (RuntimeException ex)
                {
                    ex.printStackTrace();
                }
            }
        }

        java.nio.ByteBuffer buf = java.nio.ByteBuffer.allocate(1);
        buf.put(0, (byte)b);
        while (buf.hasRemaining())
        {
            try
            {
                _fdIntrWrite.write(buf);
            }
            catch (java.io.IOException ex)
            {
                Ice.SocketException se = new Ice.SocketException();
                se.initCause(ex);
                throw se;
            }
        }
    }

    //
    // Each thread supplies a BasicStream, to avoid creating excessive
    // garbage (Java only)
    //
    private void
    run(BasicStream stream)
    {
        boolean shutdown = false;

        while (true)
        {
            if (_multipleThreads)
            {
                _threadMutex.lock();

                if (TRACE_THREAD)
                {
                    trace("thread " + Thread.currentThread() + " has the lock");
                }
            }

        repeatSelect:

            while (true)
            {
                if (shutdown) // Shutdown has been initiated.
                {
                    if (TRACE_SHUTDOWN)
                    {
                        trace("shutdown detected");
                    }

                    shutdown = false;
                    ObjectAdapterFactory factory = _instance.objectAdapterFactory();
                    if (factory != null)
                    {
                        factory.shutdown();
                    }
                }

                if (TRACE_REGISTRATION)
                {
                    java.util.Set keys = _selector.keys();
                    trace("selecting on " + keys.size() + " channels:");
                    java.util.Iterator i = keys.iterator();
                    while (i.hasNext())
                    {
                        java.nio.channels.SelectionKey key = (java.nio.channels.SelectionKey)i.next();
                        trace("  " + key.channel());
                    }
                }

                //
                // The Selector implementation works differently on Unix and
                // Windows. On Unix, calling select() always returns the
                // current number of channels with pending events. However,
                // on Windows, select() returns the number of channels
                // that have *new* events pending since the last call to
                // select(). Once a channel has been reported as ready, the
                // Windows implementation will not report it again until the
                // channel has been processed (i.e., accepted or read).
                //
                // A future version of the JDK may correct this discrepancy.
                //
                // Meanwhile, it is the set of selected keys that we are most
                // interested in. We must process each of the channels in the
                // set, because those keys will not be reported again (on
                // Windows) until we've done that.
                //
                // First, we call selectNonBlocking(). This is necessary to
                // ensure that the selected key set is updated (i.e., new
                // channels added, closed channels removed, etc.). If no keys
                // are present in the key set, then we'll call select() to
                // block until a new event is ready.
                //
                selectNonBlocking();
                if (_keys.size() == 0)
                {
                    int ret = select();
                    if (ret == 0) // Timeout.
                    {
                        if (TRACE_SELECT)
                        {
                            trace("timeout");
                        }

                        _timeout = 0;
                        shutdown = true;
                    }
                }

                EventHandler handler = null;
                boolean finished = false;

                synchronized (this)
                {
                    if (_keys.contains(_fdIntrReadKey) && _fdIntrReadKey.isReadable())
                    {
                        if (TRACE_SELECT || TRACE_INTERRUPT)
                        {
                            trace("detected interrupt");
                        }

                        //
                        // There are three possibilities for an interrupt:
                        //
                        // - The thread pool has been destroyed.
                        //
                        // - Server shutdown has been initiated.
                        //
                        // - An event handler was registered or unregistered.
                        //

                        //
                        // Thread pool destroyed?
                        //
                        if (_destroyed)
                        {
                            if (TRACE_SHUTDOWN)
                            {
                                trace("destroyed, thread id = " + Thread.currentThread());
                            }

                            //
                            // Don't clear the interrupt fd if destroyed, so that
                            // the other threads exit as well.
                            //
                            return;
                        }

                        //
                        // Remove the interrupt channel from the selected key set.
                        //
                        _keys.remove(_fdIntrReadKey);

                        shutdown = clearInterrupt();

                        //
                        // Server shutdown?
                        //
                        if (shutdown)
                        {
                            continue repeatSelect;
                        }

                        //
                        // An event handler must have been registered or
                        // unregistered.
                        //
                        assert(!_changes.isEmpty());
                        FdHandlerPair change = (FdHandlerPair)_changes.removeFirst();

                        if (change.handler != null) // Addition if handler is set.
                        {
                            int op;
                            if ((change.fd.validOps() & java.nio.channels.SelectionKey.OP_READ) > 0)
                            {
                                op = java.nio.channels.SelectionKey.OP_READ;
                            }
                            else
                            {
                                op = java.nio.channels.SelectionKey.OP_ACCEPT;
                            }

                            java.nio.channels.SelectionKey key = null;
                            try
                            {
                                key = change.fd.register(_selector, op, change.handler);
                            }
                            catch (java.nio.channels.ClosedChannelException ex)
                            {
                                assert(false);
                            }
                            _handlerMap.put(change.fd, new HandlerKeyPair(change.handler, key));

                            if (TRACE_REGISTRATION)
                            {
                                trace("added handler (" + change.handler.getClass().getName() + ") for fd " +
                                      change.fd);
                            }

                            continue repeatSelect;
                        }
                        else // Removal if handler is not set.
                        {
                            HandlerKeyPair pair = (HandlerKeyPair)_handlerMap.remove(change.fd);
                            assert(pair != null);
                            handler = pair.handler;
                            finished = true;
                            pair.key.cancel();

                            if (TRACE_REGISTRATION)
                            {
                                trace("removed handler (" + handler.getClass().getName() + ") for fd " + change.fd);
                            }

                            // Don't goto repeatSelect; we have to call
                            // finished() on the event handler below, outside
                            // the thread synchronization.
                        }
                    }
                    else
                    {
                        java.nio.channels.SelectionKey key = null;
                        java.util.Iterator iter = _keys.iterator();
                        while (iter.hasNext())
                        {
                            //
                            // Ignore selection keys that have been cancelled
                            //
                            java.nio.channels.SelectionKey k = (java.nio.channels.SelectionKey)iter.next();
                            iter.remove();
                            if (k.isValid() && key != _fdIntrReadKey)
                            {
                                if (TRACE_SELECT)
                                {
                                    trace("found a key: " + keyToString(k));
                                }

                                key = k;
                                break;
                            }
                        }

                        if (key == null)
                        {
                            if (TRACE_SELECT)
                            {
                                trace("didn't find a valid key");
                            }

                            continue repeatSelect;
                        }

                        handler = (EventHandler)key.attachment();
                    }
                }

                assert(handler != null);

                if (finished)
                {
                    //
                    // Notify a handler about it's removal from the thread
                    // pool.
                    //
                    handler.finished(this);

                    synchronized (this)
                    {
                        assert(_handlers > 0);
                        if (--_handlers == 0)
                        {
                            notifyAll(); // For waitUntilFinished().
                        }
                    }
                }
                else
                {
                    //
                    // If the handler is "readable", try to read a message.
                    //
                    // NOTE: On Win32 platforms, select may report a channel
                    // as readable although nothing can be read.  We want to
                    // ignore the event in this case.
                    //
                    try
                    {
                        if (handler.readable())
                        {
                            try
                            {
                                if (!read(handler)) // No data available.
                                {
                                    if (TRACE_SELECT)
                                    {
                                        trace("no input");
                                    }

                                    continue repeatSelect;
                                }
                            }
                            catch (Ice.TimeoutException ex) // Expected
                            {
                                continue repeatSelect;
                            }
                            catch (Ice.LocalException ex)
                            {
                                if (TRACE_EXCEPTION)
                                {
                                    trace("informing handler (" + handler.getClass().getName() +
                                          ") about exception " + ex);
                                    ex.printStackTrace();
                                }

                                handler.exception(ex);
                                continue repeatSelect;
                            }

                            stream.swap(handler._stream);
                            assert(stream.pos() == stream.size());
                        }

                        handler.message(stream, this);
                    }
                    finally
                    {
                        stream.reset();
                    }
                }

                break; // inner while loop
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

    private void
    selectNonBlocking()
    {
        while (true)
        {
            try
            {
                if (TRACE_SELECT)
                {
                    trace("non-blocking select on " + _selector.keys().size() + " keys, thread id = " +
                          Thread.currentThread());
                }

                _selector.selectNow();

                if (TRACE_SELECT)
                {
                    if (_keys.size() > 0)
                    {
                        trace("after selectNow, there are " + _keys.size() + " selected keys:");
                        java.util.Iterator i = _keys.iterator();
                        while (i.hasNext())
                        {
                            java.nio.channels.SelectionKey key = (java.nio.channels.SelectionKey)i.next();
                            trace("  " + keyToString(key));
                        }
                    }
                }

                break;
            }
            catch (java.io.InterruptedIOException ex)
            {
                continue;
            }
            catch (java.io.IOException ex)
            {
                //
                // Pressing Ctrl-C causes select() to raise an
                // IOException, which seems like a JDK bug. We trap
                // for that special case here and ignore it.
                // Hopefully we're not masking something important!
                //
                if (ex.getMessage().indexOf("Interrupted system call") != -1)
                {
                    continue;
                }

                Ice.SocketException se = new Ice.SocketException();
                se.initCause(ex);
                throw se;
            }
        }
    }

    private int
    select()
    {
        int ret = 0;

        //
        // On Win32 platforms, select() occasionally returns 0 when it
        // is supposed to block indefinitely. As a workaround, we only
        // treat this occurrence as a timeout if we have a timeout value,
        // and if the proper amount of time has elapsed.
        //
        long nextTimeout = 0;

        int timeoutMillis = _timeoutMillis;

        while (true)
        {
            try
            {
                if (TRACE_SELECT)
                {
                    trace("select on " + _selector.keys().size() + " keys, thread id = " + Thread.currentThread());
                }

                if (_timeout > 0 && nextTimeout == 0)
                {
                    nextTimeout = System.currentTimeMillis() + _timeoutMillis;
                }

                ret = _selector.select(timeoutMillis);
            }
            catch (java.io.InterruptedIOException ex)
            {
                continue;
            }
            catch (java.io.IOException ex)
            {
                //
                // Pressing Ctrl-C causes select() to raise an
                // IOException, which seems like a JDK bug. We trap
                // for that special case here and ignore it.
                // Hopefully we're not masking something important!
                //
                if (ex.getMessage().indexOf("Interrupted system call") != -1)
                {
                    continue;
                }

                Ice.SocketException se = new Ice.SocketException();
                se.initCause(ex);
                throw se;
            }

            if (TRACE_SELECT)
            {
                trace("select() returned " + ret + ", _keys.size() = " + _keys.size());
            }

            if (ret == 0) // Potential timeout.
            {
                if (_timeout > 0)
                {
                    long now = System.currentTimeMillis();
                    if (now >= nextTimeout) // Timeout.
                    {
                        break;
                    }
                    timeoutMillis -= (nextTimeout - now);

                    if (TRACE_SELECT)
                    {
                        trace("timeout workaround");
                    }
                }

                continue;
            }

            break;
        }

        return ret;
    }

    private void
    trace(String msg)
    {
        System.err.println(_name + ": " + msg);
    }

    private String
    keyToString(java.nio.channels.SelectionKey key)
    {
        String ops = "[";
        if (key.isAcceptable())
        {
            ops += " OP_ACCEPT";
        }
        if (key.isReadable())
        {
            ops += " OP_READ";
        }
        if (key.isConnectable())
        {
            ops += " OP_CONNECT";
        }
        if (key.isWritable())
        {
            ops += " OP_WRITE";
        }
        ops += " ]";
        return key.channel() + " " + ops;
    }

    private static final class FdHandlerPair
    {
        java.nio.channels.SelectableChannel fd;
        EventHandler handler;

        FdHandlerPair(java.nio.channels.SelectableChannel fd, EventHandler handler)
        {
            this.fd = fd;
            this.handler = handler;
        }
    }

    private static final class HandlerKeyPair
    {
        EventHandler handler;
        java.nio.channels.SelectionKey key;

        HandlerKeyPair(EventHandler handler, java.nio.channels.SelectionKey key)
        {
            this.handler = handler;
            this.key = key;
        }
    }

    private Instance _instance;
    private boolean _destroyed;
    private java.nio.channels.ReadableByteChannel _fdIntrRead;
    private java.nio.channels.SelectionKey _fdIntrReadKey;
    private java.nio.channels.WritableByteChannel _fdIntrWrite;
    private java.nio.channels.Selector _selector;
    private java.util.Set _keys;
    private java.util.LinkedList _changes = new java.util.LinkedList();
    private java.util.HashMap _handlerMap = new java.util.HashMap();
    private int _handlers;
    private int _timeout;
    private int _timeoutMillis;
    private RecursiveMutex _threadMutex = new RecursiveMutex();
    private boolean _multipleThreads;
    private String _name;

    private final class EventHandlerThread extends Thread
    {
        EventHandlerThread(String name)
        {
            super(name);
        }

        public void
        run()
        {
            BasicStream stream = new BasicStream(_instance);

            try
            {
		ThreadPool.this.run(stream);
            }
            catch (Ice.LocalException ex)
            {
                java.io.StringWriter sw = new java.io.StringWriter();
                java.io.PrintWriter pw = new java.io.PrintWriter(sw);
                ex.printStackTrace(pw);
                pw.flush();
                String s = "exception in thread pool:\n" + sw.toString();
                _instance.logger().error(s);
            }
            catch (RuntimeException ex)
            {
                java.io.StringWriter sw = new java.io.StringWriter();
                java.io.PrintWriter pw = new java.io.PrintWriter(sw);
                ex.printStackTrace(pw);
                pw.flush();
                String s = "unknown exception in thread pool thread " + getName() + ":\n" + sw.toString();
                _instance.logger().error(s);
            }

            synchronized(ThreadPool.this)
            {
                --_threadNum;
                assert(_threadNum >= 0);

                //
                // The notifyAll() shouldn't be needed, *except* if one of the
                // threads exits because of an exception. (Which is an error
                // condition in Ice and if it happens needs to be debugged.)
                // However, I call notifyAll() anyway, in all cases, using a
                // "defensive" programming approach when it comes to
                // multithreading.
                //
                if (_threadNum == 0)
                {
                    ThreadPool.this.notifyAll(); // For waitUntil...Finished() methods.
                }
            }

            if (TRACE_THREAD)
            {
                trace("run() terminated - promoting follower");
            }

            promoteFollower();

            stream.destroy();
        }
    }
    private EventHandlerThread[] _threads;
    private int _threadNum; // Number of running threads
}
