// Copyright (c) ZeroC, Inc.

package test.IceBox.admin;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.ProcessPrx;
import com.zeroc.Ice.Properties;

import test.TestHelper;

public class Client extends TestHelper {
    public void run(String[] args) {
        Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Default.Host", "127.0.0.1");
        try (Communicator communicator = initialize(properties)) {
            AllTests.allTests(this);

            // Shutdown the IceBox server.
            var prx =
                    ProcessPrx.createProxy(
                            communicator(), "DemoIceBox/admin -f Process:default -p 9996");
            prx.shutdown();
        }
    }
}
