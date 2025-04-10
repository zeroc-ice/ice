// Copyright (c) ZeroC, Inc.

package test.Ice.maxDispatches;

import test.TestHelper;

public class Client extends TestHelper {
    @Override
    public void run(String[] args) {
        var properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.maxDispatches");

        try (var communicator = initialize(properties)) {
            AllTests.allTests(this);
        }
    }
}
