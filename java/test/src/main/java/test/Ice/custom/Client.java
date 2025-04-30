// Copyright (c) ZeroC, Inc.

package test.Ice.custom;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.ModuleToPackageSliceLoader;

import test.Ice.custom.Test.TestIntfPrx;
import test.TestHelper;

public class Client extends TestHelper {
    public void run(String[] args) {
        var initData = new InitializationData();
        initData.sliceLoader = new ModuleToPackageSliceLoader("::Test", "test.Ice.custom.Test");
        initData.properties = createTestProperties(args);
        initData.properties.setProperty("Ice.CacheMessageBuffers", "0");

        try (Communicator communicator = initialize(initData)) {
            TestIntfPrx test = AllTests.allTests(this);
            test.shutdown();
        }
    }
}
