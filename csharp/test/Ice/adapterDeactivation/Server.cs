//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace ZeroC.Ice.Test.AdapterDeactivation
{
    public class Server : TestHelper
    {
        public override void Run(string[] args)
        {
            using Communicator communicator = Initialize(ref args);
            communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0));
            ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
            adapter.AddDefault(new Servant());
            adapter.Activate();
            ServerReady();
            adapter.WaitForDeactivate();
        }

        public static int Main(string[] args) => TestDriver.RunTest<Server>(args);
    }
}
