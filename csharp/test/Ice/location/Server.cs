//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using System.Collections.Generic;

namespace Ice.location
{
    public class Server : TestHelper
    {
        public override void Run(string[] args)
        {
            //
            // Register the server manager. The server manager creates a new
            // 'server'(a server isn't a different process, it's just a new
            // communicator and object adapter).
            //
            Dictionary<string, string> properties = CreateTestProperties(ref args);

            using Communicator communicator = Initialize(properties);
            communicator.SetProperty("ServerManagerAdapter.Endpoints", GetTestEndpoint(0));
            ObjectAdapter adapter = communicator.CreateObjectAdapter("ServerManagerAdapter");
            //
            // We also register a sample server locator which implements the
            // locator interface, this locator is used by the clients and the
            // 'servers' created with the server manager interface.
            //
            var registry = new ServerLocatorRegistry();
            var obj = new ServerManager(registry, this);
            adapter.Add("ServerManager", obj);
            registry.AddObject(adapter.CreateProxy("ServerManager", IObjectPrx.Factory));
            ILocatorRegistryPrx registryPrx = adapter.Add("registry", registry, ILocatorRegistryPrx.Factory);
            adapter.Add("locator", new ServerLocator(registry, registryPrx));

            adapter.Activate();
            ServerReady();
            communicator.WaitForShutdown();
        }

        public static int Main(string[] args) => TestDriver.RunTest<Server>(args);
    }
}
