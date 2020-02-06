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
                    ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
                    adapter.Add("factory", new RemoteCommunicatorFactoryI());
                    adapter.Activate();
                    serverReady();
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
