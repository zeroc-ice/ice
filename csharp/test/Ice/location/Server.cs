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
            var properties = createTestProperties(ref args);
            properties["Ice.ThreadPool.Server.Size"] = "2";

            using var communicator = initialize(properties);
            communicator.SetProperty("ServerManagerAdapter.Endpoints", getTestEndpoint(0));
            var adapter = communicator.CreateObjectAdapter("ServerManagerAdapter");
            //
            // We also register a sample server locator which implements the
            // locator interface, this locator is used by the clients and the
            // 'servers' created with the server manager interface.
            //
            var registry = new ServerLocatorRegistry();
            var obj = new ServerManager(registry, this);
            adapter.Add(obj, "ServerManager");
            registry.addObject(adapter.CreateProxy("ServerManager"));
            var registryPrx = adapter.Add(registry, "registry");
            adapter.Add(new ServerLocator(registry, registryPrx), "locator");

            adapter.Activate();
            serverReady();
            communicator.WaitForShutdown();
        }

        public static int Main(string[] args) => TestDriver.runTest<Server>(args);
    }
}
