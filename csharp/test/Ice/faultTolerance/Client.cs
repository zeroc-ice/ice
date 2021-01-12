// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.FaultTolerance
{
    public class Client : TestHelper
    {
        public override Task RunAsync(string[] args)
        {
            var ports = args.Select(v => int.Parse(v)).ToList();
            if (ports.Count == 0)
            {
                throw new ArgumentException("Client: no ports specified");
            }
            return AllTests.RunAsync(this, ports);
        }

        public static async Task<int> Main(string[] args)
        {
            Dictionary<string, string> properties = CreateTestProperties(ref args);
            properties["Ice.Warn.Connections"] = "0";

            await using var communicator = CreateCommunicator(properties);
            await communicator.ActivateAsync();
            return await RunTestAsync<Client>(communicator, args);
        }
    }
}
