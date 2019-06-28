//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.operations;

public class Collocated extends test.TestHelper
{
    public void run(String[] args)
    {
        Ice.Properties properties = createTestProperties(args);
        if(properties.getPropertyAsInt("Ice.ThreadInterruptSafe") > 0 || isAndroid())
        {
            properties.setProperty("Ice.ThreadPool.Server.Size", "2");
        }
        properties.setProperty("Ice.Package.Test", "test.Ice.operations");

        properties.setProperty("Ice.BatchAutoFlushSize", "100");

        //
        // Its possible to have batch oneway requests dispatched
        // after the adapter is deactivated due to thread
        // scheduling so we supress this warning.
        //
        properties.setProperty("Ice.Warn.Dispatch", "0");

        try(Ice.Communicator communicator = initialize(properties))
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            Ice.ObjectPrx prx = adapter.add(new MyDerivedClassI(), Ice.Util.stringToIdentity("test"));
            adapter.add(new BI(), Ice.Util.stringToIdentity("b"));
            //adapter.activate(); // Don't activate OA to ensure collocation is used.

            if(prx.ice_getConnection() != null)
            {
                throw new RuntimeException();
            }

            AllTests.allTests(this);
        }
    }
}
