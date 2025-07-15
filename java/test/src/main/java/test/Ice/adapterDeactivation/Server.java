// Copyright (c) ZeroC, Inc.

package test.Ice.adapterDeactivation;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.ObjectAdapter;

import test.TestHelper;

public class Server extends TestHelper {
    public void run(String[] args) {
        try (Communicator communicator = initialize(args)) {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.addServantLocator(new ServantLocatorI(), "");
            adapter.activate();
            serverReady();
            adapter.waitForDeactivate();
        }
    }
}
