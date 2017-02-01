// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.retry;

import test.Ice.retry.Test.RetryPrx;

public class Collocated extends test.Util.Application
{
    private Instrumentation instrumentation = new Instrumentation();

    private void setupObjectAdapter(com.zeroc.Ice.Communicator communicator)
    {
        com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("");
        adapter.add(new RetryI(), com.zeroc.Ice.Util.stringToIdentity("retry"));
        //adapter.activate(); // Don't activate OA to ensure collocation is used.
    }

    @Override
    public int run(String[] args)
    {
        com.zeroc.Ice.Communicator communicator = communicator();

        //
        // Configure a second communicator for the invocation timeout
        // + retry test, we need to configure a large retry interval
        // to avoid time-sensitive failures.
        //
        com.zeroc.Ice.InitializationData initData2 = createInitializationData();
        initData2.properties = communicator.getProperties()._clone();
        initData2.properties.setProperty("Ice.RetryIntervals", "0 1 10000");
        initData2.observer = instrumentation.getObserver();
        com.zeroc.Ice.Communicator communicator2 = initialize(initData2);

        setupObjectAdapter(communicator);
        setupObjectAdapter(communicator2);

        try
        {
            RetryPrx retry = AllTests.allTests(this, communicator, communicator2, instrumentation, "retry");
            retry.shutdown();
            return 0;
        }
        finally
        {
            communicator2.destroy();
        }
    }

    @Override
    protected com.zeroc.Ice.InitializationData getInitData(String[] args, java.util.List<String> rArgs)
    {
        com.zeroc.Ice.InitializationData initData = super.getInitData(args, rArgs);
        initData.observer = instrumentation.getObserver();

        initData.properties.setProperty("Ice.Package.Test", "test.Ice.retry");

        initData.properties.setProperty("Ice.RetryIntervals", "0 1 10 1");

        //
        // We don't want connection warnings because of the timeout
        //
        initData.properties.setProperty("Ice.Warn.Connections", "0");
        initData.properties.setProperty("Ice.Warn.Dispatch", "0");

        return initData;
    }

    public static void main(String[] args)
    {
        Collocated app = new Collocated();
        int result = app.main("Collocated", args);
        System.gc();
        System.exit(result);
    }
}

