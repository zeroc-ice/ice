// Copyright (c) ZeroC, Inc.

package test.Ice.threadPoolPriority;

import com.zeroc.Ice.Current;

import test.Ice.threadPoolPriority.Test.Priority;

public class PriorityI implements Priority {
    @Override
    public void shutdown(Current current) {
        current.adapter.getCommunicator().shutdown();
    }

    @Override
    public int getPriority(Current current) {
        return Thread.currentThread().getPriority();
    }
}
