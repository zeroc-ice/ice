// Copyright (c) ZeroC, Inc.

namespace IceGrid.simple;

public sealed class TestI : Test.TestIntfDisp_
{
    public override void
    shutdown(Ice.Current current) => current.adapter.getCommunicator().shutdown();
}
