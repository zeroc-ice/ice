// Copyright (c) ZeroC, Inc.

package test.Ice.idleTimeout;

import com.zeroc.Ice.Identity;

public class Server extends test.TestHelper {
    public void run(String[] args) {
        var properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.idleTimeout");
        properties.setProperty("Ice.Warn.Connections", "0");
        properties.setProperty("TestAdapter.Connection.IdleTimeout", "1"); // 1 second
        properties.setProperty(
                "TestAdapter.ThreadPool.Size", "1"); // dedicated thread pool with a single thread

        try (var communicator = initialize(properties)) {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint());
            var adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new TestIntfI(), new Identity("test", ""));
            adapter.activate();

            communicator.getProperties().setProperty("TestAdapter3s.Endpoints", getTestEndpoint(1));
            communicator.getProperties().setProperty("TestAdapter3s.Connection.IdleTimeout", "3");
            var adapter3s = communicator.createObjectAdapter("TestAdapter3s");
            adapter3s.add(new TestIntfI(), new Identity("test", ""));
            adapter3s.activate();

            serverReady();
            communicator.waitForShutdown();
        }
    }
}
