//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.threadPoolPriority;

import test.Ice.threadPoolPriority.Test._PriorityDisp;

public class PriorityI extends _PriorityDisp
{

    @Override
    public void shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    @Override
    public int getPriority(Ice.Current current)
    {
        return Thread.currentThread().getPriority();
    }
}
