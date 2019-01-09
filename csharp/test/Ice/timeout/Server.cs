// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

namespace Ice
{
    namespace timeout
    {
        public class Server : global::Test.TestHelper
        {
            public override void run(string[] args)
            {
                var properties = createTestProperties(ref args);
                //
                // This test kills connections, so we don't want warnings.
                //
                properties.setProperty("Ice.Warn.Connections", "0");

                //
                // The client sends large messages to cause the transport
                // buffers to fill up.
                //
                properties.setProperty("Ice.MessageSizeMax", "20000");

                //
                // Limit the recv buffer size, this test relies on the socket
                // send() blocking after sending a given amount of data.
                //
                properties.setProperty("Ice.TCP.RcvSize", "50000");
                using(var communicator = initialize(properties))
                {
                    communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                    communicator.getProperties().setProperty("ControllerAdapter.Endpoints", getTestEndpoint(1));
                    communicator.getProperties().setProperty("ControllerAdapter.ThreadPool.Size", "1");

                    var adapter = communicator.createObjectAdapter("TestAdapter");
                    adapter.add(new TimeoutI(), Ice.Util.stringToIdentity("timeout"));
                    adapter.activate();

                    var controllerAdapter = communicator.createObjectAdapter("ControllerAdapter");
                    controllerAdapter.add(new ControllerI(adapter), Ice.Util.stringToIdentity("controller"));
                    controllerAdapter.activate();
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
