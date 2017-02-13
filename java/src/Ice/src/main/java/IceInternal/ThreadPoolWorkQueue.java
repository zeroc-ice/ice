// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

import java.util.List;

final class ThreadPoolWorkQueue extends EventHandler
{
    ThreadPoolWorkQueue(Instance instance, ThreadPool threadPool, Selector selector)
    {
        _threadPool = threadPool;
        _selector = selector;
        _destroyed = false;
        
        _registered = SocketOperation.Read;
    }

    @Override
    protected synchronized void finalize() throws Throwable
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

    synchronized void destroy()
    {
        assert(!_destroyed);
        _destroyed = true;
        _selector.wakeup();
    }

    synchronized void queue(ThreadPoolWorkItem item)
    {
        if(_destroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }
        assert(item != null);
        _workItems.add(item);
        _selector.wakeup();
    }

    @Override
    public void message(ThreadPoolCurrent current)
    {
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
                _selector.wakeup();
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
    public void finished(ThreadPoolCurrent current, boolean close)
    {
        assert(false);
    }

    @Override
    public String toString()
    {
        return "work queue";
    }

    @Override
    public java.nio.channels.SelectableChannel fd()
    {
        return null;
    }

    // Return the number of pending events.
    synchronized int size()
    {
        int sz = _workItems.size();
        if(_destroyed)
        {
            sz++;
        }
        return sz;
    }
    
    synchronized void update(List<EventHandlerOpPair> handlers)
    {
        int sz = size();
        while(sz > 0)
        {
            handlers.add(_opPair);
            --sz;
        }
    }

    private final ThreadPool _threadPool;
    private boolean _destroyed;
    private Selector _selector;

    private EventHandlerOpPair _opPair = new EventHandlerOpPair(this, SocketOperation.Read);
    private java.util.LinkedList<ThreadPoolWorkItem> _workItems = new java.util.LinkedList<ThreadPoolWorkItem>();

}
