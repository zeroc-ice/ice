//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using Ice.seqMapping.Test;

namespace Ice
{
    namespace seqMapping
    {
        public class Collocated : TestHelper
        {
            public override void run(string[] args)
            {
                var initData = new InitializationData();
                initData.typeIdNamespaces = new string[] { "Ice.seqMapping.TypeId" };
                initData.properties = createTestProperties(ref args);
                using (var communicator = initialize(initData))
                {
                    communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                    var adapter = communicator.createObjectAdapter("TestAdapter");
                    adapter.Add(new MyClassI(), "test");
                    //adapter.activate(); // Don't activate OA to ensure collocation is used.
                    AllTests.allTests(this, true);
                }
            }

            public static int Main(string[] args)
            {
                return TestDriver.runTest<Collocated>(args);
            }
        }
    }
}
