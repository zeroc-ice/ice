//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace Ice
{
    namespace exceptions
    {
        public class Client : TestHelper
        {
            public override void run(string[] args)
            {
                var typeIdNamespaces = new string[] { "Ice.exceptions.TypeId" };
                var properties = createTestProperties(ref args);
                properties["Ice.Warn.Connections"] = "0";
                properties["Ice.MessageSizeMax"] = "10"; // 10KB max
                using var communicator = initialize(properties, typeIdNamespaces: typeIdNamespaces);
                communicator.SetProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                var thrower = AllTests.allTests(this);
                thrower.shutdown();
            }

            public static int Main(string[] args)
            {
                return TestDriver.runTest<Client>(args);
            }
        }
    }
}
