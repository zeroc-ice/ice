//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using Test;

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
                    communicator.getProperties().setProperty("TestAdapter.AdapterId", "test");
                    communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                    Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
                    Ice.ObjectPrx prx = adapter.add(new MyDerivedClassI(), Ice.Util.stringToIdentity("test"));
                    adapter.add(new BI(), Ice.Util.stringToIdentity("b"));
                    //adapter.activate(); // Don't activate OA to ensure collocation is used.

                    if (prx.ice_getConnection() != null)
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
