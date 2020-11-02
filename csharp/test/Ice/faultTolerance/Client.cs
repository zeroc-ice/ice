// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.FaultTolerance
{
    public class Client : TestHelper
    {
        public override async Task RunAsync(string[] args)
        {
            Dictionary<string, string> properties = CreateTestProperties(ref args);
            properties["Ice.Warn.Connections"] = "0";
            await using Communicator communicator = Initialize(properties);
            var ports = args.Select(v => int.Parse(v)).ToList();
            if (ports.Count == 0)
            {
                throw new ArgumentException("Client: no ports specified");
            }
            AllTests.Run(this, ports);
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Client>(args);
    }
}
