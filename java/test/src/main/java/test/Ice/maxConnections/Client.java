// Copyright (c) ZeroC, Inc.

package test.Ice.maxConnections;

import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.ModuleToPackageSliceLoader;

import test.TestHelper;

public class Client extends TestHelper {
    @Override
    public void run(String[] args) {
        var initData = new InitializationData();
        initData.sliceLoader = new ModuleToPackageSliceLoader("::Test", "test.Ice.maxConnections.Test");
        initData.properties = createTestProperties(args);
        // We disable retries to make the logs clearer and avoid hiding potential issues.
        initData.properties.setProperty("Ice.RetryIntervals", "-1");

        try (var communicator = initialize(initData)) {
            AllTests.allTests(this);
        }
    }
}
