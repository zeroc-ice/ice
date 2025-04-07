// Copyright (c) ZeroC, Inc.

package test.Ice.exceptions;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.Object;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.Util;

import test.TestHelper;

public class AMDServer extends TestHelper {
    public void run(String[] args) {
        InitializationData initData = new InitializationData();

        //
        // For this test, we need a dummy logger, otherwise the
        // assertion test will print an error message.
        //
        initData.logger = new DummyLogger();

        initData.properties = createTestProperties(args);
        initData.properties.setProperty("Ice.Warn.Dispatch", "0");
        initData.properties.setProperty("Ice.Warn.Connections", "0");
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.exceptions.AMD");
        initData.properties.setProperty("Ice.MessageSizeMax", "10"); // 10KB max
        try (Communicator communicator = initialize(initData)) {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            communicator.getProperties().setProperty("TestAdapter2.Endpoints", getTestEndpoint(1));
            communicator.getProperties().setProperty("TestAdapter2.MessageSizeMax", "0");
            communicator.getProperties().setProperty("TestAdapter3.Endpoints", getTestEndpoint(2));
            communicator.getProperties().setProperty("TestAdapter3.MessageSizeMax", "1");

            ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            ObjectAdapter adapter2 = communicator.createObjectAdapter("TestAdapter2");
            ObjectAdapter adapter3 = communicator.createObjectAdapter("TestAdapter3");
            Object object = new AMDThrowerI();
            adapter.add(object, Util.stringToIdentity("thrower"));
            adapter2.add(object, Util.stringToIdentity("thrower"));
            adapter3.add(object, Util.stringToIdentity("thrower"));
            adapter.activate();
            adapter2.activate();
            adapter3.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
