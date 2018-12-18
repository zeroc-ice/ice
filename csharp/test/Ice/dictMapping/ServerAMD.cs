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
    namespace dictMapping
    {
        namespace AMD
        {
            public class Server : TestHelper
            {
                public override void run(string[] args)
                {
                    using(var communicator = initialize(ref args))
                    {
                        communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
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
