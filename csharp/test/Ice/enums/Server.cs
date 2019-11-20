//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using Ice.enums.Test;

namespace Ice
{
    namespace enums
    {
        public class Server : TestHelper
        {
            public override void run(string[] args)
            {
                Ice.Properties properties = createTestProperties(ref args);
                properties.setProperty("Ice.ServerIdleTime", "30");
                using (var communicator = initialize(properties))
                {
                    communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                    Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
                    adapter.Add(new TestI(), "test");
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
