// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice.ami
{
    public class Server : TestHelper
    {
        public override void run(string[] args)
        {
            Ice.Properties properties = createTestProperties(ref args);

            //
            // Disable collocation optimization to test async/await dispatch.
            //
            properties.setProperty("Ice.Default.CollocationOptimized", "0");

            //
            // This test kills connections, so we don't want warnings.
            //
            properties.setProperty("Ice.Warn.Connections", "0");

            //
            // Limit the recv buffer size, this test relies on the socket
            // send() blocking after sending a given amount of data.
            //
            properties.setProperty("Ice.TCP.RcvSize", "50000");

            using (var communicator = initialize(properties))
            {
                communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                communicator.getProperties().setProperty("ControllerAdapter.Endpoints", getTestEndpoint(1));
                communicator.getProperties().setProperty("ControllerAdapter.ThreadPool.Size", "1");

                Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
                Ice.ObjectAdapter adapter2 = communicator.createObjectAdapter("ControllerAdapter");

                adapter.add(new TestI(), Ice.Util.stringToIdentity("test"));
                adapter.add(new TestII(), Ice.Util.stringToIdentity("test2"));
                adapter.activate();
                adapter2.add(new TestControllerI(adapter), Ice.Util.stringToIdentity("testController"));
                adapter2.activate();
                serverReady();
                communicator.waitForShutdown();
            }
        }

        public static Task<int> Main(string[] args) =>
            TestDriver.runTestAsync<Server>(args);
    }
}
