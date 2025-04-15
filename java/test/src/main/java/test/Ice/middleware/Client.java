// Copyright (c) ZeroC, Inc.

package test.Ice.middleware;

import com.zeroc.Ice.Communicator;

import test.TestHelper;

public class Client extends TestHelper {
    public void run(String[] args) {
        var properties = createTestProperties(args);

        // Suppress dispatch exception logging; the error observer middleware
        // tests throw exceptions during dispatch.
        properties.setProperty("Ice.Warn.Dispatch", "0");
        try (Communicator communicator = initialize(properties)) {
            AllTests.allTests(this);
        }
    }
}
