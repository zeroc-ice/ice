// Copyright (c) ZeroC, Inc.

package test.IceBox.configuration;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.ProcessPrx;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.ModuleToPackageSliceLoader;

import test.TestHelper;

public class Client extends TestHelper {
    public void run(String[] args) {
        var initData = new InitializationData();
        initData.sliceLoader = new ModuleToPackageSliceLoader("::Test", "test.IceBox.configuration.Test");
        initData.properties = createTestProperties(args);

        try (Communicator communicator = initialize(initData)) {
            AllTests.allTests(this);

            // Shutdown the IceBox server.
            var prx =
                ProcessPrx.createProxy(
                    communicator(), "DemoIceBox/admin -f Process:default -p 9996");
            prx.shutdown();
        }
    }
}
