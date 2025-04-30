// Copyright (c) ZeroC, Inc.

package test.Ice.executor;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.ModuleToPackageSliceLoader;

import test.TestHelper;

public class Client extends TestHelper {
    public void run(String[] args) {
        InitializationData initData = new InitializationData();
        initData.sliceLoader = new ModuleToPackageSliceLoader("::Test", "test.Ice.executor.Test");
        initData.properties = createTestProperties(args);
        // Limit the send buffer size, this test relies on the socket
        // send() blocking after sending a given amount of data.
        initData.properties.setProperty("Ice.TCP.SndSize", "50000");

        CustomExecutor executor = new CustomExecutor();
        initData.executor = executor;

        try (Communicator communicator = initialize(initData)) {
            AllTests.allTests(this, executor);
        }
        executor.terminate();
    }
}
