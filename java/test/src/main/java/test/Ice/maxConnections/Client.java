// Copyright (c) ZeroC, Inc.

package test.Ice.maxConnections;

import test.TestHelper;

public class Client extends TestHelper {
    @Override
    public void run(String[] args) {
        var properties = createTestProperties(args);
        // We disable retries to make the logs clearer and avoid hiding potential issues.
        properties.setProperty("Ice.RetryIntervals", "-1");

        try (var communicator = initialize(properties)) {
            AllTests.allTests(this);
        }
    }
}
