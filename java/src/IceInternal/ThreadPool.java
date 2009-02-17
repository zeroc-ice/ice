// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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

    public
    ThreadPool(Instance instance, String prefix, int timeout)
    {
        _instance = instance;
        _destroyed = false;
        _prefix = prefix;
        _timeout = timeout;
        _selector = new Selector(instance, timeout);
        _threadIndex = 0;
        _running = 0;
        _inUse = 0;
        _load = 1.0;
        _promote = true;
        _serialize = _instance.initializationData().properties.getPropertyAsInt(_prefix + ".Serialize") > 0;
        _warnUdp = _instance.initializationData().properties.getPropertyAsInt("Ice.Warn.Datagrams") > 0;

        String programName = _instance.initializationData().properties.getProperty("Ice.ProgramName");
        if(programName.length() > 0)
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
            String s = _prefix + ".Size < 1; Size adjusted to 1";
            _instance.initializationData().logger.warning(s);
            size = 1;
        }               

        int sizeMax = 
            _instance.initializationData().properties.getPropertyAsIntWithDefault(_prefix + ".SizeMax", size);
        if(sizeMax < size)
        {
            String s = _prefix + ".SizeMax < " + _prefix + ".Size; SizeMax adjusted to Size (" + size + ")";
            _instance.initializationData().logger.warning(s);
            sizeMax = size;
        }
                
        int sizeWarn = _instance.initializationData().properties.getPropertyAsIntWithDefault(
                                                                _prefix + ".SizeWarn", sizeMax * 80 / 100);
        if(sizeWarn > sizeMax)
        {
            String s = _prefix + ".SizeWarn > " + _prefix + ".SizeMax; adjusted SizeWarn to SizeMax (" + sizeMax + ")";
            _instance.initializationData().logger.warning(s);
            sizeWarn = sizeMax;
        }

        _size = size;
        _sizeMax = sizeMax;
        _sizeWarn = sizeWarn;
        
        try
        {
            _threads = new java.util.ArrayList<EventHandlerThread>();
            for(int i = 0; i < _size; i++)
            {
                EventHandlerThread thread = new EventHandlerThread(_programNamePrefix + _prefix + "-" +
                                                                   _threadIndex++);
                _threads.add(thread);
                thread.start();
                ++_running;
            }
        }
        catch(RuntimeException ex)
        {
            java.io.StringWriter sw = new java.io.StringWriter();
            java.io.PrintWriter pw = new java.io.PrintWriter(sw);
            ex.printStackTrace(pw);
            pw.flush();
            String s = "cannot create thread for `" + _prefix + "':\n" + sw.toString();
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
        IceUtilInternal.Assert.FinalizerAssert(_destroyed);
    }

    public synchronized void
    destroy()
    {
        if(TRACE_SHUTDOWN)
        {
            trace("destroy");
        }

        assert(!_destroyed);
        _destroyed = true;
        _selector.setInterrupt();
    }

    public synchronized void
    _register(EventHandler handler)
    {
        assert(!_destroyed);

        if(!handler._registered)
        {
            if(TRACE_REGISTRATION)
            {
                trace("adding handler of type " + handler.getClass().getName() + " for channel " + handler.fd());
            }

            if(!handler._serializing)
            {
                _selector.add(handler, SocketStatus.NeedRead);
            }
            handler._registered = true;
        }
    }

    public synchronized void
    unregister(EventHandler handler)
    {
        assert(!_destroyed);
        if(handler._registered)
        {
            if(TRACE_REGISTRATION)
            {
                trace("removing handler for channel " + handler.fd());
            }

            if(!handler._serializing)
            {
                _selector.remove(handler);
            }
            handler._registered = false;
        }
    }

    public synchronized void
    finish(EventHandler handler)
    {
        assert(!_destroyed);

        if(TRACE_REGISTRATION)
        {
            trace("finishing handler for channel " + handler.fd());
        }

        if(handler._registered)
        {
            if(!handler._serializing)
            {
                _selector.remove(handler);
            }
            handler._registered = false;
        }

        _finished.add(handler);
        _selector.setInterrupt();
    }            

    public synchronized void
    execute(ThreadPoolWorkItem workItem)
    {
        if(_destroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }
        _workItems.add(workItem);
        _selector.setInterrupt();
    }

    public void
    promoteFollower(EventHandler handler)
    {
        if(_sizeMax > 1)
        {
            synchronized(this)
            {
                if(_serialize && handler != null)
                {
                    handler._serializing = true;
                    if(handler._registered)
                    {
                        _selector.remove(handler); 
                    }
                }

                assert(!_promote);
                _promote = true;
                notify();
                    
                if(!_destroyed)
                {
                    assert(_inUse >= 0);
                    ++_inUse;
                    
                    if(_inUse == _sizeWarn)
                    {
                        String s = "thread pool `" + _prefix + "' is running low on threads\n"
                            + "Size=" + _size + ", " + "SizeMax=" + _sizeMax + ", " + "SizeWarn=" + _sizeWarn;
                        _instance.initializationData().logger.warning(s);
                    }
                    
                    assert(_inUse <= _running);
                    if(_inUse < _sizeMax && _inUse == _running)
                    {
                        try
                        {
                            EventHandlerThread thread = new EventHandlerThread(_programNamePrefix + _prefix + "-" +
                                                                               _threadIndex++);
                            _threads.add(thread);
                            thread.start();
                            ++_running;
                        }
                        catch(RuntimeException ex)
                        {
                            java.io.StringWriter sw = new java.io.StringWriter();
                            java.io.PrintWriter pw = new java.io.PrintWriter(sw);
                            ex.printStackTrace(pw);
                            pw.flush();
                            String s = "cannot create thread for `" + _prefix + "':\n" + sw.toString();
                            _instance.initializationData().logger.error(s);
                        }
                    }
                }
            }
        }
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
        java.util.Iterator<EventHandlerThread> i = _threads.iterator();
        while(i.hasNext())
        {
            EventHandlerThread thread = i.next();
            
            while(true)
            {
                try
                {
                    thread.join();
                    break;
                }
                catch(InterruptedException ex)
                {
                }
            }
        }

        //
        // Destroy the selector
        //
        _selector.destroy();
    }

    public String
    prefix()
    {
        return _prefix;
    }

    //
    // Each thread supplies a BasicStream, to avoid creating excessive
    // garbage (Java only).
    //
    private boolean
    run(BasicStream stream)
    {
        if(_sizeMax > 1)
        {
            synchronized(this)
            {
                while(!_promote)
                {
                    try
                    {
                        wait();
                    }
                    catch(InterruptedException ex)
                    {
                    }
                }
                
                _promote = false;
            }
            
            if(TRACE_THREAD)
            {
                trace("thread " + Thread.currentThread() + " has the lock");
            }
        }

        while(true)
        {
            try
            {
                _selector.select();
            }
            catch(java.io.IOException ex)
            {
                Ice.SocketException se = new Ice.SocketException();
                se.initCause(ex);
                //throw se;
                java.io.StringWriter sw = new java.io.StringWriter();
                java.io.PrintWriter pw = new java.io.PrintWriter(sw);
                se.printStackTrace(pw);
                pw.flush();
                String s = "exception in `" + _prefix + "':\n" + sw.toString();
                _instance.initializationData().logger.error(s);
                continue;
            }

            EventHandler handler = null;
            ThreadPoolWorkItem workItem = null;
            boolean finished = false;
            boolean shutdown = false;

            synchronized(this)
            {
                if(_selector.checkTimeout())
                {
                    assert(_timeout > 0);
                    shutdown = true;
                }
                else if(_selector.isInterrupted())
                {
                    if(_selector.processInterrupt())
                    {
                        continue;
                    }
                    
                    //
                    // There are three possiblities for an interrupt:
                    //
                    // 1. The thread pool has been destroyed.
                    //
                    // 2. An event handler is being finished.
                    //
                    // 3. A work item has been scheduled.
                    //
                    
                    if(!_finished.isEmpty())
                    {
                        _selector.clearInterrupt();
                        handler = _finished.removeFirst();
                        finished = true;
                    }
                    else if(!_workItems.isEmpty())
                    {
                        //
                        // Work items must be executed first even if the thread pool is destroyed.
                        //
                        _selector.clearInterrupt();
                        workItem = _workItems.removeFirst();
                    }
                    else if(_destroyed)
                    {
                        //
                        // Don't clear the interrupt if destroyed, so that the other threads exit as well.
                        //
                        return true;
                    }
                    else
                    {
                        assert(false);
                    }
                }
                else
                {
                    handler = (EventHandler)_selector.getNextSelected();
                    if(handler == null)
                    {
                        continue;
                    }
                }
            }

            //
            // Now we are outside the thread synchronization.
            //
            
            if(shutdown)
            {
                //
                // Initiate server shutdown.
                //
                ObjectAdapterFactory factory;
                try
                {
                    factory = _instance.objectAdapterFactory();
                }
                catch(Ice.CommunicatorDestroyedException e)
                {
                    continue;
                }

                promoteFollower(null);
                factory.shutdown();

                //
                // No "continue", because we want shutdown to be done in
                // its own thread from this pool. Therefore we called
                // promoteFollower().
                //
            }
            else if(workItem != null)
            {
                try
                {
                    workItem.execute(this);
                }
                catch(Ice.LocalException ex)
                {
                    java.io.StringWriter sw = new java.io.StringWriter();
                    java.io.PrintWriter pw = new java.io.PrintWriter(sw);
                    ex.printStackTrace(pw);
                    pw.flush();
                    String s = "exception in `" + _prefix + "' while calling execute():\n" + sw.toString();
                    _instance.initializationData().logger.error(s);
                }

                //
                // No "continue", because we want execute() to
                // be called in its own thread from this
                // pool. Note that this means that execute()
                // must call promoteFollower().
                //
            }
            else
            {
                assert(handler != null);

                if(finished)
                {
                    //
                    // Notify a handler about its removal from the thread pool.
                    //
                    try
                    {
                        handler.finished(this);
                    }
                    catch(Ice.LocalException ex)
                    {
                        java.io.StringWriter sw = new java.io.StringWriter();
                        java.io.PrintWriter pw = new java.io.PrintWriter(sw);
                        ex.printStackTrace(pw);
                        pw.flush();
                        String s = "exception in `" + _prefix + "' while calling finished():\n" +
                            sw.toString() + "\n" + handler.toString();
                        _instance.initializationData().logger.error(s);
                    }

                    //
                    // No "continue", because we want finished() to be
                    // called in its own thread from this pool. Note
                    // that this means that finished() must call
                    // promoteFollower().
                    //
                }
                else
                {
                    //
                    // If the handler is "readable", try to read a
                    // message.
                    //
                    try
                    {
                        if(handler.readable())
                        {
                            try
                            {
                                if(!read(handler))
                                {
                                    continue; // Can't read without blocking.
                                }

                                if(handler.hasMoreData())
                                {
                                    _selector.hasMoreData(handler);
                                }
                            }
                            catch(Ice.TimeoutException ex)
                            {
                                assert(false); // This shouldn't occur as we only perform non-blocking reads.
                                continue;
                            }
                            catch(Ice.DatagramLimitException ex) // Expected.
                            {
                                handler._stream.resize(0, true);
                                continue;
                            }
                            catch(Ice.SocketException ex)
                            {
                                if(TRACE_EXCEPTION)
                                {
                                    trace("informing handler (" + handler.getClass().getName() +
                                          ") about exception " + ex);
                                    ex.printStackTrace();
                                }
                                    
                                handler.exception(ex);
                                continue;
                            }
                            catch(Ice.LocalException ex)
                            {
                                if(handler.datagram())
                                {
                                    if(_instance.initializationData().properties.getPropertyAsInt(
                                                                                "Ice.Warn.Connections") > 0)
                                    {
                                        _instance.initializationData().logger.warning(
                                            "datagram connection exception:\n" + ex + "\n" + handler.toString());
                                    }
                                    handler._stream.resize(0, true);
                                }
                                else
                                {
                                    if(TRACE_EXCEPTION)
                                    {
                                        trace("informing handler (" + handler.getClass().getName() +
                                              ") about exception " + ex);
                                        ex.printStackTrace();
                                    }
                                    
                                    handler.exception(ex);
                                }
                                continue;
                            }
                                
                            stream.swap(handler._stream);
                            assert(stream.pos() == stream.size());
                        }
                            
                        //
                        // Provide a new message to the handler.
                        //
                        try
                        {
                            handler.message(stream, this);
                        }
                        catch(Ice.LocalException ex)
                        {
                            java.io.StringWriter sw = new java.io.StringWriter();
                            java.io.PrintWriter pw = new java.io.PrintWriter(sw);
                            ex.printStackTrace(pw);
                            pw.flush();
                            String s = "exception in `" + _prefix + "' while calling message():\n" +
                                sw.toString() + "\n" + handler.toString();
                            _instance.initializationData().logger.error(s);
                        }

                        //
                        // No "continue", because we want message() to
                        // be called in its own thread from this
                        // pool. Note that this means that message()
                        // must call promoteFollower().
                        //
                    }
                    finally
                    {
                        stream.reset();
                    }
                }
            }

            if(_sizeMax > 1)
            {
                synchronized(this)
                {
                    if(!_destroyed)
                    {
                        if(_serialize && handler != null && handler._serializing)
                        {
                            if(handler._registered)
                            {
                                _selector.add(handler, SocketStatus.NeedRead);
                            }
                            handler._serializing = false;
                        }


                        if(_size < _sizeMax) // Dynamic thread pool
                        {
                            //
                            // First we reap threads that have been
                            // destroyed before.
                            //
                            int sz = _threads.size();
                            assert(_running <= sz);
                            if(_running < sz)
                            {
                                java.util.Iterator<EventHandlerThread> i = _threads.iterator();
                                while(i.hasNext())
                                {
                                    EventHandlerThread thread = i.next();

                                    if(!thread.isAlive())
                                    {
                                        try
                                        {
                                            thread.join();
                                            i.remove();
                                        }
                                        catch(InterruptedException ex)
                                        {
                                        }
                                    }
                                }
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
                                final double loadFactor = 0.05; // TODO: Configurable?
                                final double oneMinusLoadFactor = 1 - loadFactor;
                                _load = _load * oneMinusLoadFactor + _inUse * loadFactor;
                            }

                            if(_running > _size)
                            {
                                int load = (int)(_load + 0.5);

                                //
                                // We add one to the load factor because one
                                // additional thread is needed for select().
                                //
                                if(load + 1 < _running)
                                {
                                    assert(_inUse > 0);
                                    --_inUse;
                                
                                    assert(_running > 0);
                                    --_running;
                                
                                    return false;
                                }
                            }
                        }

                        assert(_inUse > 0);
                        --_inUse;
                    }

                    //
                    // Do not wait to be promoted again to release these objects.
                    //
                    handler = null;
                    workItem = null;

                    while(!_promote)
                    {
                        try
                        {
                            wait();
                        }
                        catch(InterruptedException ex)
                        {
                        }
                    }
                    
                    _promote = false;
                }
                
                if(TRACE_THREAD)
                {
                    trace("thread " + Thread.currentThread() + " has the lock");
                }
            }
        }
    }

    private boolean
    read(EventHandler handler)
    {
        BasicStream stream = handler._stream;

        if(stream.pos() >= Protocol.headerSize)
        {
            if(!handler.read(stream))
            {
                return false;
            }
            assert(stream.pos() == stream.size());
            return true;
        }

        if(stream.size() == 0)
        {
            stream.resize(Protocol.headerSize, true);
            stream.pos(0);
        }

        if(stream.pos() != stream.size())
        {
            if(!handler.read(stream))
            {
                return false;
            }
            assert(stream.pos() == stream.size());
        }

        int pos = stream.pos();
        if(pos < Protocol.headerSize)
        {
            //
            // This situation is possible for small UDP packets.
            //
            throw new Ice.IllegalMessageSizeException();
        }
        stream.pos(0);
        byte[] m = new byte[4];
        m[0] = stream.readByte();
        m[1] = stream.readByte();
        m[2] = stream.readByte();
        m[3] = stream.readByte();
        if(m[0] != Protocol.magic[0] || m[1] != Protocol.magic[1]
           || m[2] != Protocol.magic[2] || m[3] != Protocol.magic[3])
        {
            Ice.BadMagicException ex = new Ice.BadMagicException();
            ex.badMagic = m;
            throw ex;
        }

        byte pMajor = stream.readByte();
        byte pMinor = stream.readByte();
        if(pMajor != Protocol.protocolMajor || pMinor > Protocol.protocolMinor)
        {
            Ice.UnsupportedProtocolException e = new Ice.UnsupportedProtocolException();
            e.badMajor = pMajor < 0 ? pMajor + 255 : pMajor;
            e.badMinor = pMinor < 0 ? pMinor + 255 : pMinor;
            e.major = Protocol.protocolMajor;
            e.minor = Protocol.protocolMinor;
            throw e;
        }

        byte eMajor = stream.readByte();
        byte eMinor = stream.readByte();
        if(eMajor != Protocol.encodingMajor || eMinor > Protocol.encodingMinor)
        {
            Ice.UnsupportedEncodingException e = new Ice.UnsupportedEncodingException();
            e.badMajor = eMajor < 0 ? eMajor + 255 : eMajor;
            e.badMinor = eMinor < 0 ? eMinor + 255 : eMinor;
            e.major = Protocol.encodingMajor;
            e.minor = Protocol.encodingMinor;
            throw e;
        }

        stream.readByte(); // messageType
        stream.readByte(); // compress
        int size = stream.readInt();
        if(size < Protocol.headerSize)
        {
            throw new Ice.IllegalMessageSizeException();
        }
        if(size > _instance.messageSizeMax())
        {
            throw new Ice.MemoryLimitException();
        }
        if(size > stream.size())
        {
            stream.resize(size, true);
        }
        stream.pos(pos);

        if(stream.pos() != stream.size())
        {
            if(handler.datagram())
            {
                if(_warnUdp)
                {
                    _instance.initializationData().logger.warning("DatagramLimitException: maximum size of "
                                                                  + stream.pos() + " exceeded");
                }
                throw new Ice.DatagramLimitException();
            }
            else
            {
                if(!handler.read(stream))
                {
                    return false;
                }
                assert(stream.pos() == stream.size());
            }
        }

        return true;
    }

