// Copyright (c) ZeroC, Inc.

package test.IceGrid.simple;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Object;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.ObjectAdapterDeactivatedException;
import com.zeroc.Ice.Properties;
import com.zeroc.Ice.Util;

import test.TestHelper;

public class Server extends TestHelper {
    public void run(String[] args) {
        Properties properties = createTestProperties(args);

        // It's possible to have batch oneway requests dispatched after the adapter is deactivated
        // due to thread scheduling so we suppress this warning.
        properties.setProperty("Ice.Warn.Dispatch", "0");
        properties.setProperty("Ice.Package.Test", "test.IceGrid.simple");
        try (Communicator communicator = initialize(args)) {
            communicator.getProperties().parseCommandLineOptions("TestAdapter", args);

            ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            Object object = new TestI();
            String id = communicator.getProperties().getPropertyWithDefault("Identity", "test");
            adapter.add(object, Util.stringToIdentity(id));
            try {
                adapter.activate();
            } catch (ObjectAdapterDeactivatedException ex) {
            }
            communicator().waitForShutdown();
        }
    }
}
