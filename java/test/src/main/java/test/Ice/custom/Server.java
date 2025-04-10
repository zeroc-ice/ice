// Copyright (c) ZeroC, Inc.

package test.Ice.custom;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Object;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.Properties;
import com.zeroc.Ice.Util;

import test.TestHelper;

public class Server extends TestHelper {
    public void run(String[] args) {
        Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.custom");
        properties.setProperty("Ice.CacheMessageBuffers", "0");
        try (Communicator communicator = initialize(properties)) {
            properties.setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            Object test = new TestI(communicator);
            adapter.add(test, Util.stringToIdentity("test"));

            adapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
