// Copyright (c) ZeroC, Inc.

package test.Ice.ami;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Properties;

import test.TestHelper;

public class Client extends TestHelper {
    public void run(String[] args) {
        Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.ami");
        properties.setProperty("Ice.Warn.AMICallback", "0");
        properties.setProperty("Ice.Warn.Connections", "0");

        //
        // Limit the send buffer size, this test relies on the socket
        // send() blocking after sending a given amount of data.
        //
        properties.setProperty("Ice.TCP.SndSize", "50000");
        try (Communicator communicator = initialize(properties)) {
            AllTests.allTests(this, false);
        }
    }
}
