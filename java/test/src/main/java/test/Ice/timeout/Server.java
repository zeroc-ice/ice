// Copyright (c) ZeroC, Inc.

package test.Ice.timeout;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.ModuleToPackageSliceLoader;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.Util;

import test.TestHelper;

public class Server extends TestHelper {
    public void run(String[] args) {
        var initData = new InitializationData();
        initData.sliceLoader = new ModuleToPackageSliceLoader("::Test", "test.Ice.timeout.Test");
        initData.properties = createTestProperties(args);

        try (Communicator communicator = initialize(initData)) {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            communicator
                .getProperties()
                .setProperty("ControllerAdapter.Endpoints", getTestEndpoint(1));
            communicator.getProperties().setProperty("ControllerAdapter.ThreadPool.Size", "1");

            ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new TimeoutI(), Util.stringToIdentity("timeout"));
            adapter.activate();

            ObjectAdapter controllerAdapter =
                communicator.createObjectAdapter("ControllerAdapter");
            controllerAdapter.add(
                new ControllerI(adapter), Util.stringToIdentity("controller"));
            controllerAdapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
