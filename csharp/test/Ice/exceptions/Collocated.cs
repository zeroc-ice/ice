//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using Ice.exceptions.Test;

namespace Ice
{
    namespace exceptions
    {
        public class Collocated : TestHelper
        {
            public override void run(string[] args)
            {
                var typeIdNamespaces = new string[] { "Ice.exceptions.TypeId" };
                var properties = createTestProperties(ref args);
                properties["Ice.Warn.Connections"] = "0";
                properties["Ice.Warn.Dispatch"] = "0";
                properties["Ice.MessageSizeMax"] = "10"; // 10KB max
                using var communicator = initialize(properties, typeIdNamespaces: typeIdNamespaces);
                communicator.SetProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
                adapter.Add(new ThrowerI(), "thrower");
                AllTests.allTests(this);
            }

            public static int Main(string[] args)
            {
                return TestDriver.runTest<Collocated>(args);
            }
        }
    }
}
