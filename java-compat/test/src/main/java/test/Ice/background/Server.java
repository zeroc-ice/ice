// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package test.Ice.background;

import test.Ice.background.PluginFactory.PluginI;

public class Server extends test.Util.Application
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

    @Override
    public int
    run(String[] args)
    {
        PluginI plugin = (PluginI)communicator().getPluginManager().getPlugin("Test");
        Configuration configuration = plugin.getConfiguration();

        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("TestAdapter");
        Ice.ObjectAdapter adapter2 = communicator().createObjectAdapter("ControllerAdapter");

        BackgroundControllerI backgroundController = new BackgroundControllerI(configuration, adapter);

        adapter.add(new BackgroundI(backgroundController), Ice.Util.stringToIdentity("background"));
        adapter.add(new LocatorI(backgroundController), Ice.Util.stringToIdentity("locator"));
        adapter.add(new RouterI(backgroundController), Ice.Util.stringToIdentity("router"));
        adapter.activate();

        adapter2.add(backgroundController, Ice.Util.stringToIdentity("backgroundController"));
        adapter2.activate();

        return WAIT;
    }

    @Override
    protected Ice.InitializationData getInitData(Ice.StringSeqHolder argsH)
    {
        Ice.InitializationData initData = super.getInitData(argsH);

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
        initData.properties.setProperty("Ice.Plugin.Test", "test.Ice.background.PluginFactory");
        String defaultProtocol = initData.properties.getPropertyWithDefault("Ice.Default.Protocol", "tcp");
        initData.properties.setProperty("Ice.Default.Protocol", "test-" + defaultProtocol);

        initData.properties.setProperty("Ice.Package.Test", "test.Ice.background");

        initData.properties.setProperty("TestAdapter.Endpoints", getTestEndpoint(initData.properties, 0));
        initData.properties.setProperty("ControllerAdapter.Endpoints", getTestEndpoint(initData.properties, 1, "tcp"));
        initData.properties.setProperty("ControllerAdapter.ThreadPool.Size", "1");

        return initData;
    }

    public static void
    main(String[] args)
    {
        Server app = new Server();
        int result = app.main("Server", args);
        System.gc();
        System.exit(result);
    }
}
