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
                var initData = new InitializationData();
                initData.typeIdNamespaces = new string[]{"Ice.serialize.TypeId"};
                initData.properties = createTestProperties(ref args);
                using(var communicator = initialize(initData))
                {
                    AllTests.allTests(this);
                }
            }

            public static int Main(string[] args)
            {
                return TestDriver.runTest<Client>(args);
            }
        }
    }
}
