//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using Ice.operations.AMD.Test;

namespace Ice
{
    namespace operations
    {
        namespace AMD
        {
            public class Server : TestHelper
            {
                public override void run(string[] args)
                {
                    var initData = new InitializationData();
                    initData.typeIdNamespaces = new string[] { "Ice.operations.AMD.TypeId" };
                    initData.properties = createTestProperties(ref args);

                    //
                    // We don't want connection warnings because of the timeout test.
                    //
                    initData.properties.setProperty("Ice.Warn.Connections", "0");
                    using (var communicator = initialize(initData))
                    {
                        communicator.Properties.setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
                        adapter.Add(new MyDerivedClassI(), "test");
                        adapter.Activate();
                        serverReady();
                        communicator.waitForShutdown();
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
