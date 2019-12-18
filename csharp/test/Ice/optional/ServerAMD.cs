//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using Ice.optional.AMD.Test;

namespace Ice
{
    namespace optional
    {
        namespace AMD
        {
            public class Server : TestHelper
            {
                public override void run(string[] args)
                {
                    using var communicator = initialize(createTestProperties(ref args),
                        typeIdNamespaces: new string[] { "Ice.optional.AMD.TypeId" });
                    communicator.SetProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                    var adapter = communicator.createObjectAdapter("TestAdapter");
                    adapter.Add(new InitialI(), "initial");
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
