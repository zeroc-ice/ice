// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Location
{
    public class Server : TestHelper
    {
        public override async Task RunAsync(string[] args)
        {
            // Register the server manager. The server manager creates a new 'server'(a server isn't a different
            // process, it's just a new communicator and object adapter).
            Dictionary<string, string> properties = CreateTestProperties(ref args);

            await using Communicator communicator = Initialize(properties);
            await communicator.ActivateAsync();
            communicator.SetProperty("ServerManagerAdapter.Endpoints", GetTestEndpoint(0));
            ObjectAdapter adapter = communicator.CreateObjectAdapter("ServerManagerAdapter");

            // We also register a sample server locator which implements the locator interface, this locator is used by
            // the clients and the 'servers' created with the server manager interface.
            var registry = new ServerLocatorRegistry();
            var obj = new ServerManager(registry, this);
            adapter.Add("ServerManager", obj);
            registry.AddObject(adapter.CreateProxy("ServerManager", IObjectPrx.Factory));
            ILocatorRegistryPrx registryPrx = adapter.Add("registry", registry, ILocatorRegistryPrx.Factory);
            adapter.Add("locator", new ServerLocator(registry, registryPrx));

            await adapter.ActivateAsync();
            ServerReady();
            await communicator.WaitForShutdownAsync();
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Server>(args);
    }
}
