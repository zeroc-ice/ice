//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Linq;
using System.Collections.Generic;
using Test;
using System.Threading.Tasks;

namespace ZeroC.Ice.Test.FaultTolerance
{
    public class Client : TestHelper
    {
        public override Task RunAsync(string[] args)
        {
            Dictionary<string, string> properties = CreateTestProperties(ref args);
            properties["Ice.Warn.Connections"] = "0";
            using Communicator communicator = Initialize(properties);
            var ports = args.Select(v => int.Parse(v)).ToList();
            if (ports.Count == 0)
            {
                throw new ArgumentException("Client: no ports specified");
            }
            AllTests.allTests(this, ports);
            return Task.CompletedTask;
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Client>(args);
    }
}
