// Copyright (c) ZeroC, Inc.

package test.Ice.proxy;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.Util;

import test.TestHelper;

public class Server extends TestHelper {
    public void run(String[] args) {
        var properties = createTestProperties(args);
        properties.setProperty("Ice.Warn.Dispatch", "0");

        try (Communicator communicator = initialize(properties)) {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new MyDerivedClassI(), Util.stringToIdentity("test"));
            adapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
