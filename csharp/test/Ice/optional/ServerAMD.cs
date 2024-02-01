//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;
using Test;

namespace Ice
{
    namespace optional
    {
        namespace AMD
        {
            public class Server : TestHelper
            {
                public override void run(string[] args)
                {
                    var initData = new InitializationData();
                    initData.typeIdNamespaces = new string[]{"Ice.optional.AMD.TypeId"};
                    initData.properties = createTestProperties(ref args);
                    using(var communicator = initialize(initData))
                    {
                        communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                        var adapter = communicator.createObjectAdapter("TestAdapter");
                        adapter.add(new InitialI(), Ice.Util.stringToIdentity("initial"));
                        adapter.activate();
                        serverReady();
                        communicator.waitForShutdown();
                    }
                }

                public static Task<int> Main(string[] args) =>
                    TestDriver.runTestAsync<Server>(args);
            }
        }
    }
}
