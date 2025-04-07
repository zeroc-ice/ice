// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.nio.channels.SelectableChannel;
import java.util.LinkedList;

final class ThreadPoolWorkQueue extends EventHandler {
    ThreadPoolWorkQueue(Instance instance, ThreadPool threadPool, Selector selector) {
        _threadPool = threadPool;
        _selector = selector;
        _destroyed = false;
        _registered = SocketOperation.Read;
    }

    @SuppressWarnings("deprecation")
    @Override
    protected synchronized void finalize() throws Throwable {
        try {
            Assert.FinalizerAssert(_destroyed);
        } catch (Exception ex) {
        } finally {
            super.finalize();
        }
    }

    void destroy() {
        // Called with the thread pool locked
        assert (!_destroyed);
        _destroyed = true;
        _selector.ready(this, SocketOperation.Read, true);
    }

    void queue(ThreadPoolWorkItem item) {
        // Called with the thread pool locked
        assert (item != null);
        _workItems.add(item);
        if (_workItems.size() == 1) {
            _selector.ready(this, SocketOperation.Read, true);
        }
    }

    @Override
    public void message(ThreadPoolCurrent current) {
        ThreadPoolWorkItem workItem = null;
        synchronized (_threadPool) {
            if (!_workItems.isEmpty()) {
                workItem = _workItems.removeFirst();
                assert (workItem != null);
            }
            if (_workItems.isEmpty() && !_destroyed) {
                _selector.ready(this, SocketOperation.Read, false);
            }
        }

        if (workItem != null) {
            workItem.execute(current);
        } else {
            assert _destroyed;
            _threadPool.ioCompleted(current);
            throw new ThreadPool.DestroyedException();
        }
    }

    @Override
    public void finished(ThreadPoolCurrent current, boolean close) {
        assert false;
    }

    @Override
    public String toString() {
        return "work queue";
    }

    @Override
    public SelectableChannel fd() {
        return null;
    }

    @Override
    public void setReadyCallback(ReadyCallback callback) {
        // Ignore, we don't use the ready callback.
    }

    private final ThreadPool _threadPool;
    private boolean _destroyed;
    private final Selector _selector;
    private final LinkedList<ThreadPoolWorkItem> _workItems = new LinkedList<>();
}
