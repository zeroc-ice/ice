//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace ZeroC.Ice.Test.AMI
{
    public class Server : TestHelper
    {
        public override void Run(string[] args)
        {
            System.Collections.Generic.Dictionary<string, string> properties = CreateTestProperties(ref args);

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

            using Communicator communicator = Initialize(properties);
            communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0));
            communicator.SetProperty("TestAdapter2.Endpoints", GetTestEndpoint(1));

            ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
            adapter.Add("test", new TestIntf());
            adapter.Add("test2", new TestIntf2());
            adapter.Activate();

            ObjectAdapter adapter2 = communicator.CreateObjectAdapter("TestAdapter2", serializeDispatch: true);
            adapter2.Add("serialized", new TestIntf());
            adapter2.Activate();

            ServerReady();
            communicator.WaitForShutdown();
        }

        public static int Main(string[] args) => TestDriver.RunTest<Server>(args);
    }
}
