// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice
{
    namespace operations
    {
        public class Collocated : TestHelper
        {
            public override async Task runAsync(string[] args)
            {
                var initData = new InitializationData();
                initData.properties = createTestProperties(ref args);
                initData.properties.setProperty("Ice.ThreadPool.Client.Size", "2");
                initData.properties.setProperty("Ice.ThreadPool.Client.SizeWarn", "0");
                initData.properties.setProperty("Ice.BatchAutoFlushSize", "100");
                using var communicator = initialize(initData);
                communicator.getProperties().setProperty("TestAdapter.AdapterId", "test");
                communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
                ObjectPrx prx = adapter.add(new MyDerivedClassI(), Ice.Util.stringToIdentity("test"));
                // Don't activate OA to ensure collocation is used.

                if (prx.ice_getConnection() != null)
                {
                    throw new System.Exception();
                }

                await AllTests.allTests(this);
            }

            public static Task<int> Main(string[] args) =>
                TestDriver.runTestAsync<Collocated>(args);
        }
    }
}
