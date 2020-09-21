// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.DefaultValue
{
    public class Client : TestHelper
    {
        public override Task RunAsync(string[] args)
        {
            AllTests.Run(this);
            return Task.CompletedTask;
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Client>(args);
    }
}
