//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;
using Test;

namespace Ice
{
    namespace seqMapping
    {
        public class Collocated : TestHelper
        {
            public override async Task runAsync(string[] args)
            {
                var initData = new InitializationData();
                initData.typeIdNamespaces = new string[]{"Ice.seqMapping.TypeId"};
                initData.properties = createTestProperties(ref args);
                using(var communicator = initialize(initData))
                {
                    communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                    var adapter = communicator.createObjectAdapter("TestAdapter");
                    adapter.add(new MyClassI(), Ice.Util.stringToIdentity("test"));
                    //adapter.activate(); // Don't activate OA to ensure collocation is used.
                    await AllTests.allTests(this, true);
                }
            }

            public static Task<int> Main(string[] args) =>
                TestDriver.runTestAsync<Collocated>(args);
        }
    }
}
