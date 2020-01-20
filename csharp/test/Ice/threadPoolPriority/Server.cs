//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice.threadPoolPriority.Test;

namespace Ice.threadPoolPriority
{
    public class Server : global::Test.TestHelper
    {
        public override void run(string[] args)
        {
            var properties = createTestProperties(ref args);
            properties["Ice.ThreadPool.Server.ThreadPriority"] = "AboveNormal";
            using var communicator = initialize(properties);
            communicator.SetProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            var adapter = communicator.CreateObjectAdapter("TestAdapter");
            adapter.Add(new Priority(), "test");
            adapter.Activate();
            serverReady();
            communicator.WaitForShutdown();
        }

        public static int Main(string[] args) => global::Test.TestDriver.runTest<Server>(args);
    }
}
