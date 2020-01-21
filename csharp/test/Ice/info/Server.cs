//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using Ice.info.Test;

namespace Ice.info
{
    public class Server : TestHelper
    {
        public override void run(string[] args)
        {
            using var communicator = initialize(ref args);
            communicator.SetProperty("TestAdapter.Endpoints", getTestEndpoint(0) + ":" + getTestEndpoint(0, "udp"));
            ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
            adapter.Add(new TestIntf(), "test");
            adapter.Activate();
            serverReady();
            communicator.WaitForShutdown();
        }

        public static int Main(string[] args) => TestDriver.runTest<Server>(args);
    }
}
