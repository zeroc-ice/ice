// Copyright (c) ZeroC, Inc.

package test.Ice.exceptions;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Properties;

import test.Ice.exceptions.Test.ThrowerPrx;
import test.TestHelper;

public class Client extends TestHelper {
    public void run(String[] args) {
        Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.exceptions");
        properties.setProperty("Ice.Warn.Connections", "0");
        properties.setProperty("Ice.MessageSizeMax", "10"); // 10KB max
        try (Communicator communicator = initialize(properties)) {
            ThrowerPrx thrower = AllTests.allTests(this);
            thrower.shutdown();
        }
    }
}
