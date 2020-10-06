// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Threading
{
    public class Client : TestHelper
    {
        public override async Task RunAsync(string[] args)
        {
            await using Communicator communicator = Initialize(ref args);
            try
            {
                ITestIntfPrx server = await AllTests.Run(this, false);
                await server.ShutdownAsync();
            }
            catch (TestFailedException ex)
            {
                Output.WriteLine($"test failed: {ex.Reason}");
                Assert(false);
                throw;
            }
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Client>(args);
    }
}
