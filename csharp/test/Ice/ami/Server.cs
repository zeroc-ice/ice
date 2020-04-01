//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using Ice.ami.Test;
using Ice.ami.Test.Outer.Inner;

namespace Ice.ami
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

            using var communicator = initialize(properties);
            communicator.SetProperty("TestAdapter.Endpoints", getTestEndpoint(0));

            var adapter = communicator.CreateObjectAdapter("TestAdapter");
            adapter.Add("test", new TestIntf());
            adapter.Add("test2", new TestIntf2());
            adapter.Activate();
            serverReady();
            communicator.WaitForShutdown();
        }

        public static int Main(string[] args) => TestDriver.runTest<Server>(args);
    }
}
