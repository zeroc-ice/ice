//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Operations
{
    public class Client : TestHelper
    {
        public override async Task RunAsync(string[] args)
        {
            await using Communicator communicator = Initialize(ref args);
            var myClass = AllTests.allTests(this);

            Console.Out.Write("testing server shutdown... ");
            Console.Out.Flush();
            await myClass.ShutdownAsync();
            try
            {
                await myClass.IcePingAsync();
                Assert(false);
            }
            catch
            {
                 Console.Out.WriteLine("ok");
            }
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Client>(args);
    }
}
