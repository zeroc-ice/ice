//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace ZeroC.Ice.Test.Echo
{
    public class Server : TestHelper
    {
        private class Echo : IEcho
        {
            public void shutdown(Current current) => current.Adapter.Communicator.Shutdown();
        }

        public override void Run(string[] args)
        {
            using Communicator communicator = Initialize(ref args);
            communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0));
            ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
            var blob = new BlobjectI();
            adapter.AddDefault(blob);
            adapter.Add("__echo", new Echo());
            adapter.Activate();
            communicator.WaitForShutdown();
        }

        public static int Main(string[] args) => TestDriver.RunTest<Server>(args);
    }
}
