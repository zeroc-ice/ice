// Copyright (c) ZeroC, Inc.

namespace Ice.threadPoolPriority;

public class PriorityI : Test.PriorityDisp_
{
    public override void shutdown(Ice.Current current) => current.adapter.getCommunicator().shutdown();

    public override string getPriority(Ice.Current current) => Thread.CurrentThread.Priority.ToString();
}
