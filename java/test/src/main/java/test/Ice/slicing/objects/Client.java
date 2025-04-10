// Copyright (c) ZeroC, Inc.

package test.Ice.slicing.objects;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Properties;

import test.Ice.slicing.objects.client.Test.TestIntfPrx;
import test.TestHelper;

public class Client extends TestHelper {
    public void run(String[] args) {
        Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.slicing.objects.client");
        try (Communicator communicator = initialize(properties)) {
            TestIntfPrx test = AllTests.allTests(this, false);
            test.shutdown();
        }
    }
}
