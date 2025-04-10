// Copyright (c) ZeroC, Inc.

package test.Ice.slicing.exceptions;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Properties;

import test.Ice.slicing.exceptions.client.Test.TestIntfPrx;
import test.TestHelper;

public class Client extends TestHelper {
    public void run(String[] args) {
        Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.slicing.exceptions.client");
        try (Communicator communicator = initialize(properties)) {
            TestIntfPrx test = AllTests.allTests(this, false);
            test.shutdown();
        }
    }
}
