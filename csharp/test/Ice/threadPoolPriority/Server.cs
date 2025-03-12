// Copyright (c) ZeroC, Inc.

namespace Ice.threadPoolPriority
{
    public class Server : global::Test.TestHelper
    {
        public override void run(string[] args)
        {
            var properties = createTestProperties(ref args);
            properties.setProperty("Ice.ThreadPool.Server.ThreadPriority", "AboveNormal");
            using (var communicator = initialize(properties))
            {
                communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                var adapter = communicator.createObjectAdapter("TestAdapter");
                adapter.add(new PriorityI(), Ice.Util.stringToIdentity("test"));
                adapter.activate();
                serverReady();
                communicator.waitForShutdown();
            }
        }

        public static Task<int> Main(string[] args) =>
            global::Test.TestDriver.runTestAsync<Server>(args);
    }
}
