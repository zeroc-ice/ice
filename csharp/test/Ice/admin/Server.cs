//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using Ice.admin.Test;

namespace Ice
{
    namespace admin
    {
        public class Server : TestHelper
        {
            public override void run(string[] args)
            {
                using (var communicator = initialize(ref args))
                {
                    communicator.SetProperty("TestAdapter.Endpoints", $"{getTestEndpoint(0)} -t 10000");
                    ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
                    adapter.Add(new RemoteCommunicatorFactoryI(), "factory");
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
