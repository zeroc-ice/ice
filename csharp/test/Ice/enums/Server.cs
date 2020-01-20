//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using Ice.enums.Test;

namespace Ice.enums
{
    public class Server : TestHelper
    {
        public override void run(string[] args)
        {
            var properties = createTestProperties(ref args);
            properties["Ice.ServerIdleTime"] = "30";
            using var communicator = initialize(properties);
            communicator.SetProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
            adapter.Add(new TestIntf(), "test");
            adapter.Activate();
            serverReady();
            communicator.WaitForShutdown();
        }

        public static int Main(string[] args) => TestDriver.runTest<Server>(args);
    }
}
