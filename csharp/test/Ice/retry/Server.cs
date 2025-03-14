// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice.retry;

public class Server : TestHelper
{
    public override void run(string[] args)
    {
        var properties = createTestProperties(ref args);
        properties.setProperty("Ice.Warn.Dispatch", "0");
        properties.setProperty("Ice.Warn.Connections", "0");
        using var communicator = initialize(properties);
        communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
        var adapter = communicator.createObjectAdapter("TestAdapter");
        adapter.add(new RetryI(), Ice.Util.stringToIdentity("retry"));
        adapter.activate();
        serverReady();
        communicator.waitForShutdown();
    }

    public static Task<int> Main(string[] args) =>
        TestDriver.runTestAsync<Server>(args);
}
