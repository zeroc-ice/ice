// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.ACM
{
    public class Client : TestHelper
    {
        public override async Task RunAsync(string[] args)
        {
            Dictionary<string, string> properties = CreateTestProperties(ref args);
            properties["Ice.Warn.Connections"] = "0";
            await using Communicator communicator = Initialize(properties);
            AllTests.Run(this);
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Client>(args);
    }
}
