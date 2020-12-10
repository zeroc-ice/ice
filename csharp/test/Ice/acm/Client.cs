// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading.Tasks;
using TestNew;

namespace ZeroC.Ice.Test.ACM
{
    public class Client : TestHelper
    {
        public override async Task RunAsync(string[] args) => await AllTests.RunAsync(this);

        public static async Task<int> Main(string[] args)
        {
            await using var communicator = CreateCommunicator(ref args);
            return await RunTestAsync<Client>(communicator, args);
        }
    }
}
