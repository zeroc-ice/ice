// Copyright (c) ZeroC, Inc.

package test.Ice.seqMapping;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.Properties;
import com.zeroc.Ice.Util;

import test.TestHelper;

public class Collocated extends TestHelper {
    public void run(String[] args) {
        Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.seqMapping");
        try (Communicator communicator = initialize(properties)) {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new MyClassI(), Util.stringToIdentity("test"));
            // adapter.activate(); // Don't activate OA to ensure collocation is used.
            AllTests.allTests(this, true);
        }
    }
}
