// Copyright (c) ZeroC, Inc.

namespace Ice.idleTimeout;

public class Server : global::Test.TestHelper
{
    public override void run(string[] args)
    {
        var properties = createTestProperties(ref args);

        properties.setProperty("Ice.Warn.Connections", "0");
        properties.setProperty("TestAdapter.Connection.IdleTimeout", "1"); // 1 second
        // Serialize dispatches on each incoming connection.
        properties.setProperty("TestAdapter.Connection.MaxDispatches", "1");

        using var communicator = initialize(properties);
        communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
        var adapter = communicator.createObjectAdapter("TestAdapter");
        adapter.add(new TestIntfI(), Ice.Util.stringToIdentity("test"));
        adapter.activate();

        communicator.getProperties().setProperty("TestAdapterDefaultMax.Endpoints", getTestEndpoint(1));
        communicator.getProperties().setProperty("TestAdapterDefaultMax.Connection.IdleTimeout", "1");
        var adapterDefaultMax = communicator.createObjectAdapter("TestAdapterDefaultMax");
        adapterDefaultMax.add(new TestIntfI(), Ice.Util.stringToIdentity("test"));
        adapterDefaultMax.activate();

        communicator.getProperties().setProperty("TestAdapter3s.Endpoints", getTestEndpoint(2));
        communicator.getProperties().setProperty("TestAdapter3s.Connection.IdleTimeout", "3");
        var adapter3s = communicator.createObjectAdapter("TestAdapter3s");
        adapter3s.add(new TestIntfI(), Ice.Util.stringToIdentity("test"));
        adapter3s.activate();

        communicator.getProperties().setProperty("TestAdapterNoIdleTimeout.Endpoints", getTestEndpoint(3));
        communicator.getProperties().setProperty("TestAdapterNoIdleTimeout.Connection.IdleTimeout", "0");
        var adapterNoIdleTimeout = communicator.createObjectAdapter("TestAdapterNoIdleTimeout");
        adapterNoIdleTimeout.add(new TestIntfI(), Ice.Util.stringToIdentity("test"));
        adapterNoIdleTimeout.activate();

        serverReady();
        communicator.waitForShutdown();
    }

    public static Task<int> Main(string[] args) =>
        global::Test.TestDriver.runTestAsync<Server>(args);
}