/*
 *  Commented out because it is unused.
 *
    private void
    selectNonBlocking()
    {
        while(true)
        {
            try
            {
                if(TRACE_SELECT)
                {
                    trace("non-blocking select on " + _selector.keys().size() + " keys, thread id = " +
                          Thread.currentThread());
                }

                _selector.selectNow();

                if(TRACE_SELECT)
                {
                    if(_keys.size() > 0)
                    {
                        trace("after selectNow, there are " + _keys.size() + " selected keys:");
                        java.util.Iterator<java.nio.channels.SelectionKey> i = _keys.iterator();
                        while(i.hasNext())
                        {
                            java.nio.channels.SelectionKey key = i.next();
                            trace("  " + keyToString(key));
                        }
                    }
                }

                break;
            }
            catch(java.io.InterruptedIOException ex)
            {
                continue;
            }
            catch(java.io.IOException ex)
            {
                //
                // Pressing Ctrl-C causes select() to raise an
                // IOException, which seems like a JDK bug. We trap
                // for that special case here and ignore it.
                // Hopefully we're not masking something important!
                //
                if(ex.getMessage().indexOf("Interrupted system call") != -1)
                {
                    continue;
                }

                Ice.SocketException se = new Ice.SocketException();
                se.initCause(ex);
                //throw se;
                java.io.StringWriter sw = new java.io.StringWriter();
                java.io.PrintWriter pw = new java.io.PrintWriter(sw);
                se.printStackTrace(pw);
                pw.flush();
                String s = "exception in `" + _prefix + "':\n" + sw.toString();
                _instance.initializationData().logger.error(s);
                continue;
            }
        }
    }
*/

    private void
    trace(String msg)
    {
        System.err.println(_prefix + ": " + msg);
    }

    private String
    keyToString(java.nio.channels.SelectionKey key)
    {
        String ops = "[";
        if(key.isAcceptable())
        {
            ops += " OP_ACCEPT";
        }
        if(key.isReadable())
        {
            ops += " OP_READ";
        }
        if(key.isConnectable())
        {
            ops += " OP_CONNECT";
        }
        if(key.isWritable())
        {
            ops += " OP_WRITE";
        }
        ops += " ]";
        return key.channel() + " " + ops;
    }

    private Instance _instance;
    private boolean _destroyed;
    private final String _prefix;
    private final String _programNamePrefix;
    private final Selector _selector;
    private java.util.LinkedList<ThreadPoolWorkItem> _workItems = new java.util.LinkedList<ThreadPoolWorkItem>();
    private java.util.LinkedList<EventHandler> _finished = new java.util.LinkedList<EventHandler>();
    private int _timeout;

    private final class EventHandlerThread extends Thread
    {
        EventHandlerThread(String name)
        {
            super(name);
        }

        public void
        run()
        {
            if(_instance.initializationData().threadHook != null)
            {
                _instance.initializationData().threadHook.start();
            }

            BasicStream stream = new BasicStream(_instance);

            boolean promote;

            try
            {
                promote = ThreadPool.this.run(stream);
            }
            catch(java.lang.Exception ex)
            {
                java.io.StringWriter sw = new java.io.StringWriter();
                java.io.PrintWriter pw = new java.io.PrintWriter(sw);
                ex.printStackTrace(pw);
                pw.flush();
                String s = "exception in `" + _prefix + "' thread " + getName() + ":\n" + sw.toString();
                _instance.initializationData().logger.error(s);
                promote = true;
            }

            if(promote && _sizeMax > 1)
            {
                //
                // Promote a follower, but w/o modifying _inUse or
                // creating new threads.
                //
                synchronized(ThreadPool.this)
                {
                    assert(!_promote);
                    _promote = true;
                    ThreadPool.this.notify();
                }
            }

            if(TRACE_THREAD)
            {
                trace("run() terminated");
            }

            if(_instance.initializationData().threadHook != null)
            {
                _instance.initializationData().threadHook.stop();
            }
        }
    }

    private final int _size; // Number of threads that are pre-created.
    private final int _sizeMax; // Maximum number of threads.
    private final int _sizeWarn; // If _inUse reaches _sizeWarn, a "low on threads" warning will be printed.
    private final boolean _serialize; // True if requests need to be serialized over the connection.

    private java.util.List<EventHandlerThread> _threads; // All threads, running or not.
    private int _threadIndex; // For assigning thread names.
    private int _running; // Number of running threads.
    private int _inUse; // Number of threads that are currently in use.
    private double _load; // Current load in number of threads.

    private boolean _promote;

    private final boolean _warnUdp;
}
