// Copyright (c) ZeroC, Inc.

package test.Ice.invoke;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Properties;

import test.Ice.invoke.Test.MyClassPrx;
import test.TestHelper;

public class Client extends TestHelper {
    public void run(String[] args) {
        Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.invoke");
        try (Communicator communicator = initialize(properties)) {
            MyClassPrx myClass = AllTests.allTests(this);
            myClass.shutdown();
        }
    }
}
