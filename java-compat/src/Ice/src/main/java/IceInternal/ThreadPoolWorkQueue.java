//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package IceInternal;

final class ThreadPoolWorkQueue extends EventHandler
{
    ThreadPoolWorkQueue(Instance instance, ThreadPool threadPool, Selector selector)
    {
        _threadPool = threadPool;
        _selector = selector;
        _destroyed = false;
        _registered = SocketOperation.Read;
    }

    @SuppressWarnings("deprecation")
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

    void destroy()
    {
        // Called with the thread pool locked
        assert(!_destroyed);
        _destroyed = true;
        _selector.ready(this, SocketOperation.Read, true);
    }

    void queue(ThreadPoolWorkItem item)
    {
        // Called with the thread pool locked
        assert(item != null);
        _workItems.add(item);
        if(_workItems.size() == 1)
        {
            _selector.ready(this, SocketOperation.Read, true);
        }
    }

    @Override
    public void message(ThreadPoolCurrent current)
    {
        ThreadPoolWorkItem workItem = null;
        synchronized(_threadPool)
        {
            if(!_workItems.isEmpty())
            {
                workItem = _workItems.removeFirst();
                assert(workItem != null);
            }
            if(_workItems.isEmpty() && !_destroyed)
            {
                _selector.ready(this, SocketOperation.Read, false);
            }
        }

        if(workItem != null)
        {
            workItem.execute(current);
        }
        else
        {
            assert(_destroyed);
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

    @Override
    public void setReadyCallback(ReadyCallback callback)
    {
        // Ignore, we don't use the ready callback.
    }

    private final ThreadPool _threadPool;
    private boolean _destroyed;
    private Selector _selector;
    private java.util.LinkedList<ThreadPoolWorkItem> _workItems = new java.util.LinkedList<ThreadPoolWorkItem>();
}
