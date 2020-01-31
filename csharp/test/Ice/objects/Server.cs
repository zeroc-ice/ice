//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using Ice.objects.Test;

namespace Ice.objects
{
    public class Server : TestHelper
    {
        public override void run(string[] args)
        {
            var properties = createTestProperties(ref args);
            properties["Ice.Warn.Dispatch"] = "0";
            using var communicator = initialize(properties, typeIdNamespaces: new string[] { "Ice.objects.TypeId" });
            communicator.SetProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
            adapter.Add(new Initial(adapter), "initial");
            adapter.Add(new F2(), "F21");
            var uoet = new UnexpectedObjectExceptionTest();
            adapter.Add(uoet.Dispatch, "uoet");
            adapter.Activate();
            serverReady();
            communicator.WaitForShutdown();
        }

        public static int Main(string[] args) => TestDriver.runTest<Server>(args);
    }
}
