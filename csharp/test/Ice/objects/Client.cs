//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Objects
{
    public class Client : TestHelper
    {
        public override Task RunAsync(string[] args)
        {
            using var communicator = Initialize(ref args);
            var initial = AllTests.allTests(this);
            initial.shutdown();
            return Task.CompletedTask;
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Client>(args);
    }
}
