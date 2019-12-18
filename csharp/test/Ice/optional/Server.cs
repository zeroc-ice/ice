//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using Ice.optional.Test;

namespace Ice
{
    namespace optional
    {
        public class Server : global::Test.TestHelper
        {
            public override void run(string[] args)
            {
                using var communicator = initialize(createTestProperties(ref args),
                    typeIdNamespaces: new string[] { "Ice.optional.TypeId" });
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
