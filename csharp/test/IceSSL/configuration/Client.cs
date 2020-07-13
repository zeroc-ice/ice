//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Threading.Tasks;
using Test;

namespace ZeroC.IceSSL.Test.Configuration
{
    public class Client : TestHelper
    {
        public override Task Run(string[] args)
        {
            using var communicator = Initialize(ref args);
            if (args.Length < 1)
            {
                throw new ArgumentException("Usage: client testdir");
            }

            IServerFactoryPrx factory;
            factory = AllTests.allTests(this, args[0]);
            factory.shutdown();
            return Task.CompletedTask;
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Client>(args);
    }
}
