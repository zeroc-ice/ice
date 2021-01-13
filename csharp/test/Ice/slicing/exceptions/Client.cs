// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Slicing.Exceptions
{
    public class Client : TestHelper
    {
        public override Task RunAsync(string[] args) => AllTests.RunAsync(this);

        public static async Task<int> Main(string[] args)
        {
            await using var communicator = CreateCommunicator(ref args);
            await communicator.ActivateAsync();
            return await RunTestAsync<Client>(communicator, args);
        }
    }
}
