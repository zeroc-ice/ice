//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using Ice.optional.Test;

namespace Ice
{
    namespace optional
    {
        public class Server : global::Test.TestHelper
        {
            public override void run(string[] args)
            {
                var initData = new InitializationData();
                initData.typeIdNamespaces = new string[] { "Ice.optional.TypeId" };
                initData.properties = createTestProperties(ref args);
                using (var communicator = initialize(initData))
                {
                    communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                    var adapter = communicator.createObjectAdapter("TestAdapter");
                    adapter.Add(new InitialI(), Util.stringToIdentity("initial"));
                    adapter.activate();
                    serverReady();
                    communicator.waitForShutdown();
                }
            }

            public static int Main(string[] args)
            {
                return TestDriver.runTest<Server>(args);
            }
        }
    }
}
