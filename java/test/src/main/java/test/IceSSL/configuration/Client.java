// Copyright (c) ZeroC, Inc.

package test.IceSSL.configuration;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Properties;

import test.IceSSL.configuration.Test.ServerFactoryPrx;
import test.TestHelper;

import java.util.ArrayList;
import java.util.List;

public class Client extends TestHelper {
    public void run(String[] args) {
        List<String> rargs = new ArrayList<String>();
        Properties properties = createTestProperties(args, rargs);
        if (rargs.size() < 1) {
            throw new RuntimeException("Usage: client testdir");
        }

        String testDir = rargs.get(0);

        properties.setProperty("Ice.Package.Test", "test.IceSSL.configuration");
        try (Communicator communicator = initialize(properties)) {
            PlatformTests.allTests(this, testDir);
            ServerFactoryPrx factory = AllTests.allTests(this, testDir);
            factory.shutdown();
        }
    }
}
