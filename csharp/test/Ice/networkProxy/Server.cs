//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace ZeroC.Ice.Test.NetworkProxy
{
    public class Server : TestHelper
    {
        public class TestIntf : ITestIntf
        {
            public void shutdown(ZeroC.Ice.Current current) => current.Adapter.Communicator.Shutdown();
        }

        public override void Run(string[] args)
        {
            using var communicator = Initialize(ref args);
            communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0));
            ZeroC.Ice.ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
            adapter.Add("test", new TestIntf());
            adapter.Activate();

            communicator.WaitForShutdown();
        }

        public static int Main(string[] args) => TestDriver.RunTest<Server>(args);
    }
}
