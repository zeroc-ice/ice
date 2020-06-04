//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace ZeroC.Ice.Test.Location
{
    public class Client : TestHelper
    {
        public override void Run(string[] args)
        {
            System.Collections.Generic.Dictionary<string, string> properties = CreateTestProperties(ref args);
            properties["Ice.Default.Locator"] = $"locator:{GetTestEndpoint(properties, 0)}";
            using Communicator communicator = Initialize(properties);
            AllTests.allTests(this);
        }

        public static int Main(string[] args) => TestDriver.RunTest<Client>(args);
    }
}
