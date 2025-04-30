// Copyright (c) ZeroC, Inc.

package test.IceSSL.configuration;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.ModuleToPackageSliceLoader;

import test.IceSSL.configuration.Test.ServerFactoryPrx;
import test.TestHelper;

import java.util.ArrayList;
import java.util.List;

public class Client extends TestHelper {
    public void run(String[] args) {
        List<String> remainingArgs = new ArrayList<String>();
        var initData = new InitializationData();
        initData.sliceLoader = new ModuleToPackageSliceLoader("::Test", "test.IceSSL.configuration.Test");
        initData.properties = createTestProperties(args, remainingArgs);
        if (remainingArgs.size() < 1) {
            throw new RuntimeException("Usage: client testdir");
        }

        String testDir = remainingArgs.get(0);
        try (Communicator communicator = initialize(initData)) {
            PlatformTests.allTests(this, testDir);
            ServerFactoryPrx factory = AllTests.allTests(this, testDir);
            factory.shutdown();
        }
    }
}
