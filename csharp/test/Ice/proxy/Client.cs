//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace Ice.proxy
{
    public class Client : TestHelper
    {
        public override void Run(string[] args)
        {
            using var communicator = Initialize(ref args);
            AllTests.allTests(this).shutdown();
        }

        public static int Main(string[] args) => TestDriver.RunTest<Client>(args);
    }
}
