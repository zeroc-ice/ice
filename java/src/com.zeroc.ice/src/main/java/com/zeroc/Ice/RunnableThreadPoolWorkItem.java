// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

//
// A helper class for thread pool work items that only need to call user callbacks. If an executor
// is installed with the communicator, the thread pool work item is executed with the executor,
// otherwise it's executed by a thread pool thread (after promoting a follower thread).
//
abstract class RunnableThreadPoolWorkItem implements ThreadPoolWorkItem, Runnable {
    public RunnableThreadPoolWorkItem() {
    }

    public RunnableThreadPoolWorkItem(Connection connection) {
        _connection = connection;
    }

    @Override
    public final void execute(ThreadPoolCurrent current) {
        current.ioCompleted(); // Promote a follower
        current.executeFromThisThread(this);
    }

    public Connection getConnection() {
        return _connection;
    }

    private Connection _connection;
}
