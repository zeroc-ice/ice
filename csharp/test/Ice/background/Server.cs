// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.IO;
using System.Reflection;
using System.Threading.Tasks;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Server : TestCommon.Application
{
    internal class LocatorI : Ice.LocatorDisp_
    {
        public override Task<Ice.ObjectPrx>
        findAdapterByIdAsync(string adapter,  Ice.Current current)
        {
            _controller.checkCallPause(current);
            Ice.Communicator communicator = current.adapter.getCommunicator();
            return Task<Ice.ObjectPrx>.FromResult(current.adapter.createDirectProxy(Ice.Util.stringToIdentity("dummy")));
        }

        public override Task<Ice.ObjectPrx>
        findObjectByIdAsync(Ice.Identity id,  Ice.Current current)
        {
            _controller.checkCallPause(current);
            return Task<Ice.ObjectPrx>.FromResult(current.adapter.createDirectProxy(id));
        }

        public override Ice.LocatorRegistryPrx getRegistry(Ice.Current current)
        {
            return null;
        }

        internal LocatorI(BackgroundControllerI controller)
        {
            _controller = controller;
        }

        private BackgroundControllerI _controller;
    }

    internal class RouterI : Ice.RouterDisp_
    {
        public override Ice.ObjectPrx getClientProxy(Ice.Current current)
        {
            _controller.checkCallPause(current);
            return null;
        }

        public override Ice.ObjectPrx getServerProxy(Ice.Current current)
        {
            _controller.checkCallPause(current);
            return null;
        }

        public override Ice.ObjectPrx[] addProxies(Ice.ObjectPrx[] proxies, Ice.Current current)
        {
            return new Ice.ObjectPrx[0];
        }

        internal RouterI(BackgroundControllerI controller)
        {
            _controller = controller;
        }

        private BackgroundControllerI _controller;
    }

    public override int run(string[] args)
    {
        PluginI plugin = new PluginI(communicator());
        plugin.initialize();
        communicator().getPluginManager().addPlugin("Test", plugin);

        //
        // When running as a MIDlet the properties for the server may be
        // overridden by configuration. If it isn't then we assume
        // defaults.
        //
        if(communicator().getProperties().getProperty("TestAdapter.Endpoints").Length == 0)
        {
            communicator().getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
        }
        if(communicator().getProperties().getProperty("ControllerAdapter.Endpoints").Length == 0)
        {
            communicator().getProperties().setProperty("ControllerAdapter.Endpoints", getTestEndpoint(1, "tcp"));
            communicator().getProperties().setProperty("ControllerAdapter.ThreadPool.Size", "1");
        }

        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("TestAdapter");
        Ice.ObjectAdapter adapter2 = communicator().createObjectAdapter("ControllerAdapter");

        BackgroundControllerI backgroundController = new BackgroundControllerI(adapter);

        adapter.add(new BackgroundI(backgroundController), Ice.Util.stringToIdentity("background"));
        adapter.add(new LocatorI(backgroundController), Ice.Util.stringToIdentity("locator"));
        adapter.add(new RouterI(backgroundController), Ice.Util.stringToIdentity("router"));
        adapter.activate();

        adapter2.add(backgroundController, Ice.Util.stringToIdentity("backgroundController"));
        adapter2.activate();

        communicator().waitForShutdown();
        return 0;
    }

    protected override Ice.InitializationData getInitData(ref string[] args)
    {
        Ice.InitializationData initData = base.getInitData(ref args);

        //
        // This test kills connections, so we don't want warnings.
        //
        initData.properties.setProperty("Ice.Warn.Connections", "0");

        initData.properties.setProperty("Ice.MessageSizeMax", "50000");

        // This test relies on filling the TCP send/recv buffer, so
        // we rely on a fixed value for these buffers.
        initData.properties.setProperty("Ice.TCP.RcvSize", "50000");

        //
        // Setup the test transport plug-in.
        //
        string defaultProtocol = initData.properties.getPropertyWithDefault("Ice.Default.Protocol", "tcp");
        initData.properties.setProperty("Ice.Default.Protocol", "test-" + defaultProtocol);
        return initData;
    }

    public static int Main(string[] args)
    {
        Server app = new Server();
        return app.runmain(args);
    }
}
