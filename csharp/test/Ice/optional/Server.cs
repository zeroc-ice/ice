//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using Ice.optional.Test;

namespace Ice.optional
{
    public class Server : TestHelper
    {
        public override void Run(string[] args)
        {
            using var communicator = Initialize(CreateTestProperties(ref args),
                typeIdNamespaces: new string[] { "Ice.optional.TypeId" });
            communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0));
            var adapter = communicator.CreateObjectAdapter("TestAdapter");
            adapter.Add("initial", new Initial());
            adapter.Activate();
            ServerReady();
            communicator.WaitForShutdown();
        }

        public static int Main(string[] args) => TestDriver.RunTest<Server>(args);
    }
}
