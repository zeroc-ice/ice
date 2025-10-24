// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice.retry;

public class Server : TestHelper
{
    public override void run(string[] args)
    {
        Properties properties = createTestProperties(ref args);
        properties.setProperty("Ice.Warn.Dispatch", "0");
        properties.setProperty("Ice.Warn.Connections", "0");
        using Communicator communicator = initialize(properties);
        communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
        ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        adapter.add(new RetryI(), Ice.Util.stringToIdentity("retry"));
        adapter.activate();
        serverReady();
        communicator.waitForShutdown();
    }

    public static Task<int> Main(string[] args) =>
        TestDriver.runTestAsync<Server>(args);
}
