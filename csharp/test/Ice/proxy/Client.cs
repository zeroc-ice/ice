//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Proxy
{
    public class Client : TestHelper
    {
        public override Task RunAsync(string[] args)
        {
            using var communicator = Initialize(ref args);
            AllTests.allTests(this).shutdown();
            return Task.CompletedTask;
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Client>(args);
    }
}
