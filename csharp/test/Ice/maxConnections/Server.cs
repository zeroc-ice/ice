// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice.maxConnections;

public class Server : global::Test.TestHelper
{
    public override void run(string[] args)
    {
        using var communicator = initialize(ref args);

        // Plain adapter with no limit.
        communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
        var adapter = communicator.createObjectAdapter("TestAdapter");
        adapter.add(new TestIntfI(), Ice.Util.stringToIdentity("test"));
        adapter.activate();

        // Accepts 10 connections.
        communicator.getProperties().setProperty("TestAdapterMax10.Endpoints", getTestEndpoint(1));
        communicator.getProperties().setProperty("TestAdapterMax10.MaxConnections", "10");
        var adapterMax10 = communicator.createObjectAdapter("TestAdapterMax10");
        adapterMax10.add(new TestIntfI(), Ice.Util.stringToIdentity("test"));
        adapterMax10.activate();

        serverReady();
        communicator.waitForShutdown();
    }

    public static Task<int> Main(string[] args) =>
        global::Test.TestDriver.runTestAsync<Server>(args);
}
