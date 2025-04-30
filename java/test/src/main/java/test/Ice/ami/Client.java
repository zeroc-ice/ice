// Copyright (c) ZeroC, Inc.

package test.Ice.ami;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.ModuleToPackageSliceLoader;

import test.TestHelper;

public class Client extends TestHelper {
    public void run(String[] args) {
        var initData = new InitializationData();
        initData.sliceLoader = new ModuleToPackageSliceLoader("::Test", "test.Ice.ami.Test");
        initData.properties = createTestProperties(args);
        initData.properties.setProperty("Ice.Warn.AMICallback", "0");
        initData.properties.setProperty("Ice.Warn.Connections", "0");
        // Limit the send buffer size, this test relies on the socket
        // send() blocking after sending a given amount of data.
        initData.properties.setProperty("Ice.TCP.SndSize", "50000");

        try (Communicator communicator = initialize(initData)) {
            AllTests.allTests(this, false);
        }
    }
}
