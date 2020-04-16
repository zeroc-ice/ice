//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace Ice.serialize
{
    public class Client : TestHelper
    {
        public override void Run(string[] args)
        {
            using var communicator = Initialize(CreateTestProperties(ref args),
                typeIdNamespaces: new string[] { "Ice.serialize.TypeId" });
            AllTests.allTests(this);
        }

        public static int Main(string[] args) => TestDriver.RunTest<Client>(args);
    }
}
