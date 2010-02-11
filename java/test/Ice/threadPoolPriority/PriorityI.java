
package test.Ice.threadPoolPriority;

import test.Ice.threadPoolPriority.Test._PriorityDisp;

public class PriorityI extends _PriorityDisp
{

    public void shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    public int getPriority(Ice.Current current)
    {
        return Thread.currentThread().getPriority();
    }
}
