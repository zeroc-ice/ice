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
                properties["Ice.Warn.Connections"] = "0";
                properties["Ice.ACM.Timeout"] = "1";
                using (var communicator = initialize(properties))
                {
                    communicator.SetProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                    communicator.SetProperty("TestAdapter.ACM.Timeout", "0");
                    var adapter = communicator.CreateObjectAdapter("TestAdapter");
                    adapter.Add("communicator", new RemoteCommunicator());
                    adapter.Activate();
                    serverReady();
                    communicator.SetProperty("Ice.PrintAdapterReady", "0");
                    communicator.WaitForShutdown();
                }
            }

            public static int Main(string[] args)
            {
                return TestDriver.runTest<Server>(args);
            }
        }
    }
}
