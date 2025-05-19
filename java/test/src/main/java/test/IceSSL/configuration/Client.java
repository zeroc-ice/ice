// Copyright (c) ZeroC, Inc.

package test.IceSSL.configuration;

import com.zeroc.Ice.Communicator;

import test.IceSSL.configuration.Test.ServerFactoryPrx;
import test.TestHelper;

import java.util.ArrayList;
import java.util.List;

public class Client extends TestHelper {
    public void run(String[] args) {
        List<String> remainingArgs = new ArrayList<String>();
        var properties = createTestProperties(args, remainingArgs);
        if (remainingArgs.size() < 1) {
            throw new RuntimeException("Usage: client testdir");
        }

        String testDir = remainingArgs.get(0);
        try (Communicator communicator = initialize(properties)) {
            PlatformTests.allTests(this, testDir);

            // Run the tests with the default JKS certificates
            ServerFactoryPrx factory = AllTests.allTests(this, testDir, "JKS");

            // Run the tests again with the PKCS12 certificates
            factory = AllTests.allTests(this, testDir, "PKCS12");
            factory.shutdown();
        }
    }
}
