// Copyright (c) ZeroC, Inc.

package test.Ice.location;

import com.zeroc.Ice.Communicator;

import test.TestHelper;

public class Client extends TestHelper {
    public void run(String[] args) {
        var properties = createTestProperties(args);
        properties.setProperty("Ice.Default.Locator", "locator:" + getTestEndpoint(properties, 0));

        try (Communicator communicator = initialize(properties)) {
            AllTests.allTests(this);
        } catch (Exception ex) {
            throw new RuntimeException(ex);
        }
    }
}
