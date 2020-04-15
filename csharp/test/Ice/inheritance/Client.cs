//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace Ice.inheritance
{
    public class Client : TestHelper
    {
        public override void Run(string[] args)
        {
            using Communicator communicator = Initialize(ref args);
            AllTests.allTests(this).shutdown();
        }

        public static int Main(string[] args) => TestDriver.RunTest<Client>(args);
    }
}
