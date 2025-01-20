// Copyright (c) ZeroC, Inc.

package test.Ice.timeout;

public class Client extends test.TestHelper {
    @Override
    public void run(String[] args) {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.timeout");

        //
        // For this test, we want to disable retries.
        //
        properties.setProperty("Ice.RetryIntervals", "-1");

        properties.setProperty("Ice.Connection.Client.ConnectTimeout", "1");
        properties.setProperty("Ice.Connection.Client.CloseTimeout", "1");

        try (com.zeroc.Ice.Communicator communicator = initialize(properties)) {
            AllTests.allTests(this);
        }
    }
}
