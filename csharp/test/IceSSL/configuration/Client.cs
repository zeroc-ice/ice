//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using Test;

namespace ZeroC.IceSSL.Test.Configuration
{
    public class Client : TestHelper
    {
        public override void Run(string[] args)
        {
            using var communicator = Initialize(ref args);
            if (args.Length < 1)
            {
                throw new ArgumentException("Usage: client testdir");
            }

            IServerFactoryPrx factory;
            factory = AllTests.allTests(this, args[0]);
            factory.shutdown();
        }

        public static int Main(string[] args) => TestDriver.RunTest<Client>(args);
    }
}
