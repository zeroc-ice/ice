//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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

            public static int Main(string[] args)
            {
                return TestDriver.runTest<Client>(args);
            }
        }
    }
}
