// Copyright (c) ZeroC, Inc.

package test.Ice.inactivityTimeout;

import com.zeroc.Ice.Identity;

public class Server extends test.TestHelper {
    public void run(String[] args) {
        var properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.inactivityTimeout");
        // We configure a low idle timeout to make sure we send heartbeats frequently. It's the sending of the heartbeats that schedules the inactivity timer.
        properties.setProperty("Ice.Connection.Server.IdleTimeout", "1");
        properties.setProperty("TestAdapter.Connection.InactivityTimeout", "5");
        properties.setProperty("TestAdapter3s.Connection.InactivityTimeout", "3");

        try (var communicator = initialize(properties)) {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint());
            communicator.getProperties().setProperty("TestAdapter3s.Endpoints", getTestEndpoint(1));

            var adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new TestIntfI(), new Identity("test", ""));
            adapter.activate();

            var adapter3s = communicator.createObjectAdapter("TestAdapter3s");
            adapter3s.add(new TestIntfI(), new Identity("test", ""));
            adapter3s.activate();

            serverReady();
            communicator.waitForShutdown();
        }
    }
}
