// Copyright (c) ZeroC, Inc.

package test.Ice.background;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Current;
import com.zeroc.Ice.Identity;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.Locator;
import com.zeroc.Ice.LocatorRegistryPrx;
import com.zeroc.Ice.ModuleToPackageSliceLoader;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.ObjectPrx;
import com.zeroc.Ice.Router;
import com.zeroc.Ice.Util;

import test.Ice.background.PluginFactory.PluginI;
import test.TestHelper;

import java.util.Collections;
import java.util.Optional;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CompletionStage;

public class Server extends TestHelper {
    public static class LocatorI implements Locator {
        @Override
        public CompletionStage<ObjectPrx> findAdapterByIdAsync(
                String adapter, Current current) {
            _controller.checkCallPause(current);
            return CompletableFuture.completedFuture(
                current.adapter.createDirectProxy(
                    Util.stringToIdentity("dummy")));
        }

        @Override
        public CompletionStage<ObjectPrx> findObjectByIdAsync(
                Identity id, Current current) {
            _controller.checkCallPause(current);
            return CompletableFuture.completedFuture(current.adapter.createDirectProxy(id));
        }

        @Override
        public LocatorRegistryPrx getRegistry(Current current) {
            return null;
        }

        LocatorI(BackgroundControllerI controller) {
            _controller = controller;
        }

        private final BackgroundControllerI _controller;
    }

    public static class RouterI implements Router {
        @Override
        public Router.GetClientProxyResult getClientProxy(
                Current current) {
            _controller.checkCallPause(current);
            return new Router.GetClientProxyResult(null, Optional.of(true));
        }

        @Override
        public ObjectPrx getServerProxy(Current current) {
            _controller.checkCallPause(current);
            return null;
        }

        @Override
        public ObjectPrx[] addProxies(
                ObjectPrx[] proxies, Current current) {
            return new ObjectPrx[0];
        }

        RouterI(BackgroundControllerI controller) {
            _controller = controller;
        }

        private final BackgroundControllerI _controller;
    }

    @Override
    public void run(String[] args) {
        var initData = new InitializationData();
        initData.sliceLoader = new ModuleToPackageSliceLoader("::Test", "test.Ice.background.Test");
        // Install transport plug-in.
        initData.pluginFactories = Collections.singletonList(new PluginFactory());

        initData.properties = createTestProperties(args);
        // This test kills connections, so we don't want warnings.
        initData.properties.setProperty("Ice.Warn.Connections", "0");
        initData.properties.setProperty("Ice.MessageSizeMax", "50000");
        // This test relies on filling the TCP send/recv buffer, so we rely on a fixed value for these buffers.
        initData.properties.setProperty("Ice.TCP.RcvSize", "50000");
        initData.properties.setProperty(
            "Ice.Default.Protocol",
            "test-" + initData.properties.getIceProperty("Ice.Default.Protocol"));

        try (Communicator communicator = initialize(initData)) {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            communicator
                .getProperties()
                .setProperty("ControllerAdapter.Endpoints", getTestEndpoint(1, "tcp"));
            communicator.getProperties().setProperty("ControllerAdapter.ThreadPool.Size", "1");

            PluginI plugin = (PluginI) communicator().getPluginManager().getPlugin("Test");
            Configuration configuration = plugin.getConfiguration();

            ObjectAdapter adapter = communicator().createObjectAdapter("TestAdapter");
            ObjectAdapter adapter2 =
                communicator().createObjectAdapter("ControllerAdapter");

            BackgroundControllerI backgroundController =
                new BackgroundControllerI(configuration, adapter);

            adapter.add(
                new BackgroundI(backgroundController),
                Util.stringToIdentity("background"));
            adapter.add(
                new LocatorI(backgroundController),
                Util.stringToIdentity("locator"));
            adapter.add(
                new RouterI(backgroundController),
                Util.stringToIdentity("router"));
            adapter.activate();

            adapter2.add(
                backgroundController,
                Util.stringToIdentity("backgroundController"));
            adapter2.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
