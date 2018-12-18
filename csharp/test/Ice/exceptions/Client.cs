// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

using Test;

namespace Ice
{
    namespace exceptions
    {
        public class Client : TestHelper
        {
            public override void run(string[] args)
            {
                var initData = new InitializationData();
                initData.typeIdNamespaces = new string[]{"Ice.exceptions.TypeId"};
                initData.properties = createTestProperties(ref args);
                initData.properties.setProperty("Ice.Warn.Connections", "0");
                initData.properties.setProperty("Ice.MessageSizeMax", "10"); // 10KB max
                using(var communicator = initialize(initData))
                {
                    communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                    var thrower = AllTests.allTests(this);
                    thrower.shutdown();
                }
            }

            public static int Main(string[] args)
            {
                return TestDriver.runTest<Client>(args);
            }
        }
    }
}
