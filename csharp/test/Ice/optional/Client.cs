//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace Ice.optional
{
    public class Client : TestHelper
    {
        public override void Run(string[] args)
        {
            using var communicator = Initialize(CreateTestProperties(ref args),
                typeIdNamespaces: new string[] { "Ice.optional.TypeId" });
            AllTests.allTests(this).shutdown();
        }

        public static int Main(string[] args) => TestDriver.RunTest<Client>(args);
    }
}
