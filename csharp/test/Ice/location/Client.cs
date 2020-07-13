//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Location
{
    public class Client : TestHelper
    {
        public override Task RunAsync(string[] args)
        {
            System.Collections.Generic.Dictionary<string, string> properties = CreateTestProperties(ref args);
            properties["Ice.Default.Locator"] = $"locator:{GetTestEndpoint(properties, 0)}";
            using Communicator communicator = Initialize(properties);
            AllTests.allTests(this);
            return Task.CompletedTask;
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Client>(args);
    }
}
