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
                    using var communicator = initialize(createTestProperties(ref args),
                        typeIdNamespaces: new string[] { "Ice.seqMapping.AMD.TypeId" });
                    communicator.SetProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                    var adapter = communicator.createObjectAdapter("TestAdapter");
                    adapter.Add(new MyClassI(), "test");
                    adapter.Activate();
                    serverReady();
                    communicator.waitForShutdown();
                }

                public static int Main(string[] args)
                {
                    return TestDriver.runTest<Server>(args);
                }
            }
        }
    }
}
