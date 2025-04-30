// Copyright (c) ZeroC, Inc.

package test.Ice.idleTimeout;

import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.ModuleToPackageSliceLoader;

import test.TestHelper;

public class Client extends TestHelper {
    @Override
    public void run(String[] args) {
        var initData = new InitializationData();
        initData.sliceLoader = new ModuleToPackageSliceLoader("::Test", "test.Ice.idleTimeout.Test");
        initData.properties = createTestProperties(args);
        initData.properties.setProperty("Ice.Connection.Client.IdleTimeout", "1");

        try (var communicator = initialize(initData)) {
            AllTests.allTests(this);
        }
    }
}
