// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

import java.util.concurrent.ExecutorService;

final class ThreadPoolWorkQueue extends EventHandler
{
    ThreadPoolWorkQueue(Instance instance, ThreadPool threadPool, Selector selector)
    {
        _executor = instance.getQueueExecutor();
        _threadPool = threadPool;
        _selector = selector;
        _destroyed = false;

        Network.SocketPair pair = Network.createPipe();
        _fdIntrRead = (java.nio.channels.ReadableByteChannel)pair.source;
        _fdIntrWrite = pair.sink;
        try
        {
            pair.source.configureBlocking(false);
        }
        catch(java.io.IOException ex)
        {
            throw new Ice.SyscallException(ex);
        }

        _selector.initialize(this);
        _selector.update(this, SocketOperation.None, SocketOperation.Read);
    }

    @Override
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
    close()
    {
        try
        {
            _fdIntrWrite.close();
        }
        catch(java.io.IOException ex)
        {
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

    public synchronized
    void destroy()
    {
        assert(!_destroyed);
        _destroyed = true;
        postMessage();
    }

    public synchronized void
    queue(ThreadPoolWorkItem item)
    {
        if(_destroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }
        assert(item != null);
        _workItems.add(item);
        postMessage();
    }

    @Override
    public void
    message(ThreadPoolCurrent current)
    {
        java.nio.ByteBuffer buf = java.nio.ByteBuffer.allocate(1);
        try
        {
            buf.rewind();
            int ret = _fdIntrRead.read(buf);
            assert(ret > 0);
        }
        catch(java.io.IOException ex)
        {
            throw new Ice.SocketException(ex);
        }

        ThreadPoolWorkItem workItem = null;
        synchronized(this)
        {
            if(!_workItems.isEmpty())
            {
                workItem = _workItems.removeFirst();
                assert(workItem != null);
            }
            else
            {
                assert(_destroyed);
                postMessage();
            }
        }

        if(workItem != null)
        {
            workItem.execute(current);
        }
        else
        {
            _threadPool.ioCompleted(current);
            throw new ThreadPool.DestroyedException();
        }
    }

    @Override
    public void
    finished(ThreadPoolCurrent current)
    {
        assert(false);
    }

    @Override
    public String
    toString()
    {
        return "work queue";
    }

    @Override
    public java.nio.channels.SelectableChannel
    fd()
    {
        return (java.nio.channels.SelectableChannel)_fdIntrRead;
    }

    public void
    postMessage()
    {
        if(_executor != null)
        {
            _executor.submit(new Runnable() {

                @Override
                public void run()
                {
                    postMessageInternal();
                }
            });
        }
        else {
            postMessageInternal();
        }
    }

    private void
    postMessageInternal()
    {
        java.nio.ByteBuffer buf = java.nio.ByteBuffer.allocate(1);
        buf.put(0, (byte)0);
        while(buf.hasRemaining())
        {
            try
            {
                _fdIntrWrite.write(buf);
            }
            //
            // This is thrown if the thread is interrupted.
            //
            catch(java.nio.channels.ClosedChannelException ex)
            {
                break;
            }
            catch(java.io.IOException ex)
            {
                throw new Ice.SocketException(ex);
            }
        }
    }

    private final ThreadPool _threadPool;
    private final Selector _selector;
    boolean _destroyed;

    private java.nio.channels.ReadableByteChannel _fdIntrRead;
    private java.nio.channels.WritableByteChannel _fdIntrWrite;

    private java.util.LinkedList<ThreadPoolWorkItem> _workItems = new java.util.LinkedList<ThreadPoolWorkItem>();
    private ExecutorService _executor;
}
