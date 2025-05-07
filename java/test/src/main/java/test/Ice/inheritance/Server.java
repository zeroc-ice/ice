// Copyright (c) ZeroC, Inc.

package test.Ice.inheritance;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Object;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.Util;

import test.TestHelper;

public class Server extends TestHelper {
    public void run(String[] args) {
        try (Communicator communicator = initialize(args)) {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            Object object = new InitialI(adapter);
            adapter.add(object, Util.stringToIdentity("initial"));
            adapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
