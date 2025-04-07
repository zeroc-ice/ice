// Copyright (c) ZeroC, Inc.

package test.Ice.executor;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.Util;

import test.TestHelper;

public class Server extends TestHelper {
    public void run(String[] args) {
        InitializationData initData = new InitializationData();
        CustomExecutor executor = new CustomExecutor();
        initData.properties = createTestProperties(args);
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.executor");
        //
        // Limit the recv buffer size, this test relies on the socket
        // send() blocking after sending a given amount of data.
        //
        initData.properties.setProperty("Ice.TCP.RcvSize", "50000");
        initData.executor = executor;
        try (Communicator communicator = initialize(initData)) {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            communicator
                    .getProperties()
                    .setProperty("ControllerAdapter.Endpoints", getTestEndpoint(1, "tcp"));
            communicator.getProperties().setProperty("ControllerAdapter.ThreadPool.Size", "1");

            ObjectAdapter adapter = communicator().createObjectAdapter("TestAdapter");
            ObjectAdapter adapter2 =
                    communicator().createObjectAdapter("ControllerAdapter");

            adapter.add(new TestI(executor), Util.stringToIdentity("test"));
            adapter.activate();
            adapter2.add(
                    new TestControllerI(adapter),
                    Util.stringToIdentity("testController"));
            adapter2.activate();
            serverReady();
            communicator.waitForShutdown();
        }
        executor.terminate();
    }
}
