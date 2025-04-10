// Copyright (c) ZeroC, Inc.

package test.Ice.servantLocator;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Properties;

import test.Ice.servantLocator.Test.TestIntfPrx;
import test.TestHelper;

public class Client extends TestHelper {
    public void run(String[] args) {
        Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.servantLocator");
        try (Communicator communicator = initialize(properties)) {
            TestIntfPrx obj = AllTests.allTests(this);
            obj.shutdown();
        }
    }
}
