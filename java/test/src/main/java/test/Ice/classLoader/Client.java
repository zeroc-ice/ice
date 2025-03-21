// Copyright (c) ZeroC, Inc.

package test.Ice.classLoader;

public class Client extends test.TestHelper {
    public void run(String[] args) {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.classLoader");
        try (com.zeroc.Ice.Communicator communicator = initialize(properties)) {
            AllTests.allTests(this, false);
        }
    }
}
