//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace Ice.location
{
    public class Client : TestHelper
    {
        public override void run(string[] args)
        {
            var properties = createTestProperties(ref args);
            properties["Ice.Default.Locator"] = $"locator:{getTestEndpoint(properties, 0)}";
            using var communicator = initialize(properties);
            AllTests.allTests(this);
        }

        public static int Main(string[] args) => TestDriver.runTest<Client>(args);
    }
}
