// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice.exceptions;

public class Server : TestHelper
{
    public override void run(string[] args)
    {
        Properties properties = createTestProperties(ref args);
        properties.setProperty("Ice.Warn.Dispatch", "0");
        properties.setProperty("Ice.Warn.Connections", "0");
        properties.setProperty("Ice.MessageSizeMax", "10"); // 10KB max
        using Communicator communicator = initialize(properties);
        communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
        communicator.getProperties().setProperty("TestAdapter2.Endpoints", getTestEndpoint(1));
        communicator.getProperties().setProperty("TestAdapter2.MessageSizeMax", "0");
        communicator.getProperties().setProperty("TestAdapter3.Endpoints", getTestEndpoint(2));
        communicator.getProperties().setProperty("TestAdapter3.MessageSizeMax", "1");

        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        Ice.ObjectAdapter adapter2 = communicator.createObjectAdapter("TestAdapter2");
        Ice.ObjectAdapter adapter3 = communicator.createObjectAdapter("TestAdapter3");
        Ice.Object obj = new ThrowerI();
        adapter.add(obj, Ice.Util.stringToIdentity("thrower"));
        adapter2.add(obj, Ice.Util.stringToIdentity("thrower"));
        adapter3.add(obj, Ice.Util.stringToIdentity("thrower"));
        adapter.activate();
        adapter2.activate();
        adapter3.activate();
        serverReady();
        communicator.waitForShutdown();
    }

    public static Task<int> Main(string[] args) =>
        TestDriver.runTestAsync<Server>(args);
}
