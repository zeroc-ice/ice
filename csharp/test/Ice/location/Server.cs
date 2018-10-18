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
    namespace location
    {
        public class Server : TestHelper
        {
            public override void run(string[] args)
            {
                //
                // Register the server manager. The server manager creates a new
                // 'server'(a server isn't a different process, it's just a new
                // communicator and object adapter).
                //
                Ice.Properties properties = createTestProperties(ref args);
                properties.setProperty("Ice.ThreadPool.Server.Size", "2");

                using(var communicator = initialize(properties))
                {
                    communicator.getProperties().setProperty("ServerManagerAdapter.Endpoints", getTestEndpoint(0));
                    Ice.ObjectAdapter adapter = communicator.createObjectAdapter("ServerManagerAdapter");

                    //
                    // We also register a sample server locator which implements the
                    // locator interface, this locator is used by the clients and the
                    // 'servers' created with the server manager interface.
                    //
                    ServerLocatorRegistry registry = new ServerLocatorRegistry();
                    Ice.Object @object = new ServerManagerI(registry, this);
                    adapter.add(@object, Ice.Util.stringToIdentity("ServerManager"));
                    registry.addObject(adapter.createProxy(Ice.Util.stringToIdentity("ServerManager")));
                    Ice.LocatorRegistryPrx registryPrx =
                        Ice.LocatorRegistryPrxHelper.uncheckedCast(adapter.add(registry, Ice.Util.stringToIdentity("registry")));

                    ServerLocator locator = new ServerLocator(registry, registryPrx);
                    adapter.add(locator, Ice.Util.stringToIdentity("locator"));

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
