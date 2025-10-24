// Copyright (c) ZeroC, Inc.

package test.Ice.ami;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Identity;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.ModuleToPackageSliceLoader;
import com.zeroc.Ice.ObjectAdapter;

import test.TestHelper;

public class Server extends TestHelper {
    public void run(String[] args) {
        var initData = new InitializationData();
        initData.sliceLoader = new ModuleToPackageSliceLoader("::Test", "test.Ice.ami.Test");
        initData.properties = createTestProperties(args);
        // This test kills connections, so we don't want warnings.
        initData.properties.setProperty("Ice.Warn.Connections", "0");
        // Limit the recv buffer size, this test relies on the socket
        // send() blocking after sending a given amount of data.
        initData.properties.setProperty("Ice.TCP.RcvSize", "50000");

        try (Communicator communicator = initialize(initData)) {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            communicator
                .getProperties()
                .setProperty("ControllerAdapter.Endpoints", getTestEndpoint(1));
            communicator.getProperties().setProperty("ControllerAdapter.ThreadPool.Size", "1");

            ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            ObjectAdapter adapter2 =
                communicator.createObjectAdapter("ControllerAdapter");

            adapter.add(new TestI(), new Identity("test", ""));
            adapter.add(new TestII(), new Identity("test2", ""));
            adapter.activate();
            adapter2.add(
                new TestControllerI(adapter),
                new Identity("testController", ""));
            adapter2.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
