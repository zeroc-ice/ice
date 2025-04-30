// Copyright (c) ZeroC, Inc.

package test.Ice.networkProxy;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.ModuleToPackageSliceLoader;

import test.TestHelper;

public class Client extends TestHelper {
    public void run(String[] args) {
        var initData = new InitializationData();
        initData.sliceLoader = new ModuleToPackageSliceLoader("::Test", "test.Ice.networkProxy.Test");
        initData.properties = createTestProperties(args);

        try (Communicator communicator = initialize(initData)) {
            AllTests.allTests(this);
        }
    }
}
