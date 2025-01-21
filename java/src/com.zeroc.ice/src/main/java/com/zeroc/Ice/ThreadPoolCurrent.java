// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

final class ThreadPoolCurrent {
    ThreadPoolCurrent(
            Instance instance, ThreadPool threadPool, ThreadPool.EventHandlerThread thread) {
        operation = SocketOperation.None;
        stream =
                new InputStream(
                        instance,
                        Protocol.currentProtocolEncoding,
                        instance.cacheMessageBuffers() > 1);

        _threadPool = threadPool;
        _thread = thread;
        _ioCompleted = false;
        _leader = false;
    }

    public int operation;
    public InputStream stream; // A per-thread stream to be used by event handlers for optimization.

    public boolean ioReady() {
        return (_handler._registered & operation) > 0;
    }

    public void ioCompleted() {
        _threadPool.ioCompleted(this);
    }

    public void executeFromThisThread(RunnableThreadPoolWorkItem workItem) {
        _threadPool.executeFromThisThread(workItem);
    }

    final ThreadPool _threadPool;
    final ThreadPool.EventHandlerThread _thread;
    EventHandler _handler;
    boolean _ioCompleted;
    boolean _leader;
}
