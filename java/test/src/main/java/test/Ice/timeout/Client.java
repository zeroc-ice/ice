// Copyright (c) ZeroC, Inc.

package test.Ice.timeout;

import com.zeroc.Ice.Communicator;

import test.TestHelper;

public class Client extends TestHelper {
    @Override
    public void run(String[] args) {
        var properties = createTestProperties(args);

        // For this test, we want to disable retries.
        properties.setProperty("Ice.RetryIntervals", "-1");

        properties.setProperty("Ice.Connection.Client.ConnectTimeout", "1");
        properties.setProperty("Ice.Connection.Client.CloseTimeout", "1");

        try (Communicator communicator = initialize(properties)) {
            AllTests.allTests(this);
        }
    }
}
