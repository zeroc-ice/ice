// Copyright (c) ZeroC, Inc.

namespace Ice.idleTimeout;

public class Server : global::Test.TestHelper
{
    public override void run(string[] args)
    {
        var properties = createTestProperties(ref args);

        properties.setProperty("Ice.Warn.Connections", "0");
        properties.setProperty("TestAdapter.Connection.IdleTimeout", "1"); // 1 second
        properties.setProperty("TestAdapter.ThreadPool.Size", "1"); // dedicated thread pool with a single thread

        using var communicator = initialize(properties);
        communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint());
        var adapter = communicator.createObjectAdapter("TestAdapter");
        adapter.add(new TestIntfI(), Ice.Util.stringToIdentity("test"));
        adapter.activate();

        communicator.getProperties().setProperty("TestAdapter3s.Endpoints", getTestEndpoint(1));
        communicator.getProperties().setProperty("TestAdapter3s.Connection.IdleTimeout", "3");
        var adapter3s = communicator.createObjectAdapter("TestAdapter3s");
        adapter3s.add(new TestIntfI(), Ice.Util.stringToIdentity("test"));
        adapter3s.activate();

        serverReady();
        communicator.waitForShutdown();
    }

    public static Task<int> Main(string[] args) =>
        global::Test.TestDriver.runTestAsync<Server>(args);
}
