//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace Ice
{
    namespace operations
    {
        namespace AMD
        {
            namespace tie
            {
                public class Server : TestHelper
                {
                    public override void run(string[] args)
                    {
                        var initData = new InitializationData();
                        initData.typeIdNamespaces = new string[]{"Ice.operations.AMD.TypeId"};

                        initData.properties = createTestProperties(ref args);
                        //
                        // Its possible to have batch oneway requests dispatched
                        // after the adapter is deactivated due to thread
                        // scheduling so we supress this warning.
                        //
                        initData.properties.setProperty("Ice.Warn.Dispatch", "0");
                        //
                        // We don't want connection warnings because of the timeout test.
                        //
                        initData.properties.setProperty("Ice.Warn.Connections", "0");
                        using(var communicator = initialize(initData))
                        {
                            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
                            adapter.add(new MyDerivedClassI(), Ice.Util.stringToIdentity("test"));
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
    }
}
