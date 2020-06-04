//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace ZeroC.Ice.Test.Info
{
    public class Server : TestHelper
    {
        public override void Run(string[] args)
        {
            using var communicator = Initialize(ref args);
            communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0) + ":" + GetTestEndpoint(0, "udp"));
            ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
            adapter.Add("test", new TestIntf());
            adapter.Activate();
            ServerReady();
            communicator.WaitForShutdown();
        }

        public static int Main(string[] args) => TestDriver.RunTest<Server>(args);
    }
}
