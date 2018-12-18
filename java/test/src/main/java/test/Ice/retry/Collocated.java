// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.retry;

import test.Ice.retry.Test.RetryPrx;

public class Collocated extends test.TestHelper
{
    private Instrumentation instrumentation = new Instrumentation();

    private void setupObjectAdapter(com.zeroc.Ice.Communicator communicator)
    {
        com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("");
        adapter.add(new RetryI(), com.zeroc.Ice.Util.stringToIdentity("retry"));
        //adapter.activate(); // Don't activate OA to ensure collocation is used.
    }

    @Override
    public void run(String[] args)
    {
        com.zeroc.Ice.InitializationData initData = new com.zeroc.Ice.InitializationData();
        initData.observer = instrumentation.getObserver();
        initData.properties = createTestProperties(args);
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.retry");
        initData.properties.setProperty("Ice.RetryIntervals", "0 1 10 1");
        //
        // We don't want connection warnings because of the timeout
        //
        initData.properties.setProperty("Ice.Warn.Connections", "0");
        initData.properties.setProperty("Ice.Warn.Dispatch", "0");
        try(com.zeroc.Ice.Communicator communicator = initialize(initData))
        {
            //
            // Configure a second communicator for the invocation timeout
            // + retry test, we need to configure a large retry interval
            // to avoid time-sensitive failures.
            //
            initData.properties = communicator.getProperties()._clone();
            initData.properties.setProperty("Ice.RetryIntervals", "0 1 10000");
            initData.observer = instrumentation.getObserver();
            try(com.zeroc.Ice.Communicator communicator2 = initialize(initData))
            {

                setupObjectAdapter(communicator);
                setupObjectAdapter(communicator2);

                RetryPrx retry = AllTests.allTests(this, communicator, communicator2, instrumentation, "retry");
                retry.shutdown();
            }
        }
    }
}
