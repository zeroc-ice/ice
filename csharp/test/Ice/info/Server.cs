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
    namespace info
    {
        public class Server : TestHelper
        {
            public override void run(string[] args)
            {
                var properties = createTestProperties(ref args);
                properties.setProperty("Ice.Package.Test", "Ice.info");
                using(var communicator = initialize(properties))
                {
                    communicator.getProperties().setProperty("TestAdapter.Endpoints",
                                                             getTestEndpoint(0) + ":" + getTestEndpoint(0, "udp"));
                    Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
                    adapter.add(new TestI(), Ice.Util.stringToIdentity("test"));
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
