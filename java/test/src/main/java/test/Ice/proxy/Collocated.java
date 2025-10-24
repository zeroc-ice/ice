// Copyright (c) ZeroC, Inc.

package test.Ice.proxy;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Identity;
import com.zeroc.Ice.ObjectAdapter;

import test.TestHelper;

public class Collocated extends TestHelper {
    public void run(String[] args) {
        var properties = createTestProperties(args);
        properties.setProperty("Ice.Warn.Dispatch", "0");

        try (Communicator communicator = initialize(properties)) {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new MyDerivedClassI(), new Identity("test", ""));
            adapter.add(new CI(), new Identity("c", ""));
            // adapter.activate(); // Don't activate OA to ensure collocation is used.
            AllTests.allTests(this);
        }
    }
}
