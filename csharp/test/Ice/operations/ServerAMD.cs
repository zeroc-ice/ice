//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using Ice.operations.AMD.Test;

namespace Ice.operations.AMD
{
    public class Server : TestHelper
    {
        public override void run(string[] args)
        {
            var properties = createTestProperties(ref args);
            //
            // We don't want connection warnings because of the timeout test.
            //
            properties["Ice.Warn.Connections"] = "0";
            using var communicator = initialize(properties, typeIdNamespaces: new string[] { "Ice.operations.AMD.TypeId" });
            communicator.SetProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.Add(new MyDerivedClass(), "test");
            adapter.Activate();
            serverReady();
            communicator.waitForShutdown();
        }

        public static int Main(string[] args) => TestDriver.runTest<Server>(args);
    }
}
