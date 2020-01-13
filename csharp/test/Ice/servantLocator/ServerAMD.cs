//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using Ice.servantLocator.AMD.Test;

namespace Ice.servantLocator.AMD
{
    public class Server : TestHelper
    {
        public override void run(string[] args)
        {
            using var communicator = initialize(createTestProperties(ref args),
                typeIdNamespaces: new string[] { "Ice.servantLocator.AMD.TypeId" });
            communicator.SetProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            communicator.SetProperty("Ice.Warn.Dispatch", "0");

            var adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.AddServantLocator(new ServantLocator("category"), "category");
            adapter.AddServantLocator(new ServantLocator(""), "");
            adapter.Add(new TestIntf(), "asm");
            adapter.Add(new TestActivation(), "test/activation");
            adapter.Activate();
            serverReady();
            adapter.WaitForDeactivate();
        }

        public static int Main(string[] args) => TestDriver.runTest<Server>(args);
    }
}
