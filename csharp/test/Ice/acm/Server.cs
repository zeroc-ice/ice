// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Test;

namespace Ice
{
    namespace acm
    {
        public class Server : TestHelper
        {
            public override void run(string[] args)
            {
                var properties = createTestProperties(ref args);
                properties.setProperty("Ice.Warn.Connections", "0");
                properties.setProperty("Ice.ACM.Timeout", "1");
                using(var communicator = initialize(properties))
                {
                    communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                    communicator.getProperties().setProperty("TestAdapter.ACM.Timeout", "0");
                    var adapter = communicator.createObjectAdapter("TestAdapter");
                    var id = Util.stringToIdentity("communicator");
                    adapter.add(new RemoteCommunicatorI(), id);
                    adapter.activate();
                    serverReady();
                    communicator.getProperties().setProperty("Ice.PrintAdapterReady", "0");
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
