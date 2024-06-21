// Copyright (c) ZeroC, Inc.

namespace Ice.inactivityTimeout;

internal sealed class TestIntfI : Test.TestIntfDisp_
{
    public override void sleep(int ms, Current current) => Thread.Sleep(ms);

    public override void shutdown(Current current) => current.adapter.getCommunicator().shutdown();
}
