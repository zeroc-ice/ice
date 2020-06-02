//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using Test;

namespace ZeroC.Ice.Test.SeqMapping
{
    public class Client : TestHelper
    {
        public override void Run(string[] args)
        {
            using var communicator = Initialize(ref args);
            var myClass = AllTests.allTests(this, false);
            Console.Out.Write("shutting down server... ");
            Console.Out.Flush();
            myClass.shutdown();
            Console.Out.WriteLine("ok");
        }

        public static int Main(string[] args) => TestDriver.RunTest<Client>(args);
    }
}
