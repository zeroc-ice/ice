// Copyright (c) ZeroC, Inc.

package test.Ice.inactivityTimeout;

import test.TestHelper;

public class Client extends TestHelper {
    @Override
    public void run(String[] args) {
        var properties = createTestProperties(args);
        // We configure a low idle timeout to make sure we send heartbeats frequently. It's the
        // sending of the heartbeats that schedules the inactivity timer task.
        properties.setProperty("Ice.Connection.Client.IdleTimeout", "1");
        properties.setProperty("Ice.Connection.Client.InactivityTimeout", "3");

        try (var communicator = initialize(properties)) {
            AllTests.allTests(this);
        }
    }
}
