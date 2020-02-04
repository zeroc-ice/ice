//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace Ice.adapterDeactivation
{
    public class Server : TestHelper
    {
        public override void run(string[] args)
        {
            using (var communicator = initialize(ref args))
            {
                communicator.SetProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                var adapter = communicator.CreateObjectAdapter("TestAdapter");
                adapter.AddDefault(new Servant());
                adapter.Activate();
                serverReady();
                adapter.WaitForDeactivate();
            }
        }

        public static int Main(string[] args) => TestDriver.runTest<Server>(args);
    }
}
