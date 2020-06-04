//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace ZeroC.Ice.Test.Slicing.Exceptions
{
    public class Client : TestHelper
    {
        public override void Run(string[] args)
        {
            using var communicator = Initialize(ref args);
            ITestIntfPrx test = AllTests.allTests(this);
            test.shutdown();
        }

        public static int Main(string[] args) => TestDriver.RunTest<Client>(args);
    }
}
