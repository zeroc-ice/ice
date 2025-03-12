// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice.seqMapping
{
    public class Collocated : TestHelper
    {
        public override async Task runAsync(string[] args)
        {
            var initData = new InitializationData();
            initData.properties = createTestProperties(ref args);
            using var communicator = initialize(initData);
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            var adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new MyClassI(), Ice.Util.stringToIdentity("test"));
            // Don't activate OA to ensure collocation is used.
            await AllTests.allTests(this, true);
        }

        public static Task<int> Main(string[] args) =>
            TestDriver.runTestAsync<Collocated>(args);
    }
}
