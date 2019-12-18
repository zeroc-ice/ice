//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace Ice
{
    namespace serialize
    {
        public class Client : TestHelper
        {
            public override void run(string[] args)
            {
                using var communicator = initialize(createTestProperties(ref args),
                    typeIdNamespaces: new string[] { "Ice.serialize.TypeId" });
                AllTests.allTests(this);
            }

            public static int Main(string[] args)
            {
                return TestDriver.runTest<Client>(args);
            }
        }
    }
}
