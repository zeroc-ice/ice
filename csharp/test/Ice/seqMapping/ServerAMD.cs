//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using Ice.seqMapping.AMD.Test;

namespace Ice
{
    namespace seqMapping
    {
        namespace AMD
        {
            public class Server : TestHelper
            {
                public override void run(string[] args)
                {
                    var initData = new InitializationData();
                    initData.typeIdNamespaces = new string[] { "Ice.seqMapping.AMD.TypeId" };
                    initData.properties = createTestProperties(ref args);
                    using (var communicator = initialize(initData))
                    {
                        communicator.Properties.setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                        var adapter = communicator.createObjectAdapter("TestAdapter");
                        adapter.Add(new MyClassI(), "test");
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
