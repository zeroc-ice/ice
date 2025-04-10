// Copyright (c) ZeroC, Inc.

package test.Ice.adapterDeactivation;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.Properties;

import test.TestHelper;

public class Collocated extends TestHelper {
    public void run(String[] args) {
        Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.adapterDeactivation");

        try (Communicator communicator = initialize(properties)) {
            //
            // 2 threads are necessary to dispatch the collocated transient() call with AMI
            //
            communicator.getProperties().setProperty("TestAdapter.ThreadPool.Size", "2");
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint());
            ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.addServantLocator(new ServantLocatorI(), "");
            AllTests.allTests(this);
            adapter.waitForDeactivate();
        }
    }
}
