//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using Ice.optional.Test;

namespace Ice.optional
{
    public class Server : TestHelper
    {
        public override void run(string[] args)
        {
            using var communicator = initialize(createTestProperties(ref args),
                typeIdNamespaces: new string[] { "Ice.optional.TypeId" });
            communicator.SetProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            var adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.Add(new Initial(), "initial");
            adapter.Activate();
            serverReady();
            communicator.waitForShutdown();
        }

        public static int Main(string[] args) => TestDriver.runTest<Server>(args);
    }
}
