// Copyright (c) ZeroC, Inc.

package test.Ice.custom;

import com.zeroc.Ice.Communicator;

import test.Ice.custom.Test.TestIntfPrx;
import test.TestHelper;

public class Client extends TestHelper {
    public void run(String[] args) {
        var properties = createTestProperties(args);
        properties.setProperty("Ice.CacheMessageBuffers", "0");

        try (Communicator communicator = initialize(properties)) {
            TestIntfPrx test = AllTests.allTests(this);
            test.shutdown();
        }
    }
}
