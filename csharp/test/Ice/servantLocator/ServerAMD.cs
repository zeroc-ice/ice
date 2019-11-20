//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using Ice.servantLocator.AMD.Test;

namespace Ice
{
    namespace servantLocator
    {
        namespace AMD
        {
            public class Server : TestHelper
            {
                public override void run(string[] args)
                {
                    var initData = new InitializationData();
                    initData.typeIdNamespaces = new string[] { "Ice.servantLocator.AMD.TypeId" };
                    initData.properties = createTestProperties(ref args);
                    using (var communicator = initialize(initData))
                    {
                        communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                        communicator.getProperties().setProperty("Ice.Warn.Dispatch", "0");

                        var adapter = communicator.createObjectAdapter("TestAdapter");
                        adapter.AddServantLocator(new ServantLocatorI("category"), "category");
                        adapter.AddServantLocator(new ServantLocatorI(""), "");
                        adapter.Add(new TestI(), "asm");
                        adapter.Add(new TestActivationI(), "test/activation");
                        adapter.Activate();
                        serverReady();
                        adapter.WaitForDeactivate();
                    }
                }

                public static int Main(string[] args)
                {
                    return TestDriver.runTest<Server>(args);
                }
            }
        }
    }
}
