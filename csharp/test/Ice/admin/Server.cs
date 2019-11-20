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
                    communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0) + " -t 10000");
                    Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
                    Ice.Identity id = Ice.Util.stringToIdentity("factory");
                    adapter.Add(new RemoteCommunicatorFactoryI(), id);
                    adapter.activate();
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
