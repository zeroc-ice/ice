// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Threading
{
    public class Client : TestHelper
    {
        public override async Task RunAsync(string[] args)
        {
            try
            {
                await AllTests.RunAsync(this, false);
            }
            catch (TestFailedException ex)
            {
                Output.WriteLine($"test failed: {ex.Reason}");
                Assert(false);
                throw;
            }
        }

        public static async Task<int> Main(string[] args)
        {
            await using var communicator = CreateCommunicator(ref args);
            await communicator.ActivateAsync();
            return await RunTestAsync<Client>(communicator, args);
        }
    }
}
