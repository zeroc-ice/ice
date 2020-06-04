//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace ZeroC.Ice.Test.Proxy
{
    public class Server : TestHelper
    {
        public override void Run(string[] args)
        {
            var properties = CreateTestProperties(ref args);
            //
            // We don't want connection warnings because of the timeout test.
            //
            properties["Ice.Warn.Connections"] = "0";
            properties["Ice.Warn.Dispatch"] = "0";

            using var communicator = Initialize(properties);
            communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0));
            var adapter = communicator.CreateObjectAdapter("TestAdapter");
            adapter.Add("test", new MyDerivedClass());
            adapter.Activate();
            ServerReady();
            communicator.WaitForShutdown();
        }

        public static int Main(string[] args) => TestDriver.RunTest<Server>(args);
    }
}
