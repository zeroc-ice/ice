// Copyright (c) ZeroC, Inc.

package test.Ice.middleware;

public class Client extends test.TestHelper {
    public void run(String[] args) {
        var properties = createTestProperties(args);

        // Suppress dispatch exception logging; the error observer middleware
        // tests throw exceptions during dispatch.
        properties.setProperty("Ice.Warn.Dispatch", "0");
        try (com.zeroc.Ice.Communicator communicator = initialize(properties)) {
            AllTests.allTests(this);
        }
    }
}
