// Copyright (c) ZeroC, Inc.

namespace Ice.timeout;

public class Server : global::Test.TestHelper
{
    public override void run(string[] args)
    {
        Properties properties = createTestProperties(ref args);

        using Communicator communicator = initialize(properties);
        communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
        communicator.getProperties().setProperty("ControllerAdapter.Endpoints", getTestEndpoint(1));
        communicator.getProperties().setProperty("ControllerAdapter.ThreadPool.Size", "1");

        ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        adapter.add(new TimeoutI(), Ice.Util.stringToIdentity("timeout"));
        adapter.activate();

        ObjectAdapter controllerAdapter = communicator.createObjectAdapter("ControllerAdapter");
        controllerAdapter.add(new ControllerI(adapter), Ice.Util.stringToIdentity("controller"));
        controllerAdapter.activate();
        serverReady();
        communicator.waitForShutdown();
    }

    public static Task<int> Main(string[] args) =>
        global::Test.TestDriver.runTestAsync<Server>(args);
}
