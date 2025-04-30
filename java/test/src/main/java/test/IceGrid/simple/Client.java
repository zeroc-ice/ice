// Copyright (c) ZeroC, Inc.

package test.IceGrid.simple;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.ModuleToPackageSliceLoader;

import test.TestHelper;

import java.util.stream.Stream;

public class Client extends TestHelper {
    public void run(String[] args) {
        var initData = new InitializationData();
        initData.sliceLoader = new ModuleToPackageSliceLoader("::Test", "test.IceDiscovery.simple.Test");
        initData.properties = createTestProperties(args);

        try (Communicator communicator = initialize(initData)) {
            boolean withDeploy = Stream.of(args).anyMatch(v -> "--with-deploy".equals(v));

            if (!withDeploy) {
                AllTests.allTests(this);
            } else {
                AllTests.allTestsWithDeploy(this);
            }
        }
    }
}
