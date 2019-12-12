//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using Ice.acm.Test;

namespace Ice
{
    namespace acm
    {
        public class Server : TestHelper
        {
            public override void run(string[] args)
            {
                var properties = createTestProperties(ref args);
                properties.setProperty("Ice.Warn.Connections", "0");
                properties.setProperty("Ice.ACM.Timeout", "1");
                using (var communicator = initialize(properties))
                {
                    communicator.Properties.setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                    communicator.Properties.setProperty("TestAdapter.ACM.Timeout", "0");
                    var adapter = communicator.createObjectAdapter("TestAdapter");
                    adapter.Add(new RemoteCommunicatorI(), "communicator");
                    adapter.Activate();
                    serverReady();
                    communicator.Properties.setProperty("Ice.PrintAdapterReady", "0");
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
