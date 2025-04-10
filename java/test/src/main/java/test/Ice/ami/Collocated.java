// Copyright (c) ZeroC, Inc.

package test.Ice.ami;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.Properties;
import com.zeroc.Ice.Util;

import test.TestHelper;

public class Collocated extends TestHelper {
    public void run(String[] args) {
        Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.ami");
        properties.setProperty("Ice.Warn.AMICallback", "0");
        try (Communicator communicator = initialize(properties)) {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            communicator
                .getProperties()
                .setProperty("ControllerAdapter.Endpoints", getTestEndpoint(1));
            communicator.getProperties().setProperty("ControllerAdapter.ThreadPool.Size", "1");

            ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            ObjectAdapter adapter2 =
                communicator.createObjectAdapter("ControllerAdapter");

            adapter.add(new TestI(), Util.stringToIdentity("test"));
            adapter.add(new TestII(), Util.stringToIdentity("test2"));
            // adapter.activate(); // Collocated test doesn't need to activate the OA
            adapter2.add(
                new TestControllerI(adapter),
                Util.stringToIdentity("testController"));
            // adapter2.activate(); // Collocated test doesn't need to activate the OA

            AllTests.allTests(this, true);
        }
    }
}
