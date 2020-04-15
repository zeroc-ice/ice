//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace Ice
{
    namespace enums
    {
        public class Client : TestHelper
        {
            public override void Run(string[] args)
            {
                using Communicator communicator = Initialize(ref args);
                Test.ITestIntfPrx proxy = AllTests.allTests(this);
                proxy.shutdown();
            }

            public static int Main(string[] args) => TestDriver.RunTest<Client>(args);
        }
    }
}
