// Copyright (c) ZeroC, Inc.

package test.Ice.admin;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.ModuleToPackageSliceLoader;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.Util;

import test.TestHelper;

public class Server extends TestHelper {
    public void run(String[] args) {
        var initData = new InitializationData();
        initData.sliceLoader = new ModuleToPackageSliceLoader("::Test", "test.Ice.admin.Test");
        initData.properties = createTestProperties(args);
        initData.properties.setProperty("Ice.Warn.Dispatch", "0");

        try (Communicator communicator = initialize(initData)) {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint());
            ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(
                new RemoteCommunicatorFactoryI(),
                Util.stringToIdentity("factory"));
            adapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
