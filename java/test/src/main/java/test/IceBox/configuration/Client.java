// Copyright (c) ZeroC, Inc.

package test.IceBox.configuration;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.ProcessPrx;

import test.TestHelper;

public class Client extends TestHelper {
    public void run(String[] args) {
        try (Communicator communicator = initialize(args)) {
            AllTests.allTests(this);

            // Shutdown the IceBox server.
            var prx =
                ProcessPrx.createProxy(
                    communicator(), "DemoIceBox/admin -f Process:default -p 9996");
            prx.shutdown();
        }
    }
}
