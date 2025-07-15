// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice.maxDispatches;

public class Server : global::Test.TestHelper
{
    public override void run(string[] args)
    {
        var properties = createTestProperties(ref args);
        properties.setProperty("Ice.ThreadPool.Server.Size", "10"); // plenty of threads to handle the requests
        using var communicator = initialize(properties);

        var responder = new ResponderI();
        var testIntf = new TestIntfI(responder);

        communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
        var adapter = communicator.createObjectAdapter("TestAdapter");
        adapter.add(testIntf, Ice.Util.stringToIdentity("test"));
        adapter.activate();

        communicator.getProperties().setProperty("ResponderAdapter.Endpoints", getTestEndpoint(1));
        adapter = communicator.createObjectAdapter("ResponderAdapter");
        adapter.add(responder, Ice.Util.stringToIdentity("responder"));
        adapter.activate();

        communicator.getProperties().setProperty("TestAdapterMax10.Endpoints", getTestEndpoint(2));
        communicator.getProperties().setProperty("TestAdapterMax10.Connection.MaxDispatches", "10");
        adapter = communicator.createObjectAdapter("TestAdapterMax10");
        adapter.add(testIntf, Ice.Util.stringToIdentity("test"));
        adapter.activate();

        communicator.getProperties().setProperty("TestAdapterMax1.Endpoints", getTestEndpoint(3));
        communicator.getProperties().setProperty("TestAdapterMax1.Connection.MaxDispatches", "1");
        adapter = communicator.createObjectAdapter("TestAdapterMax1");
        adapter.add(testIntf, Ice.Util.stringToIdentity("test"));
        adapter.activate();

        communicator.getProperties().setProperty("TestAdapterSerialize.Endpoints", getTestEndpoint(4));
        communicator.getProperties().setProperty("TestAdapterSerialize.ThreadPool.Size", "10");
        communicator.getProperties().setProperty("TestAdapterSerialize.ThreadPool.Serialize", "1");
        adapter = communicator.createObjectAdapter("TestAdapterSerialize");
        adapter.add(testIntf, Ice.Util.stringToIdentity("test"));
        adapter.activate();

        serverReady();
        communicator.waitForShutdown();
    }

    public static Task<int> Main(string[] args) =>
        global::Test.TestDriver.runTestAsync<Server>(args);
}
