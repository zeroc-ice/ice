//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace Ice.DefaultServant
{
    public class Client : TestHelper
    {
        public override void run(string[] args)
        {
            using var communicator = initialize(ref args);
            AllTests.Run(this);
        }

        public static int Main(string[] args) => TestDriver.runTest<Client>(args);
    }
}
