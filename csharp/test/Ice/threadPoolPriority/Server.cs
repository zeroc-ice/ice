//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice.threadPoolPriority.Test;

namespace Ice.threadPoolPriority
{
    public class Server : global::Test.TestHelper
    {
        public override void Run(string[] args)
        {
            var properties = CreateTestProperties(ref args);
            properties["Ice.ThreadPool.Server.ThreadPriority"] = "AboveNormal";
            using var communicator = Initialize(properties);
            communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0));
            var adapter = communicator.CreateObjectAdapter("TestAdapter");
            adapter.Add("test", new Priority());
            adapter.Activate();
            ServerReady();
            communicator.WaitForShutdown();
        }

        public static int Main(string[] args) => global::Test.TestDriver.RunTest<Server>(args);
    }
}
