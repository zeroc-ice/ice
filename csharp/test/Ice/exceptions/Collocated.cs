//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;
using Test;

namespace Ice
{
    namespace exceptions
    {
        public class Collocated : TestHelper
        {
            public override async Task runAsync(string[] args)
            {
                var initData = new InitializationData();
                initData.typeIdNamespaces = new string[]{"Ice.exceptions.TypeId"};
                initData.properties = createTestProperties(ref args);
                initData.properties.setProperty("Ice.Warn.Connections", "0");
                initData.properties.setProperty("Ice.Warn.Dispatch", "0");
                initData.properties.setProperty("Ice.MessageSizeMax", "10"); // 10KB max
                using(var communicator = initialize(initData))
                {
                    communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                    Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
                    Ice.Object obj = new ThrowerI();
                    adapter.add(obj, Ice.Util.stringToIdentity("thrower"));
                    await AllTests.allTests(this);
                }
            }

            public static Task<int> Main(string[] args) =>
                TestDriver.runTestAsync<Collocated>(args);
        }
    }
}
