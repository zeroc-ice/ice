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
    namespace packagemd
    {
        public class Server : TestHelper
        {
            public override void run(string[] args)
            {
                var properties = createTestProperties(ref args);
                properties.setProperty("Ice.Package.Test", "Ice.packagemd");
                properties.setProperty("Ice.Package.Test1", "Ice.packagemd");
                using(var communicator = initialize(properties))
                {
                    properties.setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                    var adapter = communicator.createObjectAdapter("TestAdapter");
                    adapter.add(new InitialI(), Ice.Util.stringToIdentity("initial"));
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
