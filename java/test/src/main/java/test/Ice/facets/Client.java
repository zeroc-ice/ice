// Copyright (c) ZeroC, Inc.

package test.Ice.facets;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Properties;

import test.Ice.facets.Test.GPrx;
import test.TestHelper;

public class Client extends TestHelper {
    public void run(String[] args) {
        Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.facets");
        try (Communicator communicator = initialize(properties)) {
            GPrx g = AllTests.allTests(this);
            g.shutdown();
        }
    }
}
