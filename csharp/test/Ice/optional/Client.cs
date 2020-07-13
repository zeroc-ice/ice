//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Optional
{
    public class Client : TestHelper
    {
        public override Task RunAsync(string[] args)
        {
            _ = Initialize(ref args);
            AllTests.Run(this).Shutdown();
            return Task.CompletedTask;
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Client>(args);
    }
}
