// Copyright (c) ZeroC, Inc.

package test.Ice.maxDispatches;

import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.ModuleToPackageSliceLoader;

import test.TestHelper;

public class Client extends TestHelper {
    @Override
    public void run(String[] args) {
        var initData = new InitializationData();
        initData.sliceLoader = new ModuleToPackageSliceLoader("::Test", "test.Ice.maxDispatches.Tests");
        initData.properties = createTestProperties(args);

        try (var communicator = initialize(initData)) {
            AllTests.allTests(this);
        }
    }
}
