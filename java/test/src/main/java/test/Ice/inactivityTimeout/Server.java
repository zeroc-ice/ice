// Copyright (c) ZeroC, Inc.

package test.Ice.inactivityTimeout;

import com.zeroc.Ice.Identity;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.ModuleToPackageSliceLoader;

import test.TestHelper;

public class Server extends TestHelper {
    public void run(String[] args) {
        var initData = new InitializationData();
        initData.sliceLoader = new ModuleToPackageSliceLoader("::Test", "test.Ice.inactivityTimeout.Test");
        initData.properties = createTestProperties(args);
        // We configure a low idle timeout to make sure we send heartbeats frequently. It's the
        // sending of the heartbeats that schedules the inactivity timer.
        initData.properties.setProperty("Ice.Connection.Server.IdleTimeout", "1");
        initData.properties.setProperty("TestAdapter.Connection.InactivityTimeout", "5");
        initData.properties.setProperty("TestAdapter3s.Connection.InactivityTimeout", "3");

        try (var communicator = initialize(initData)) {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint());
            communicator.getProperties().setProperty("TestAdapter3s.Endpoints", getTestEndpoint(1));

            var adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new TestIntfI(), new Identity("test", ""));
            adapter.activate();

            var adapter3s = communicator.createObjectAdapter("TestAdapter3s");
            adapter3s.add(new TestIntfI(), new Identity("test", ""));
            adapter3s.activate();

            serverReady();
            communicator.waitForShutdown();
        }
    }
}
