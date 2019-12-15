//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice.threadPoolPriority.Test;

namespace Ice
{
    namespace threadPoolPriority
    {
        public class Server : global::Test.TestHelper
        {
            public override void run(string[] args)
            {
                var properties = createTestProperties(ref args);
                properties.setProperty("Ice.ThreadPool.Server.ThreadPriority", "AboveNormal");
                using (var communicator = initialize(properties))
                {
                    communicator.Properties.setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                    var adapter = communicator.createObjectAdapter("TestAdapter");
                    adapter.Add(new PriorityI(), "test");
                    adapter.Activate();
                    serverReady();
                    communicator.waitForShutdown();
                }
            }

            public static int Main(string[] args)
            {
                return global::Test.TestDriver.runTest<Server>(args);
            }
        }
    }
}
