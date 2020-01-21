//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using Ice.seqMapping.Test;

namespace Ice.seqMapping
{
    public class Server : TestHelper
    {
        public override void run(string[] args)
        {
            using var communicator = initialize(createTestProperties(ref args),
                typeIdNamespaces: new string[] { "Ice.seqMapping.TypeId" });
            communicator.SetProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            var adapter = communicator.CreateObjectAdapter("TestAdapter");
            adapter.Add(new MyClass(), "test");
            adapter.Activate();
            serverReady();
            communicator.WaitForShutdown();
        }

        public static int Main(string[] args) => TestDriver.runTest<Server>(args);
    }
}
