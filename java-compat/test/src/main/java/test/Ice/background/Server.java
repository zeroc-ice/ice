// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package test.Ice.background;

import test.Ice.background.PluginFactory.PluginI;

public class Server extends test.TestHelper
{
    static public class LocatorI extends Ice._LocatorDisp
    {
        @Override
        public void
        findAdapterById_async(Ice.AMD_Locator_findAdapterById response, String adapter, Ice.Current current)
            throws Ice.AdapterNotFoundException
        {
            _controller.checkCallPause(current);
            Ice.Communicator communicator = current.adapter.getCommunicator();
            response.ice_response(current.adapter.createDirectProxy(Ice.Util.stringToIdentity("dummy")));
        }

        @Override
        public void
        findObjectById_async(Ice.AMD_Locator_findObjectById response, Ice.Identity id, Ice.Current current)
            throws Ice.ObjectNotFoundException
        {
            _controller.checkCallPause(current);
            response.ice_response(current.adapter.createDirectProxy(id));
        }

        @Override
        public Ice.LocatorRegistryPrx
        getRegistry(Ice.Current current)
        {
            return null;
        }

        LocatorI(BackgroundControllerI controller)
        {
            _controller = controller;
        }

        final private BackgroundControllerI _controller;
    }

    static public class RouterI extends Ice._RouterDisp
    {
        @Override
        public Ice.ObjectPrx
        getClientProxy(Ice.BooleanHolder hasRoutingTable, Ice.Current current)
        {
            _controller.checkCallPause(current);
            hasRoutingTable.value = true;
            return null;
        }

        @Override
        public Ice.ObjectPrx
        getServerProxy(Ice.Current current)
        {
            _controller.checkCallPause(current);
            return null;
        }

        @Override
        public Ice.ObjectPrx[]
        addProxies(Ice.ObjectPrx[] proxies, Ice.Current current)
        {
            return new Ice.ObjectPrx[0];
        }

        RouterI(BackgroundControllerI controller)
        {
            _controller = controller;
        }

        final private BackgroundControllerI _controller;
    }

    public void
    run(String[] args)
    {
        Ice.Properties properties = createTestProperties(args);

        //
        // This test kills connections, so we don't want warnings.
        //
        properties.setProperty("Ice.Warn.Connections", "0");
        properties.setProperty("Ice.MessageSizeMax", "50000");

        // This test relies on filling the TCP send/recv buffer, so
        // we rely on a fixed value for these buffers.
        properties.setProperty("Ice.TCP.RcvSize", "50000");

        //
        // Setup the test transport plug-in.
        //
        properties.setProperty("Ice.Plugin.Test", "test.Ice.background.PluginFactory");
        String defaultProtocol = properties.getPropertyWithDefault("Ice.Default.Protocol", "tcp");
        properties.setProperty("Ice.Default.Protocol", "test-" + defaultProtocol);

        properties.setProperty("Ice.Package.Test", "test.Ice.background");

        try(Ice.Communicator communicator = initialize(properties))
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            communicator.getProperties().setProperty("ControllerAdapter.Endpoints", getTestEndpoint(1, "tcp"));
            communicator.getProperties().setProperty("ControllerAdapter.ThreadPool.Size", "1");

            PluginI plugin = (PluginI)communicator().getPluginManager().getPlugin("Test");
            Configuration configuration = plugin.getConfiguration();

            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");

            BackgroundControllerI backgroundController = new BackgroundControllerI(configuration, adapter);

            adapter.add(new BackgroundI(backgroundController), Ice.Util.stringToIdentity("background"));
            adapter.add(new LocatorI(backgroundController), Ice.Util.stringToIdentity("locator"));
            adapter.add(new RouterI(backgroundController), Ice.Util.stringToIdentity("router"));
            adapter.activate();

            Ice.ObjectAdapter adapter2 = communicator.createObjectAdapter("ControllerAdapter");
            adapter2.add(backgroundController, Ice.Util.stringToIdentity("backgroundController"));
            adapter2.activate();

            serverReady();
            communicator.waitForShutdown();
        }
    }
}
