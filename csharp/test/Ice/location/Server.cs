//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice.location
{
    using global::Test;
    using Ice.location.Test;
    public class Server : TestHelper
    {
        public override void run(string[] args)
        {
            //
            // Register the server manager. The server manager creates a new
            // 'server'(a server isn't a different process, it's just a new
            // communicator and object adapter).
            //
            Properties properties = createTestProperties(ref args);
            properties.setProperty("Ice.ThreadPool.Server.Size", "2");

            using (var communicator = initialize(properties))
            {
                communicator.Properties.setProperty("ServerManagerAdapter.Endpoints", getTestEndpoint(0));
                ObjectAdapter adapter = communicator.createObjectAdapter("ServerManagerAdapter");
                //
                // We also register a sample server locator which implements the
                // locator interface, this locator is used by the clients and the
                // 'servers' created with the server manager interface.
                //
                ServerLocatorRegistry registry = new ServerLocatorRegistry();
                var obj = new ServerManagerI(registry, this);
                adapter.Add(obj, "ServerManager");
                registry.addObject(adapter.CreateProxy("ServerManager"));
                LocatorRegistryPrx registryPrx = adapter.Add(registry, "registry");
                adapter.Add(new ServerLocator(registry, registryPrx), "locator");

                adapter.Activate();
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
