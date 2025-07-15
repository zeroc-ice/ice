// Copyright (c) ZeroC, Inc.

package test.Ice.idleTimeout;

import com.zeroc.Ice.Identity;

import test.TestHelper;

public class Server extends TestHelper {
    public void run(String[] args) {
        var properties = createTestProperties(args);
        properties.setProperty("TestAdapter.Connection.IdleTimeout", "1"); // 1 second
        properties.setProperty("TestAdapter.Connection.MaxDispatches", "1");
        properties.setProperty("Ice.Warn.Connections", "0");

        try (var communicator = initialize(properties)) {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            var adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new TestIntfI(), new Identity("test", ""));
            adapter.activate();

            communicator
                .getProperties()
                .setProperty("TestAdapterDefaultMax.Endpoints", getTestEndpoint(1));
            communicator
                .getProperties()
                .setProperty("TestAdapterDefaultMax.Connection.IdleTimeout", "1");
            var adapterDefaultMax = communicator.createObjectAdapter("TestAdapterDefaultMax");
            adapterDefaultMax.add(new TestIntfI(), new Identity("test", ""));
            adapterDefaultMax.activate();

            communicator.getProperties().setProperty("TestAdapter3s.Endpoints", getTestEndpoint(2));
            communicator.getProperties().setProperty("TestAdapter3s.Connection.IdleTimeout", "3");
            var adapter3s = communicator.createObjectAdapter("TestAdapter3s");
            adapter3s.add(new TestIntfI(), new Identity("test", ""));
            adapter3s.activate();

            communicator
                .getProperties()
                .setProperty("TestAdapterNoIdleTimeout.Endpoints", getTestEndpoint(3));
            communicator
                .getProperties()
                .setProperty("TestAdapterNoIdleTimeout.Connection.IdleTimeout", "0");
            var adapterNoIdleTimeout = communicator.createObjectAdapter("TestAdapterNoIdleTimeout");
            adapterNoIdleTimeout.add(new TestIntfI(), new Identity("test", ""));
            adapterNoIdleTimeout.activate();

            serverReady();
            communicator.waitForShutdown();
        }
    }
}
