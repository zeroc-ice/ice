//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using Ice.seqMapping.AMD.Test;

namespace Ice.seqMapping.AMD
{
    public class Server : TestHelper
    {
        public override void run(string[] args)
        {
            using var communicator = initialize(createTestProperties(ref args),
                typeIdNamespaces: new string[] { "Ice.seqMapping.AMD.TypeId" });
            communicator.SetProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            var adapter = communicator.CreateObjectAdapter("TestAdapter");
            adapter.Add("test", new MyClass());
            adapter.Activate();
            serverReady();
            communicator.WaitForShutdown();
        }

        public static int Main(string[] args) => TestDriver.runTest<Server>(args);
    }
}
