// Copyright (c) ZeroC, Inc.

package test.Ice.timeout;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.ModuleToPackageSliceLoader;

import test.TestHelper;

public class Client extends TestHelper {
    @Override
    public void run(String[] args) {
        var initData = new InitializationData();
        initData.sliceLoader = new ModuleToPackageSliceLoader("::Test", "test.Ice.timeout.Test");
        initData.properties = createTestProperties(args);

        // For this test, we want to disable retries.
        initData.properties.setProperty("Ice.RetryIntervals", "-1");

        initData.properties.setProperty("Ice.Connection.Client.ConnectTimeout", "1");
        initData.properties.setProperty("Ice.Connection.Client.CloseTimeout", "1");

        try (Communicator communicator = initialize(initData)) {
            AllTests.allTests(this);
        }
    }
}
