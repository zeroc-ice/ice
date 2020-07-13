//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Inheritance
{
    public class Client : TestHelper
    {
        public override Task Run(string[] args)
        {
            using Communicator communicator = Initialize(ref args);
            AllTests.allTests(this).shutdown();
            return Task.CompletedTask;
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Client>(args);
    }
}
