// Copyright (c) ZeroC, Inc.

namespace Ice.stream
{
    public class Client : global::Test.TestHelper
    {
        public override void run(string[] args)
        {
            var initData = new InitializationData();
            initData.properties = createTestProperties(ref args);
            using (var communicator = initialize(initData))
            {
                AllTests.allTests(this);
            }
        }

        public static Task<int> Main(string[] args) =>
            global::Test.TestDriver.runTestAsync<Client>(args);
    }
}
