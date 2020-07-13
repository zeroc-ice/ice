//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.SeqMapping
{
    public class Client : TestHelper
    {
        public override Task Run(string[] args)
        {
            using var communicator = Initialize(ref args);
            var myClass = AllTests.allTests(this, false);
            Console.Out.Write("shutting down server... ");
            Console.Out.Flush();
            myClass.shutdown();
            Console.Out.WriteLine("ok");
            return Task.CompletedTask;
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Client>(args);
    }
}
