// Copyright (c) ZeroC, Inc.

package test.Ice.idleTimeout;

import test.TestHelper;

public class Client extends TestHelper {
    @Override
    public void run(String[] args) {
        var properties = createTestProperties(args);
        properties.setProperty("Ice.Connection.Client.IdleTimeout", "1");

        try (var communicator = initialize(properties)) {
            AllTests.allTests(this);
        }
    }
}
