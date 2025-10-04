// Copyright (c) ZeroC, Inc.

namespace Ice.idleTimeout;

internal sealed class TestIntfI : Test.TestIntfDisp_
{
    public override void sleep(int ms, Current current) => Thread.Sleep(ms);

    public override void runGC(Current current) => GC.Collect();

    public override void shutdown(Current current) => current.adapter.getCommunicator().shutdown();
}
