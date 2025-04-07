// Copyright (c) ZeroC, Inc.

package test.Ice.maxConnections;

import com.zeroc.Ice.Identity;

import test.TestHelper;

public class Server extends TestHelper {
    public void run(String[] args) {
        var properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.maxConnections");

        try (var communicator = initialize(properties)) {
            // Plain adapter with no limit.
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint());
            var adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new TestIntfI(), new Identity("test", ""));
            adapter.activate();

            // Accepts 10 connections.
            communicator
                    .getProperties()
                    .setProperty("TestAdapterMax10.Endpoints", getTestEndpoint(1));
            communicator.getProperties().setProperty("TestAdapterMax10.MaxConnections", "10");
            var adapterMax10 = communicator.createObjectAdapter("TestAdapterMax10");
            adapterMax10.add(new TestIntfI(), new Identity("test", ""));
            adapterMax10.activate();

            serverReady();
            communicator.waitForShutdown();
        }
    }
}
