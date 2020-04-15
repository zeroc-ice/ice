//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace Ice.DefaultServant
{
    public class Client : TestHelper
    {
        public override void Run(string[] args)
        {
            using Communicator communicator = Initialize(ref args);
            AllTests.Run(this);
        }

        public static int Main(string[] args) => TestDriver.RunTest<Client>(args);
    }
}
