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

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Server
{
    internal class LocatorI : Ice.LocatorDisp_
    {
        public override void findAdapterById_async(Ice.AMD_Locator_findAdapterById response, string adapter,
                                                   Ice.Current current)
        {
            _controller.checkCallPause(current);
            Ice.Communicator communicator = current.adapter.getCommunicator();
            response.ice_response(current.adapter.createDirectProxy(communicator.stringToIdentity("dummy")));
        }

        public override void findObjectById_async(Ice.AMD_Locator_findObjectById response, Ice.Identity id,
                                                  Ice.Current current)
        {
            _controller.checkCallPause(current);
            response.ice_response(current.adapter.createDirectProxy(id));
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

    public static int run(string[] args, Ice.Communicator communicator, TextWriter @out)
    {
        //
        // When running as a MIDlet the properties for the server may be
        // overridden by configuration. If it isn't then we assume
        // defaults.
        //
        if(communicator.getProperties().getProperty("TestAdapter.Endpoints").Length == 0)
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -p 12010");
        }
        if(communicator.getProperties().getProperty("ControllerAdapter.Endpoints").Length == 0)
        {
            communicator.getProperties().setProperty("ControllerAdapter.Endpoints", "tcp -p 12011");
            communicator.getProperties().setProperty("ControllerAdapter.ThreadPool.Size", "1");
        }

        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        Ice.ObjectAdapter adapter2 = communicator.createObjectAdapter("ControllerAdapter");

        BackgroundControllerI backgroundController = new BackgroundControllerI(adapter);

        adapter.add(new BackgroundI(backgroundController), communicator.stringToIdentity("background"));
        adapter.add(new LocatorI(backgroundController), communicator.stringToIdentity("locator"));
        adapter.add(new RouterI(backgroundController), communicator.stringToIdentity("router"));
        adapter.activate();

        adapter2.add(backgroundController, communicator.stringToIdentity("backgroundController"));
        adapter2.activate();

        communicator.waitForShutdown();
        return 0;
    }

    public static int Main(string[] args)
    {
        int status = 0;
        Ice.Communicator communicator = null;

        try
        {
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = Ice.Util.createProperties(ref args);

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

            communicator = Ice.Util.initialize(ref args, initData);
            PluginI plugin = new PluginI(communicator);
            plugin.initialize();
            communicator.getPluginManager().addPlugin("Test", plugin);

            status = run(args, communicator, Console.Out);
        }
        catch(System.Exception ex)
        {
            Console.Error.WriteLine(ex);
            status = 1;
        }

        if(communicator != null)
        {
            try
            {
                communicator.destroy();
            }
            catch(Ice.LocalException ex)
            {
                Console.Error.WriteLine(ex);
                status = 1;
            }
        }

        return status;
    }
}
