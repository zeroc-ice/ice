//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using Ice.ami.Test;
using Ice.ami.Test.Outer;
using Ice.ami.Test.Outer.Inner;

namespace Ice
{
    namespace ami
    {
        public class Server : TestHelper
        {
            public override void run(string[] args)
            {
                var properties = createTestProperties(ref args);

                //
                // Disable collocation optimization to test async/await dispatch.
                //
                properties["Ice.Default.CollocationOptimized"] = "0";

                //
                // This test kills connections, so we don't want warnings.
                //
                properties["Ice.Warn.Connections"] = "0";

                //
                // Limit the recv buffer size, this test relies on the socket
                // send() blocking after sending a given amount of data.
                //
                properties["Ice.TCP.RcvSize"] = "50000";

                using (var communicator = initialize(properties))
                {
                    communicator.SetProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                    communicator.SetProperty("ControllerAdapter.Endpoints", getTestEndpoint(1));
                    communicator.SetProperty("ControllerAdapter.ThreadPool.Size", "1");

                    Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
                    Ice.ObjectAdapter adapter2 = communicator.createObjectAdapter("ControllerAdapter");

                    adapter.Add(new TestI(), "test");
                    adapter.Add(new TestII(), "test2");
                    adapter.Activate();
                    adapter2.Add(new TestControllerI(adapter), "testController");
                    adapter2.Activate();
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
