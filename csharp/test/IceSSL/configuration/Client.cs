// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Test;

namespace ZeroC.IceSSL.Test.Configuration
{
    public class Client : TestHelper
    {
        public override async Task RunAsync(string[] args)
        {
            Dictionary<string, string> properties = CreateTestProperties(ref args);
            // TODO: remove this when Never is the communicator default
            properties["Ice.Default.PreferNonSecure"] = "Never";
            await using Ice.Communicator communicator = Initialize(properties);
            if (args.Length < 1)
            {
                throw new ArgumentException("Usage: client testdir");
            }

            IServerFactoryPrx factory;
            factory = await AllTests.RunAsync(this, args[0]);
            await factory.ShutdownAsync();
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Client>(args);
    }
}
