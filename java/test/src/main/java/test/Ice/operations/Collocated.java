//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.operations;

import com.zeroc.Ice.Util;

public class Collocated extends test.TestHelper
{
    public void run(String[] args)
    {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
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
        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            java.io.PrintWriter out = getWriter();
            com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            com.zeroc.Ice.ObjectPrx prx = adapter.add(new MyDerivedClassI(), Util.stringToIdentity("test"));
            //adapter.activate(); // Don't activate OA to ensure collocation is used.

            if(prx.ice_getConnection() != null)
            {
                throw new RuntimeException();
            }
            AllTests.allTests(this);
        }
    }
}
