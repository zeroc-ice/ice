
using System.Threading;
using System;

public class PriorityI : Test.PriorityDisp_
{

    public override void shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    public override string getPriority(Ice.Current current)
    {
        return Thread.CurrentThread.Priority.ToString();
    }
}
