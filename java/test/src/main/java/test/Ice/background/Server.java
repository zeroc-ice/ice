// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.background;

import java.util.concurrent.CompletionStage;
import java.util.concurrent.CompletableFuture;

import test.Ice.background.PluginFactory.PluginI;

public class Server extends test.TestHelper
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
    public void run(String[] args)
    {
        com.zeroc.Ice.Properties properties = createTestProperties(args);

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
        properties.setProperty("Ice.Default.Protocol",
                               "test-" + properties.getPropertyWithDefault("Ice.Default.Protocol", "tcp"));
        properties.setProperty("Ice.Package.Test", "test.Ice.background");

        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            communicator.getProperties().setProperty("ControllerAdapter.Endpoints", getTestEndpoint(1, "tcp"));
            communicator.getProperties().setProperty("ControllerAdapter.ThreadPool.Size", "1");

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
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
