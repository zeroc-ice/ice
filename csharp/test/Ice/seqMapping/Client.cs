//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using Test;

namespace Ice
{
    namespace seqMapping
    {
        public class Client : TestHelper
        {
            public override void run(string[] args)
            {
                using var communicator = initialize(createTestProperties(ref args),
                    typeIdNamespaces: new string[] { "Ice.seqMapping.TypeId" });
                var myClass = AllTests.allTests(this, false);
                Console.Out.Write("shutting down server... ");
                Console.Out.Flush();
                myClass.shutdown();
                Console.Out.WriteLine("ok");
            }

            public static int Main(string[] args)
            {
                return TestDriver.runTest<Client>(args);
            }
        }
    }
}
