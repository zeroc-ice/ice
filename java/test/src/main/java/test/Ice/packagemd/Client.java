// Copyright (c) ZeroC, Inc.

package test.Ice.packagemd;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Properties;

import test.Ice.packagemd.Test.InitialPrx;
import test.TestHelper;

public class Client extends TestHelper {
    public void run(String[] args) {
        Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Warn.Dispatch", "0");
        properties.setProperty("Ice.Package.Test", "test.Ice.packagemd");
        properties.setProperty("Ice.Package.Test1", "test.Ice.packagemd");
        try (Communicator communicator = initialize(properties)) {
            InitialPrx initial = AllTests.allTests(this);
            initial.shutdown();
        }
    }
}
