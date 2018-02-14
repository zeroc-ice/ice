// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.background;

import java.util.concurrent.CompletionStage;
import java.util.concurrent.CompletableFuture;

import test.Ice.background.PluginFactory.PluginI;

public class Server extends test.Util.Application
{
    static public class LocatorI implements com.zeroc.Ice.Locator
    {
        @Override
        public CompletionStage<com.zeroc.Ice.ObjectPrx> findAdapterByIdAsync(String adapter,
                                                                             com.zeroc.Ice.Current current)
        {
            _controller.checkCallPause(current);
            com.zeroc.Ice.Communicator communicator = current.adapter.getCommunicator();
            return CompletableFuture.completedFuture(
                current.adapter.createDirectProxy(com.zeroc.Ice.Util.stringToIdentity("dummy")));
        }

        @Override
        public CompletionStage<com.zeroc.Ice.ObjectPrx> findObjectByIdAsync(com.zeroc.Ice.Identity id,
                                                                            com.zeroc.Ice.Current current)
        {
            _controller.checkCallPause(current);
            return CompletableFuture.completedFuture(current.adapter.createDirectProxy(id));
        }

        @Override
        public com.zeroc.Ice.LocatorRegistryPrx getRegistry(com.zeroc.Ice.Current current)
        {
            return null;
        }

        LocatorI(BackgroundControllerI controller)
        {
            _controller = controller;
        }

        final private BackgroundControllerI _controller;
    }

    static public class RouterI implements com.zeroc.Ice.Router
    {
        @Override
        public com.zeroc.Ice.Router.GetClientProxyResult getClientProxy(com.zeroc.Ice.Current current)
        {
            _controller.checkCallPause(current);
            return new com.zeroc.Ice.Router.GetClientProxyResult(null, java.util.Optional.of(true));
        }

        @Override
        public com.zeroc.Ice.ObjectPrx getServerProxy(com.zeroc.Ice.Current current)
        {
            _controller.checkCallPause(current);
            return null;
        }

        @Override
        public com.zeroc.Ice.ObjectPrx[] addProxies(com.zeroc.Ice.ObjectPrx[] proxies, com.zeroc.Ice.Current current)
        {
            return new com.zeroc.Ice.ObjectPrx[0];
        }

        RouterI(BackgroundControllerI controller)
        {
            _controller = controller;
        }

        final private BackgroundControllerI _controller;
    }

    @Override
    public int run(String[] args)
    {
        PluginI plugin = (PluginI)communicator().getPluginManager().getPlugin("Test");
        Configuration configuration = plugin.getConfiguration();

        com.zeroc.Ice.ObjectAdapter adapter = communicator().createObjectAdapter("TestAdapter");
        com.zeroc.Ice.ObjectAdapter adapter2 = communicator().createObjectAdapter("ControllerAdapter");

        BackgroundControllerI backgroundController = new BackgroundControllerI(configuration, adapter);

        adapter.add(new BackgroundI(backgroundController), com.zeroc.Ice.Util.stringToIdentity("background"));
        adapter.add(new LocatorI(backgroundController), com.zeroc.Ice.Util.stringToIdentity("locator"));
        adapter.add(new RouterI(backgroundController), com.zeroc.Ice.Util.stringToIdentity("router"));
        adapter.activate();

        adapter2.add(backgroundController, com.zeroc.Ice.Util.stringToIdentity("backgroundController"));
        adapter2.activate();

        return WAIT;
    }

    @Override
    protected com.zeroc.Ice.InitializationData getInitData(String[] args, java.util.List<String> rArgs)
    {
        com.zeroc.Ice.InitializationData initData = super.getInitData(args, rArgs);

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
        initData.properties.setProperty("ControllerAdapter.Endpoints",
                                          getTestEndpoint(initData.properties, 1, "tcp"));
        initData.properties.setProperty("ControllerAdapter.ThreadPool.Size", "1");

        return initData;
    }

    public static void main(String[] args)
    {
        Server app = new Server();
        int result = app.main("Server", args);
        System.gc();
        System.exit(result);
    }
}
