// Copyright (c) ZeroC, Inc.

package test.Ice.facets;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.ModuleToPackageSliceLoader;

import test.Ice.facets.Test.GPrx;
import test.TestHelper;

public class Client extends TestHelper {
    public void run(String[] args) {
        var initData = new InitializationData();
        initData.sliceLoader = new ModuleToPackageSliceLoader("::Test", "FAIL_IF_NONE");
        initData.properties = createTestProperties(args);

        try (Communicator communicator = initialize(initData)) {
            GPrx g = AllTests.allTests(this);
            g.shutdown();
        }
    }
}
