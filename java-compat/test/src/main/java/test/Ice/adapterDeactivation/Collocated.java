//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.adapterDeactivation;

public class Collocated extends test.TestHelper
{
    public void
    run(String[] args)
    {
        Ice.Properties properties = createTestProperties(args);
        //
        // 2 threads are necessary to dispatch the collocated transient() call with AMI
        //
        properties.setProperty("TestAdapter.ThreadPool.Size", "2");

        properties.setProperty("Ice.Package.Test", "test.Ice.adapterDeactivation");
        try(Ice.Communicator communicator = initialize(properties))
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            Ice.ServantLocator locator = new ServantLocatorI();
            adapter.addServantLocator(locator, "");
            AllTests.allTests(this);
            adapter.waitForDeactivate();
        }
    }
}
