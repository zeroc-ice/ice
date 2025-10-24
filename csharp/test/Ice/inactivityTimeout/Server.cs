// Copyright (c) ZeroC, Inc.

namespace Ice.inactivityTimeout;

public class Server : global::Test.TestHelper
{
    public override void run(string[] args)
    {
        Properties properties = createTestProperties(ref args);

        // We configure a low idle timeout to make sure we send heartbeats frequently. It's the sending of the
        // heartbeats that schedules the inactivity timer.
        properties.setProperty("Ice.Connection.Server.IdleTimeout", "1");
        properties.setProperty("TestAdapter.Connection.InactivityTimeout", "5");
        properties.setProperty("TestAdapter3s.Connection.InactivityTimeout", "3");

        using Communicator communicator = initialize(properties);
        communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
        communicator.getProperties().setProperty("TestAdapter3s.Endpoints", getTestEndpoint(1));

        ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        adapter.add(new TestIntfI(), Ice.Util.stringToIdentity("test"));
        adapter.activate();

        ObjectAdapter adapter3s = communicator.createObjectAdapter("TestAdapter3s");
        adapter3s.add(new TestIntfI(), Ice.Util.stringToIdentity("test"));
        adapter3s.activate();

        serverReady();
        communicator.waitForShutdown();
    }

    public static Task<int> Main(string[] args) =>
        global::Test.TestDriver.runTestAsync<Server>(args);
}
