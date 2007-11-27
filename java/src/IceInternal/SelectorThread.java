// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class SelectorThread
{
    public interface SocketReadyCallback
    {
        //
        // The selector thread unregisters the callback when socketReady returns SocketStatus.Finished.
        //
        SocketStatus socketReady(boolean finished);

        //
        // The selector thread doesn't unregister the callback when sockectTimeout is called; socketTimeout
        // must unregister the callback either explicitly with unregister() or by shutting down the socket 
        // (if necessary).
        //
        void socketTimeout();
    }

    SelectorThread(Instance instance)
    {
        _instance = instance;
        _destroyed = false;

        Network.SocketPair pair = Network.createPipe();
        _fdIntrRead = (java.nio.channels.ReadableByteChannel)pair.source;
        _fdIntrWrite = pair.sink;

        try
        {
            _selector = java.nio.channels.Selector.open();
            pair.source.configureBlocking(false);
            _fdIntrReadKey = pair.source.register(_selector, java.nio.channels.SelectionKey.OP_READ);
        }
        catch(java.io.IOException ex)
        {
            Ice.SyscallException sys = new Ice.SyscallException();
            sys.initCause(ex);
            throw sys;
        }

        //
        // The Selector holds a Set representing the selected keys. The
        // Set reference doesn't change, so we obtain it once here.
        //
        _keys = _selector.selectedKeys();

        _thread = new HelperThread();
        _thread.start();

        _timer = _instance.timer();
    }

    protected synchronized void
    finalize()
        throws Throwable
    {
        IceUtil.Assert.FinalizerAssert(_destroyed);
    }

    public synchronized void
    destroy()
    {
        assert(!_destroyed);
        _destroyed = true;
        setInterrupt();
    }

    public synchronized void
    _register(java.nio.channels.SelectableChannel fd, SocketReadyCallback cb, SocketStatus status, int timeout)
    {
        assert(!_destroyed); // The selector thread is destroyed after the incoming/outgoing connection factories.
        assert(status != SocketStatus.Finished);

        SocketInfo info = new SocketInfo(fd, cb, status, timeout);
        _changes.add(info);
        if(info.timeout >= 0)
        {
            _timer.schedule(info, info.timeout);
        }
        setInterrupt();
    }

    //
    // Unregister the given file descriptor. The registered callback will be notified with socketReady()
    // upon registration to allow some cleanup to be done.
    //
    public synchronized void
    unregister(java.nio.channels.SelectableChannel fd)
    {
        assert(!_destroyed); // The selector thread is destroyed after the incoming/outgoing connection factories.
        _changes.add(new SocketInfo(fd, null, SocketStatus.Finished, 0));
        setInterrupt();
    }

    public void
    joinWithThread()
    {
        if(_thread != null)
        {
            try
            {
                _thread.join();
            }
            catch(InterruptedException ex)
            {
            }
        }
    }

    private void
    clearInterrupt()
    {
        byte b = 0;

        java.nio.ByteBuffer buf = java.nio.ByteBuffer.allocate(1);
        try
        {
            while(true)
            {
                buf.rewind();
                if(_fdIntrRead.read(buf) != 1)
                {
                    break;
                }

                b = buf.get(0);
                break;
            }
        }
        catch(java.io.IOException ex)
        {
            Ice.SocketException se = new Ice.SocketException();
            se.initCause(ex);
            throw se;
        }
    }

    private void
    setInterrupt()
    {
        java.nio.ByteBuffer buf = java.nio.ByteBuffer.allocate(1);
        buf.put(0, (byte)0);
        while(buf.hasRemaining())
        {
            try
            {
                _fdIntrWrite.write(buf);
            }
            catch(java.io.IOException ex)
            {
                Ice.SocketException se = new Ice.SocketException();
                se.initCause(ex);
                throw se;
            }
        }
    }

    public void
    run()
    {
        java.util.HashMap socketMap = new java.util.HashMap();
        java.util.LinkedList readyList = new java.util.LinkedList();
        java.util.LinkedList finishedList = new java.util.LinkedList();
        while(true)
        {
            int ret = 0;

            while(true)
            {
                try
                {
                    ret = _selector.select();
                }
                catch(java.io.IOException ex)
                {
                    //
                    // Pressing Ctrl-C causes select() to raise an
                    // IOException, which seems like a JDK bug. We trap
                    // for that special case here and ignore it.
                    // Hopefully we're not masking something important!
                    //
                    if(Network.interrupted(ex))
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
                    String s = "exception in selector thread:\n" + sw.toString();
                    _instance.initializationData().logger.error(s);
                    continue;
                }

                break;
            }

            assert(readyList.isEmpty() && finishedList.isEmpty());
            
            if(_keys.contains(_fdIntrReadKey) && _fdIntrReadKey.isReadable())
            {
                synchronized(this)
                {
                    //
                    // There are two possiblities for an interrupt:
                    //
                    // 1. The selector thread has been destroyed.
                    // 2. A socket was registered or unregistered.
                    //

                    //
                    // Thread destroyed?
                    //
                    if(_destroyed)
                    {
                        break;
                    }

                    //
                    // Remove the interrupt channel from the selected key set.
                    //
                    _keys.remove(_fdIntrReadKey);

                    clearInterrupt();
                    SocketInfo info = (SocketInfo)_changes.removeFirst();
                    if(info.cb != null) // Registration
                    {
                        try
                        {
                            info.key = info.fd.register(_selector, convertStatus(info.status), info);
                        }
                        catch(java.nio.channels.ClosedChannelException ex)
                        {
                            assert(false);
                        }
                        assert(!socketMap.containsKey(info.fd));
                        socketMap.put(info.fd, info);
                    }
                    else // Unregistration
                    {
                        info = (SocketInfo)socketMap.get(info.fd);
                        if(info != null && info.status != SocketStatus.Finished)
                        {
                            if(info.timeout >= 0)
                            {
                                _timer.cancel(info);
                            }
                            
                            try
                            {
                                info.key.cancel();
                            }
                            catch(java.nio.channels.CancelledKeyException ex)
                            {
                                assert(false);
                            }
                            info.status = SocketStatus.Finished;
                            readyList.add(info);
                        }
                    }
                }
            }
            else
            {
                //
                // Examine the selection key set.
                //
                java.util.Iterator iter = _keys.iterator();
                while(iter.hasNext())
                {
                    //
                    // Ignore selection keys that have been cancelled or timed out.
                    //
                    java.nio.channels.SelectionKey key = (java.nio.channels.SelectionKey)iter.next();
                    iter.remove();
                    assert(key != _fdIntrReadKey);
                    SocketInfo info = (SocketInfo)key.attachment();
                    if(info.timeout >= 0)
                    {
                        _timer.cancel(info);
                    }
                    assert(key.isValid());
                    readyList.add(info);
                }
            }

            java.util.Iterator iter = readyList.iterator();
            while(iter.hasNext())
            {
                SocketInfo info = (SocketInfo)iter.next();
                SocketStatus status;
                try
                {
                    status = info.cb.socketReady(info.status == SocketStatus.Finished);
                }
                catch(Ice.LocalException ex)
                {
                    java.io.StringWriter sw = new java.io.StringWriter();
                    java.io.PrintWriter pw = new java.io.PrintWriter(sw);
                    ex.printStackTrace(pw);
                    pw.flush();
                    String s = "exception in selector thread " + _thread.getName() + 
                        " while calling socketReady():\n" + sw.toString();
                    _instance.initializationData().logger.error(s);
                    status = SocketStatus.Finished;
                }
                    
                if(status == SocketStatus.Finished)
                {
                    finishedList.add(info);
                }
                else
                {
                    assert(info.status != SocketStatus.Finished);
                    try
                    {
                        info.status = status;
                        info.key.interestOps(convertStatus(status));
                        if(info.timeout >= 0)
                        {
                            _timer.schedule(info, info.timeout);
                        }
                    }
                    catch(java.nio.channels.CancelledKeyException ex)
                    {
                        assert(false);
                    }
                }
            }
            readyList.clear();

            if(finishedList.isEmpty())
            {
                continue;
            }

            iter = finishedList.iterator();
            while(iter.hasNext())
            {
                SocketInfo info = (SocketInfo)iter.next();
                if(info.status != SocketStatus.Finished)
                {
                    try
                    {
                        info.key.cancel();
                    }
                    catch(java.nio.channels.CancelledKeyException ex)
                    {
                        //assert(false); // The channel might already be closed at this point so we can't assert.
                    }
                }
                socketMap.remove(info.fd);
            }
            finishedList.clear();
        }

        assert(_destroyed);

        try
        {
            _selector.close();
        }
        catch(java.io.IOException ex)
        {
            // Ignore.
        }

        try
        {
            _fdIntrWrite.close();
        }
        catch(java.io.IOException ex)
        {
            //
            // BUGFIX:
            //
            // Ignore this exception. This shouldn't happen
            // but for some reasons the close() call raises
            // "java.io.IOException: No such file or
            // directory" under Linux with JDK 1.4.2.
            //
        }
        _fdIntrWrite = null;
        
        try
        {
            _fdIntrRead.close();
        }
        catch(java.io.IOException ex)
        {
        }
        _fdIntrRead = null;
    }

    private int
    convertStatus(SocketStatus status)
    {
        if(status == SocketStatus.NeedConnect)
        {
            return java.nio.channels.SelectionKey.OP_CONNECT;
        }
        else if(status == SocketStatus.NeedRead)
        {
            return java.nio.channels.SelectionKey.OP_READ;
        }
        else
        {
            assert(status == SocketStatus.NeedWrite);
            return java.nio.channels.SelectionKey.OP_WRITE;
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

    private final class SocketInfo implements TimerTask
    {
        java.nio.channels.SelectableChannel fd;
        SocketReadyCallback cb;
        SocketStatus status;
        int timeout;
        java.nio.channels.SelectionKey key;

        public void
        runTimerTask()
        {
            this.cb.socketTimeout(); // Exceptions will be reported by the timer thread.
        }

        SocketInfo(java.nio.channels.SelectableChannel fd, SocketReadyCallback cb, SocketStatus status, int timeout)
        {
            this.fd = fd;
            this.cb = cb;
            this.status = status;
            this.timeout = timeout;
        }
    }

    private final class HelperThread extends Thread
    {
        HelperThread()
        {
            String threadName = _instance.initializationData().properties.getProperty("Ice.ProgramName");
            if(threadName.length() > 0)
            {
                threadName += "-";
            }
            setName(threadName + "Ice.SelectorThread");
        }

        public void
        run()
        {
            if(_instance.initializationData().threadHook != null)
            {
                _instance.initializationData().threadHook.start();
            }

            try
            {
                SelectorThread.this.run();
            }
            catch(Ice.LocalException ex)
            {
                java.io.StringWriter sw = new java.io.StringWriter();
                java.io.PrintWriter pw = new java.io.PrintWriter(sw);
                ex.printStackTrace(pw);
                pw.flush();
                String s = "exception in selector thread " + getName() + ":\n" + sw.toString();
                _instance.initializationData().logger.error(s);
            }
            catch(java.lang.Exception ex)
            {
                java.io.StringWriter sw = new java.io.StringWriter();
                java.io.PrintWriter pw = new java.io.PrintWriter(sw);
                ex.printStackTrace(pw);
                pw.flush();
                String s = "unknown exception in selector thread " + getName() + ":\n" + sw.toString();
                _instance.initializationData().logger.error(s);
            }

            if(_instance.initializationData().threadHook != null)
            {
                _instance.initializationData().threadHook.stop();
            }
        }
    }

    private HelperThread _thread;
    private Timer _timer;
}
