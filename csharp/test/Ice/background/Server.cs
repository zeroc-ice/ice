//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using Ice;
using System.Reflection;
using System.Threading.Tasks;
using System.Collections.Generic;

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Server : TestHelper
{
    internal class Locator : ILocator
    {
        public ValueTask<IObjectPrx?> FindAdapterByIdAsync(string adapter, Current current)
        {
            _controller.checkCallPause(current);
            return new ValueTask<IObjectPrx?>(current.Adapter.CreateDirectProxy("dummy", IObjectPrx.Factory));
        }

        public ValueTask<IObjectPrx?> FindObjectByIdAsync(Identity id, Current current)
        {
            _controller.checkCallPause(current);
            return new ValueTask<IObjectPrx?>(current.Adapter.CreateDirectProxy(id, IObjectPrx.Factory));
        }

        public ILocatorRegistryPrx? GetRegistry(Current current) => null;

        internal Locator(BackgroundController controller) => _controller = controller;

        private readonly BackgroundController _controller;
    }

    internal class RouterI : IRouter
    {
        public (IObjectPrx?, bool?) GetClientProxy(Current current)
        {
            _controller.checkCallPause(current);
            return (null, true);
        }

        public IObjectPrx? GetServerProxy(Current current)
        {
            _controller.checkCallPause(current);
            return null;
        }

        public IEnumerable<IObjectPrx> AddProxies(IObjectPrx?[] proxies, Current current) => new IObjectPrx[0];

        internal RouterI(BackgroundController controller) => _controller = controller;

        private readonly BackgroundController _controller;
    }

    public override void Run(string[] args)
    {
        Dictionary<string, string> properties = CreateTestProperties(ref args);
        //
        // This test kills connections, so we don't want warnings.
        //
        properties["Ice.Warn.Connections"] = "0";

        properties["Ice.MessageSizeMax"] = "50000";

        // This test relies on filling the TCP send/recv buffer, so
        // we rely on a fixed value for these buffers.
        properties["Ice.TCP.RcvSize"] = "50000";

        //
        // Setup the test transport plug-in.
        //
        if (!properties.TryGetValue("Ice.Default.Transport", out string? transport))
        {
            transport = "tcp";
        }
        properties["Ice.Default.Transport"] = $"test-{transport}";

        using Communicator communicator = Initialize(properties);
        var plugin = new Plugin(communicator);
        plugin.Initialize();
        communicator.AddPlugin("Test", plugin);

        //
        // When running as a MIDlet the properties for the server may be
        // overridden by configuration. If it isn't then we assume
        // defaults.
        //
        if (communicator.GetProperty("TestAdapter.Endpoints") == null)
        {
            communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0));
        }

        if (communicator.GetProperty("ControllerAdapter.Endpoints") == null)
        {
            communicator.SetProperty("ControllerAdapter.Endpoints", GetTestEndpoint(1, "tcp"));
            communicator.SetProperty("ControllerAdapter.ThreadPool.Size", "1");
        }

        ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
        ObjectAdapter adapter2 = communicator.CreateObjectAdapter("ControllerAdapter");

        var backgroundController = new BackgroundController(adapter);
        var backgroundI = new Background(backgroundController);

        adapter.Add("background", backgroundI);

        var locatorI = new Locator(backgroundController);
        adapter.Add("locator", locatorI);

        var routerI = new RouterI(backgroundController);
        adapter.Add("router", routerI);
        adapter.Activate();

        adapter2.Add("backgroundController", backgroundController);
        adapter2.Activate();

        communicator.WaitForShutdown();
    }

    public static int Main(string[] args) => TestDriver.RunTest<Server>(args);
}
