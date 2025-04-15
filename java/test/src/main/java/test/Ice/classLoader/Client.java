// Copyright (c) ZeroC, Inc.

package test.Ice.classLoader;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Properties;

import test.TestHelper;

public class Client extends TestHelper {
    public void run(String[] args) {
        Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.classLoader");
        try (Communicator communicator = initialize(properties)) {
            AllTests.allTests(this, false);
        }
    }
}
