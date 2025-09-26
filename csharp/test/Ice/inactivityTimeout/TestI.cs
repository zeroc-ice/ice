// Copyright (c) ZeroC, Inc.

namespace Ice.inactivityTimeout;

internal sealed class TestIntfI : Test.TestIntfDisp_
{
    public override Task sleepAsync(int ms, Current current) => Task.Delay(ms);

    public override void disableInactivityCheck(Current current) => current.con.disableInactivityCheck();

    public override void shutdown(Current current) => current.adapter.getCommunicator().shutdown();
}
