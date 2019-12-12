//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using Ice.info.Test;

namespace Ice
{
    namespace info
    {
        public class Server : TestHelper
        {
            public override void run(string[] args)
            {
                using (var communicator = initialize(ref args))
                {
                    communicator.Properties.setProperty("TestAdapter.Endpoints",
                                                             getTestEndpoint(0) + ":" + getTestEndpoint(0, "udp"));
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
