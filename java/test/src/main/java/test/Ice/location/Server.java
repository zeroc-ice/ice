// Copyright (c) ZeroC, Inc.

package test.Ice.location;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Identity;
import com.zeroc.Ice.LocatorRegistryPrx;
import com.zeroc.Ice.Object;
import com.zeroc.Ice.ObjectAdapter;

import test.TestHelper;

public class Server extends TestHelper {
    public void run(String[] args) {
        var properties = createTestProperties(args);
        properties.setProperty("Ice.ThreadPool.Server.Size", "2");
        properties.setProperty("Ice.ThreadPool.Server.SizeWarn", "0");

        try (Communicator communicator = initialize(properties)) {
            test(properties == communicator.getProperties());

            properties.setProperty("ServerManagerAdapter.Endpoints", getTestEndpoint(0));

            //
            // Register the server manager. The server manager creates a new
            // 'server' (a server isn't a different process, it's just a new
            // communicator and object adapter).
            //
            ObjectAdapter adapter =
                communicator.createObjectAdapter("ServerManagerAdapter");

            //
            // We also register a sample server locator which implements the
            // locator interface, this locator is used by the clients and the
            // 'servers' created with the server manager interface.
            //
            ServerLocatorRegistry registry = new ServerLocatorRegistry();
            registry.addObject(
                adapter.createProxy(new Identity("ServerManager", "")),
                null);
            Object object = new ServerManagerI(registry, this);
            adapter.add(object, new Identity("ServerManager", ""));

            LocatorRegistryPrx registryPrx =
                LocatorRegistryPrx.uncheckedCast(
                    adapter.add(registry, new Identity("registry", "")));

            adapter.add(
                new ServerLocator(registry, registryPrx),
                new Identity("locator", ""));
            adapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
