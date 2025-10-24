// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice.proxy;

public class Server : TestHelper
{
    public override void run(string[] args)
    {
        Properties properties = createTestProperties(ref args);
        //
        // We don't want connection warnings because of the timeout test.
        //
        properties.setProperty("Ice.Warn.Connections", "0");
        properties.setProperty("Ice.Warn.Dispatch", "0");

        using Communicator communicator = initialize(properties);
        communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
        ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        adapter.add(new MyDerivedClassI(), Ice.Util.stringToIdentity("test"));
        adapter.add(new CI(), Ice.Util.stringToIdentity("c"));
        adapter.activate();
        serverReady();
        communicator.waitForShutdown();
    }

    public static Task<int> Main(string[] args) =>
        TestDriver.runTestAsync<Server>(args);
}
