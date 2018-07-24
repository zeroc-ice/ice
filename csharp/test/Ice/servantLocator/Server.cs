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
    namespace servantLocator
    {
        public class Server : TestHelper
        {
            public override void run(string[] args)
            {
                var properties = createTestProperties(ref args);
                properties.setProperty("Ice.Package.Test", "Ice.servantLocator");
                using(var communicator = initialize(properties))
                {
                    communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                    communicator.getProperties().setProperty("Ice.Warn.Dispatch", "0");

                    var adapter = communicator.createObjectAdapter("TestAdapter");
                    adapter.addServantLocator(new ServantLocatorI("category"), "category");
                    adapter.addServantLocator(new ServantLocatorI(""), "");
                    adapter.add(new TestI(), Ice.Util.stringToIdentity("asm"));
                    adapter.add(new TestActivationI(), Ice.Util.stringToIdentity("test/activation"));
                    adapter.activate();
                    serverReady();
                    adapter.waitForDeactivate();
                }
            }

            public static int Main(string[] args)
            {
                return TestDriver.runTest<Server>(args);
            }
        }
    }
}
