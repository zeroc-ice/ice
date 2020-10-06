// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Scope
{
    public class Client : TestHelper
    {
        public override async Task RunAsync(string[] args)
        {
            await using var communicator = Initialize(ref args);
            Output.Write("test using same type name in different Slice modules... ");
            Output.Flush();
            AllTests.Run(this);
            Output.WriteLine("ok");
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Client>(args);
    }
}
