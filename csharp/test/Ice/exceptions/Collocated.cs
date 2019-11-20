//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using Ice.exceptions.Test;

namespace Ice
{
    namespace exceptions
    {
        public class Collocated : TestHelper
        {
            public override void run(string[] args)
            {
                var initData = new InitializationData();
                initData.typeIdNamespaces = new string[] { "Ice.exceptions.TypeId" };
                initData.properties = createTestProperties(ref args);
                initData.properties.setProperty("Ice.Warn.Connections", "0");
                initData.properties.setProperty("Ice.Warn.Dispatch", "0");
                initData.properties.setProperty("Ice.MessageSizeMax", "10"); // 10KB max
                using (var communicator = initialize(initData))
                {
                    communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                    ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
                    adapter.Add(new ThrowerI(), Util.stringToIdentity("thrower"));
                    AllTests.allTests(this);
                }
            }

            public static int Main(string[] args)
            {
                return TestDriver.runTest<Collocated>(args);
            }
        }
    }
}
