//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace ZeroC.Ice.Test.NetworkProxy
{
    public class Client : TestHelper
    {
        public override void Run(string[] args)
        {
            using var communicator = Initialize(ref args);
            AllTests.allTests(this);
        }

        public static int Main(string[] args) => TestDriver.RunTest<Client>(args);
    }
}
