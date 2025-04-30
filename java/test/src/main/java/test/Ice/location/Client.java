// Copyright (c) ZeroC, Inc.

package test.Ice.location;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.ModuleToPackageSliceLoader;

import test.TestHelper;

public class Client extends TestHelper {
    public void run(String[] args) {
        var initData = new InitializationData();
        initData.sliceLoader = new ModuleToPackageSliceLoader("::Test", "test.Ice.location.Test");
        initData.properties = createTestProperties(args);
        initData.properties.setProperty("Ice.Default.Locator", "locator:" + getTestEndpoint(initData.properties, 0));

        try (Communicator communicator = initialize(initData)) {
            AllTests.allTests(this);
        } catch (Exception ex) {
            throw new RuntimeException(ex);
        }
    }
}
