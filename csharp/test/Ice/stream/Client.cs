//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;

namespace Ice
{
    namespace stream
    {
        public class Client : global::Test.TestHelper
        {
            public override void run(string[] args)
            {
                var initData = new InitializationData();
                initData.typeIdNamespaces = new string[]{"Ice.stream.TypeId"};
                initData.properties = createTestProperties(ref args);
                using(var communicator = initialize(initData))
                {
                    AllTests.allTests(this);
                }
            }

            public static Task<int> Main(string[] args) =>
                global::Test.TestDriver.runTestAsync<Client>(args);
        }
    }
}
