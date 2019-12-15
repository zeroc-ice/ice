//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using Ice.operations.Test;

namespace Ice
{
    namespace operations
    {
        public class Collocated : TestHelper
        {
            public override void run(string[] args)
            {
                var initData = new InitializationData();
                initData.typeIdNamespaces = new string[] { "Ice.operations.TypeId" };
                initData.properties = createTestProperties(ref args);
                initData.properties.setProperty("Ice.ThreadPool.Client.Size", "2");
                initData.properties.setProperty("Ice.ThreadPool.Client.SizeWarn", "0");

                using (var communicator = initialize(initData))
                {
                    communicator.Properties.setProperty("TestAdapter.AdapterId", "test");
                    communicator.Properties.setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                    ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
                    var prx = adapter.Add(new MyDerivedClassI(), "test");
                    //adapter.activate(); // Don't activate OA to ensure collocation is used.

                    if (prx.GetConnection() != null)
                    {
                        throw new System.Exception();
                    }

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
