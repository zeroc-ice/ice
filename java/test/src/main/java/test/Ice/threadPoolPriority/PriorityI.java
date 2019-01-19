//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.threadPoolPriority;

import test.Ice.threadPoolPriority.Test.Priority;

public class PriorityI implements Priority
{
    @Override
    public void shutdown(com.zeroc.Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    @Override
    public int getPriority(com.zeroc.Ice.Current current)
    {
        return Thread.currentThread().getPriority();
    }
}
