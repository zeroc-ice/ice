// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.DictMapping
{
    public class Client : TestHelper
    {
        public override async Task RunAsync(string[] args)
        {
            await using Communicator communicator = Initialize(ref args);
            IMyClassPrx myClass = AllTests.Run(this, false);
            Output.Write("shutting down server... ");
            Output.Flush();
            await myClass.ShutdownAsync();
            Output.WriteLine("ok");
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Client>(args);
    }
}
