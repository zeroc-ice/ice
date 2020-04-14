//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace Ice.namespacemd
{
    public class Client : TestHelper
    {
        override public void Run(string[] args)
        {
            var properties = CreateTestProperties(ref args);
            properties["Ice.Warn.Dispatch"] = "0";
            using var communicator = Initialize(properties);
            var initial = AllTests.allTests(this);
            initial.shutdown();
        }

        public static int Main(string[] args) => TestDriver.RunTest<Client>(args);
    }
}
