// Copyright (c) ZeroC, Inc.

package test.Ice.admin;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.Properties;
import com.zeroc.Ice.Util;

import test.TestHelper;

public class Server extends TestHelper {
    public void run(String[] args) {
        Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.admin");
        properties.setProperty("Ice.Warn.Dispatch", "0");
        try (Communicator communicator = initialize(properties)) {
            properties.setProperty("TestAdapter.Endpoints", getTestEndpoint());
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
