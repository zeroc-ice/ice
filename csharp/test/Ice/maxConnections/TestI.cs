// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice.maxConnections;

internal sealed class TestIntfI : Test.TestIntfDisp_
{
    public override void shutdown(Current current) => current.adapter.getCommunicator().shutdown();
}
