//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using Ice.retry.Test;

namespace Ice
{
    namespace retry
    {
        public class Server : TestHelper
        {
            public override void run(string[] args)
            {
                var properties = createTestProperties(ref args);
                properties.setProperty("Ice.Warn.Dispatch", "0");
                properties.setProperty("Ice.Warn.Connections", "0");
                using (var communicator = initialize(properties))
                {
                    communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                    var adapter = communicator.createObjectAdapter("TestAdapter");
                    adapter.Add(new RetryI(), "retry");
                    adapter.Activate();
                    serverReady();
                    communicator.waitForShutdown();
                }
            }

            public static int Main(string[] args)
            {
                return TestDriver.runTest<Server>(args);
            }
        }
    }
}
