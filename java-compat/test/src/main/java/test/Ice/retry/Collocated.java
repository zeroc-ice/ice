//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.retry;

import test.Ice.retry.Test.RetryPrx;

public class Collocated extends test.TestHelper
{
    private Instrumentation instrumentation = new Instrumentation();

    private void
    setupObjectAdapter(Ice.Communicator communicator)
    {
        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("");
        adapter.add(new RetryI(), Ice.Util.stringToIdentity("retry"));
        //adapter.activate(); // Don't activate OA to ensure collocation is used.
    }

    public void run(String[] args)
    {
        Ice.InitializationData initData = new Ice.InitializationData();
        initData.observer = instrumentation.getObserver();

        initData.properties = createTestProperties(args);
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.retry");
        initData.properties.setProperty("Ice.RetryIntervals", "0 1 10 1");

        //
        // We don't want connection warnings because of the timeout
        //
        initData.properties.setProperty("Ice.Warn.Connections", "0");
        initData.properties.setProperty("Ice.Warn.Dispatch", "0");
        try(Ice.Communicator communicator = initialize(initData))
        {
            //
            // Configure a second communicator for the invocation timeout
            // + retry test, we need to configure a large retry interval
            // to avoid time-sensitive failures.
            //
            initData.properties = communicator.getProperties()._clone();
            initData.properties.setProperty("Ice.RetryIntervals", "0 1 10000");
            try(Ice.Communicator communicator2 = initialize(initData))
            {
                setupObjectAdapter(communicator);
                setupObjectAdapter(communicator2);

                RetryPrx retry = AllTests.allTests(communicator, communicator2, getWriter(), instrumentation, "retry");
                retry.shutdown();
            }
        }
    }
}
