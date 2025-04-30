// Copyright (c) ZeroC, Inc.

package test.Ice.inactivityTimeout;

import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.ModuleToPackageSliceLoader;

import test.TestHelper;

public class Client extends TestHelper {
    @Override
    public void run(String[] args) {
        var initData = new InitializationData();
        initData.sliceLoader = new ModuleToPackageSliceLoader("::Test", "test.Ice.inactivityTimeout.Test");
        initData.properties = createTestProperties(args);
        // We configure a low idle timeout to make sure we send heartbeats frequently. It's the
        // sending of the heartbeats that schedules the inactivity timer task.
        initData.properties.setProperty("Ice.Connection.Client.IdleTimeout", "1");
        initData.properties.setProperty("Ice.Connection.Client.InactivityTimeout", "3");

        try (var communicator = initialize(initData)) {
            AllTests.allTests(this);
        }
    }
}
