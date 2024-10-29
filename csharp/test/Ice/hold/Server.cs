// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice.hold;

public class Server : TestHelper
{
    public override void run(string[] args)
    {
        using var communicator = initialize(ref args);
        communicator.getProperties().setProperty("TestAdapter1.Endpoints", getTestEndpoint(0));
        communicator.getProperties().setProperty("TestAdapter1.ThreadPool.Size", "5");
        communicator.getProperties().setProperty("TestAdapter1.ThreadPool.SizeMax", "5");
        communicator.getProperties().setProperty("TestAdapter1.ThreadPool.SizeWarn", "0");
        communicator.getProperties().setProperty("TestAdapter1.ThreadPool.Serialize", "0");
        Ice.ObjectAdapter adapter1 = communicator.createObjectAdapter("TestAdapter1");
        adapter1.add(new HoldI(adapter1), Ice.Util.stringToIdentity("hold"));

        communicator.getProperties().setProperty("TestAdapter2.Endpoints", getTestEndpoint(1));
        communicator.getProperties().setProperty("TestAdapter2.ThreadPool.Size", "5");
        communicator.getProperties().setProperty("TestAdapter2.ThreadPool.SizeMax", "5");
        communicator.getProperties().setProperty("TestAdapter2.ThreadPool.SizeWarn", "0");
        communicator.getProperties().setProperty("TestAdapter2.ThreadPool.Serialize", "1");
        Ice.ObjectAdapter adapter2 = communicator.createObjectAdapter("TestAdapter2");
        adapter2.add(new HoldI(adapter2), Ice.Util.stringToIdentity("hold"));

        adapter1.activate();
        adapter2.activate();
        serverReady();
        communicator.waitForShutdown();
    }

    public static Task<int> Main(string[] args) => TestDriver.runTestAsync<Server>(args);
}
