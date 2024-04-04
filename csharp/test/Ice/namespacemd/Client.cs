// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice
{
    namespace namespacemd
    {
        public class Client : TestHelper
        {
            override public void run(string[] args)
            {
                var properties = createTestProperties(ref args);
                properties.setProperty("Ice.Warn.Dispatch", "0");
                using (var communicator = initialize(properties))
                {
                    var initial = AllTests.allTests(this);
                    initial.shutdown();
                }
            }

            public static Task<int> Main(string[] args) =>
                TestDriver.runTestAsync<Client>(args);
        }
    }
}
