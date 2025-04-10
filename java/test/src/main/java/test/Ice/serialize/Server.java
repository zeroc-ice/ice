// Copyright (c) ZeroC, Inc.

package test.Ice.serialize;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Identity;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.Properties;
import com.zeroc.Ice.Util;

import test.TestHelper;

public class Server extends TestHelper {
    public void run(String[] args) {
        Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.serialize");
        try (Communicator communicator = initialize(properties)) {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            ObjectAdapter adapter = communicator().createObjectAdapter("TestAdapter");
            Identity ident = Util.stringToIdentity("initial");
            adapter.add(new InitialI(adapter, ident), ident);
            adapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
