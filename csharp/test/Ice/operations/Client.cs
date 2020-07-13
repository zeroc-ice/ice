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
        public override Task RunAsync(string[] args)
        {
            using var communicator = Initialize(ref args);
            var myClass = AllTests.allTests(this);

            Console.Out.Write("testing server shutdown... ");
            Console.Out.Flush();
            myClass.shutdown();
            try
            {
                myClass.IcePing();
                Assert(false);
            }
            catch (Exception)
            {
                 Console.Out.WriteLine("ok");
            }
            return Task.CompletedTask;
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Client>(args);
    }
}
