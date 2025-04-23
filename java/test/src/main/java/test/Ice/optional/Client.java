// Copyright (c) ZeroC, Inc.

package test.Ice.optional;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.Properties;

import test.Ice.optional.Test.InitialPrx;
import test.TestHelper;

public class Client extends TestHelper {
    public void run(String[] args) {
        var initData = new InitializationData();
        initData.properties = createTestProperties(args);
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.optional");
        var customSliceLoader = new AllTests.CustomSliceLoader();
        initData.sliceLoader = customSliceLoader;

        try (Communicator communicator = initialize(initData)) {
            InitialPrx initial = AllTests.allTests(this, customSliceLoader);
            initial.shutdown();
        }
    }
}
