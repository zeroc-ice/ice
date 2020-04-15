//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace Ice.invoke
{
    public class Client : TestHelper
    {
        public override void Run(string[] args)
        {
            using Communicator communicator = Initialize(ref args);
            Test.IMyClassPrx myClass = AllTests.allTests(this);
            myClass.shutdown();
        }

        public static int Main(string[] args) => TestDriver.RunTest<Client>(args);
    }
}
