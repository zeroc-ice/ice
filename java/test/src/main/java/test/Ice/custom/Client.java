// Copyright (c) ZeroC, Inc.

package test.Ice.custom;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Properties;

import test.Ice.custom.Test.TestIntfPrx;
import test.TestHelper;

public class Client extends TestHelper {
    public void run(String[] args) {
        Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.custom");
        properties.setProperty("Ice.CacheMessageBuffers", "0");
        try (Communicator communicator = initialize(properties)) {
            TestIntfPrx test = AllTests.allTests(this);
            test.shutdown();
        }
    }
}
