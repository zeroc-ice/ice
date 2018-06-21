// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.location;

import com.zeroc.Ice.LocatorRegistryPrx;

public class Server extends test.TestHelper
{
    public void run(String[] args)
    {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.location");
        properties.setProperty("Ice.ThreadPool.Server.Size", "2");
        properties.setProperty("Ice.ThreadPool.Server.SizeWarn", "0");

        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
            communicator.getProperties().setProperty("ServerManagerAdapter.Endpoints", getTestEndpoint(0));

            //
            // Register the server manager. The server manager creates a new
            // 'server' (a server isn't a different process, it's just a new
            // communicator and object adapter).
            //
            com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("ServerManagerAdapter");

            //
            // We also register a sample server locator which implements the
            // locator interface, this locator is used by the clients and the
            // 'servers' created with the server manager interface.
            //
            ServerLocatorRegistry registry = new ServerLocatorRegistry();
            registry.addObject(adapter.createProxy(com.zeroc.Ice.Util.stringToIdentity("ServerManager")), null);
            com.zeroc.Ice.Object object = new ServerManagerI(registry, this);
            adapter.add(object, com.zeroc.Ice.Util.stringToIdentity("ServerManager"));

            LocatorRegistryPrx registryPrx =
                LocatorRegistryPrx.uncheckedCast(adapter.add(registry, com.zeroc.Ice.Util.stringToIdentity("registry")));

            adapter.add(new ServerLocator(registry, registryPrx), com.zeroc.Ice.Util.stringToIdentity("locator"));
            adapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
