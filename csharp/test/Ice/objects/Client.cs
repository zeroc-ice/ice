//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace Ice
{
    namespace objects
    {
        public class Client : TestHelper
        {
            public override void run(string[] args)
            {
                using var communicator = initialize(createTestProperties(ref args),
                                                    typeIdNamespaces: new string[] { "Ice.objects.TypeId" });
                var initial = Test.AllTests.allTests(this);
                initial.shutdown();
            }

            public static int Main(string[] args)
            {
                return TestDriver.runTest<Client>(args);
            }
        }
    }
}
