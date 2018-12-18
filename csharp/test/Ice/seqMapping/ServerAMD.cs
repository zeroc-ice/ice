// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Test;

namespace Ice
{
    namespace seqMapping
    {
        namespace AMD
        {
            public class Server : TestHelper
            {
                public override void run(string[] args)
                {
                    var initData = new InitializationData();
                    initData.typeIdNamespaces = new string[]{"Ice.seqMapping.AMD.TypeId"};
                    initData.properties = createTestProperties(ref args);
                    using(var communicator = initialize(initData))
                    {
                        communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                        var adapter = communicator.createObjectAdapter("TestAdapter");
                        adapter.add(new MyClassI(), Ice.Util.stringToIdentity("test"));
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
